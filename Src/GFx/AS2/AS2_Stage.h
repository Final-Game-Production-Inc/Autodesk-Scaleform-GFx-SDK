/**************************************************************************

Filename    :   AS2_Stage.h
Content     :   Stage class implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_STAGE_H
#define INC_SF_GFX_STAGE_H

#include "GFxConfig.h"
#ifdef GFX_AS2_ENABLE_STAGE
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class StageObject;
class StageProto;
class StageCtorFunction;

// ***** External Classes
class Value;


// ActionScript Stage objects.

class StageObject : public Object
{
    friend class StageProto;
protected:
    StageObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    StageObject(Environment* penv);

    virtual ObjectType          GetObjectType() const   { return Object_Stage; }
};

class StageProto : public Prototype<StageObject>
{
public:
    StageProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

};

class StageCtorFunction : public CFunctionObject
{
    MovieImpl* pMovieRoot;

#ifdef GFX_AS_ENABLE_GC
    void Finalize_GC()
    {
        pMovieRoot = 0;
        CFunctionObject::Finalize_GC();
    }
#endif // SF_NO_GC
    static Value CreateRectangleObject(Environment *penv, const RectF& rect);
public:
    StageCtorFunction(ASStringContext *psc, MovieImpl* movieRoot);

    bool    GetMember(Environment *penv, const ASString& name, Value* val);
    bool    GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val);
    bool    SetMember(Environment *penv, const ASString& name, const Value& val, const PropFlags& flags);
    bool    SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val, const PropFlags& flags);

    void    NotifyOnResize(Environment* penv);
    static void NotifyOnResize(const FnCall& fn);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment*) const { return 0; }

    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif //#ifdef GFX_AS2_ENABLE_STAGE
#endif // INC_SF_GFX_STAGE_H

