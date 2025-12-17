/**************************************************************************

Filename    :   AS2_BlurFilter.h
Content     :   BlurFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GASBlurFilter_H
#define INC_GASBlurFilter_H

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_BitmapFilter.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class BlurFilterObject;
class BlurFilterProto;
class BlurFilterCtorFunction;


// 
// Actionscript BlurFilter object declaration
//

class BlurFilterObject : public BitmapFilterObject
{
    friend class BlurFilterProto;
    friend class BlurFilterCtorFunction;

protected:

//     BlurFilterObject();
//     BlurFilterObject(const BlurFilterObject& obj) : BitmapFilterObject(obj) {}
    BlurFilterObject(ASStringContext *psc, Object* pprototype)
        : BitmapFilterObject(psc, pprototype)
    { 
    }

    virtual ~BlurFilterObject();

public:

    BlurFilterObject(Environment* penv);

    virtual ObjectType  GetObjectType() const { return Object_BlurFilter; }

    virtual bool        SetMember(Environment* penv, 
                                  const ASString& name, const Value& val, 
                                  const PropFlags& flags = PropFlags());
    virtual bool        GetMember(Environment* penv, 
                                  const ASString& name, Value* val);
};


class BlurFilterProto : public Prototype<BlurFilterObject>
{
public:
    BlurFilterProto(ASStringContext *psc, Object* prototype, 
                    const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    //
    // Default BitmapFilter object functions
    //
    static void         Clone(const FnCall& fn);
};


class BlurFilterCtorFunction : public CFunctionObject
{
public:
    BlurFilterCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_FILTERS

#endif  // INC_GASBlurFilter_H
