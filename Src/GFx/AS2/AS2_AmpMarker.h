/**************************************************************************

Filename    :   AS2_AmpMarker.h
Content     :   Implementation of marker class for AMP
Created     :   May, 2010
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_GAS_AMP_MARKER_H
#define INC_GAS_AMP_MARKER_H

#include "GFxConfig.h"
#ifdef SF_AMP_SERVER
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** External Classes
class ArrayObject;
class Environment;

class AmpMarker : public Object
{
protected:
    AmpMarker(ASStringContext *sc, Object* prototype)
        : Object(sc)
    { 
        Set__proto__(sc, prototype); // this ctor is used for prototype obj only
    }

    void commonInit(Environment* env);

public:
    AmpMarker(Environment* env);
};

class AmpMarkerProto : public Prototype<AmpMarker>
{
public:
    AmpMarkerProto(ASStringContext* sc, Object* prototype, const FunctionRef& constructor);

};

//
// AMP Marker static class
//
// A constructor function object for Object
class AmpMarkerCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void AddMarker(const FnCall& fn);

public:
    AmpMarkerCtorFunction(ASStringContext *sc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment*) const { return 0; }

    virtual bool GetMember(Environment* penv, const ASString& name, Value* pval);
    virtual bool SetMember(Environment *penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());

    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif

#endif // INC_GAS_AMP_MARKER_H
