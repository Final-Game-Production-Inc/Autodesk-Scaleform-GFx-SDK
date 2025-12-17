/**********************************************************************

Filename    :   AS2_ColorMatrixFilter.h
Content     :   ColorMatrixFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_ColorMatrixFilter_H
#define INC_ColorMatrixFilter_H

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_BitmapFilter.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class GASColorMatrixFilterObject;
class GASColorMatrixFilterProto;
class GASColorMatrixFilterCtorFunction;


// 
// Actionscript ColorMatrixFilter object declaration
//

class ColorMatrixFilterObject : public BitmapFilterObject
{
    friend class ColorMatrixFilterProto;
    friend class ColorMatrixFilterCtorFunction;

protected:

//     ColorMatrixFilterObject();
//     ColorMatrixFilterObject(const ColorMatrixFilterObject& obj) : BitmapFilterObject(obj) {}
    ColorMatrixFilterObject(ASStringContext *psc, Object* pprototype)
        : BitmapFilterObject(psc, pprototype)
    { 
    }

    virtual             ~ColorMatrixFilterObject();

public:

    ColorMatrixFilterObject(Environment* penv);

    virtual ObjectType  GetObjectType() const { return Object_ColorMatrixFilter; }

    virtual bool        SetMember(Environment* penv, 
        const ASString& name, const Value& val, 
        const PropFlags& flags = PropFlags());

    virtual bool        GetMember(Environment* penv, 
        const ASString& name, Value* val);
};


class ColorMatrixFilterProto : public Prototype<ColorMatrixFilterObject>
{
public:
    ColorMatrixFilterProto(ASStringContext *psc, Object* prototype, 
        const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    //
    // Default BitmapFilter object functions
    //
    static void         Clone(const FnCall& fn);
};


class ColorMatrixFilterCtorFunction : public CFunctionObject
{
public:
    ColorMatrixFilterCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};

}}}; //SF::GFx::AS2

#endif  // GFX_AS2_ENABLE_FILTERS


#endif  // INC_ColorMatrixFilter_H
