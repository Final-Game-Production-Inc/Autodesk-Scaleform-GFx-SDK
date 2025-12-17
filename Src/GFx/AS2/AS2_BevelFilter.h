/**********************************************************************

Filename    :   AS2_BevelFilter.h
Content     :   BevelFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_BevelFilter_H
#define INC_BevelFilter_H

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_BitmapFilter.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class BevelFilterObject;
class BevelFilterProto;
class BevelFilterCtorFunction;


// 
// Actionscript BevelFilter object declaration
//

class BevelFilterObject : public BitmapFilterObject
{
    friend class BevelFilterProto;
    friend class BevelFilterCtorFunction;

protected:

//     BevelFilterObject();
//     BevelFilterObject(const BevelFilterObject& obj) : BitmapFilterObject(obj) {}
    BevelFilterObject(ASStringContext *psc, Object* pprototype)
        : BitmapFilterObject(psc, pprototype)
    { 
    }

    virtual             ~BevelFilterObject();

public:

    BevelFilterObject(Environment* penv);

    virtual ObjectType  GetObjectType() const { return Object_BevelFilter; }

    virtual bool        SetMember(Environment* penv, 
        const ASString& name, const Value& val, 
        const PropFlags& flags = PropFlags());

    virtual bool        GetMember(Environment* penv, 
        const ASString& name, Value* val);
};


class BevelFilterProto : public Prototype<BevelFilterObject>
{
public:
    BevelFilterProto(ASStringContext *psc, Object* prototype, 
        const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    //
    // Default BitmapFilter object functions
    //
    static void         Clone(const FnCall& fn);
};


class BevelFilterCtorFunction : public CFunctionObject
{
public:
    BevelFilterCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};

}}}; //SF::GFx::AS2

#endif  // GFX_AS2_ENABLE_FILTERS


#endif  // INC_BevelFilter_H
