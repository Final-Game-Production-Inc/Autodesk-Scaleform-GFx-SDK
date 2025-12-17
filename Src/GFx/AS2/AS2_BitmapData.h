/**************************************************************************

Filename    :   AS2_BitmapData.h
Content     :   Implementation of BitmapData class
Created     :   March, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_BITMAPDATA_H
#define INC_SF_GFX_BITMAPDATA_H

#include "GFxConfig.h"
#ifdef GFX_AS2_ENABLE_BITMAPDATA
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"


namespace Scaleform { namespace GFx { namespace AS2 {

    // ***** Declared Classes
class BitmapData;
class BitmapDataProto;
class BitmapDataCtorFunction;

// ***** External Classes
class ArrayObject;
class Environment;



class BitmapData : public Object
{
    Ptr<ImageResource> pImageRes;
    Ptr<MovieDef>      pMovieDef; // holder for MovieDef to avoid "_Images" heap release

    void commonInit (Environment* penv);
#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        pImageRes = NULL;
        pMovieDef = NULL;
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
protected:
    BitmapData(ASStringContext* psc, Object* pprototype) 
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    BitmapData(Environment* penv);
    ~BitmapData() {}

    ObjectType      GetObjectType() const   { return Object_BitmapData; }

    void            SetImage(Environment* penv, ImageResource* pimg, MovieDef* pmovieDef);
    ImageResource*  GetImage() const                 { return pImageRes; }

    bool            SetMember(Environment* penv, const ASString& name, const Value& val, 
                              const PropFlags& flags = PropFlags());
    bool            GetMember(Environment* penv, const ASString& name, Value* val);

    MovieDef*       GetMovieDef() const { return pMovieDef; }

    static BitmapData* LoadBitmap(Environment* penv, const String& linkageId);
};

class BitmapDataProto : public Prototype<BitmapData>
{
public:
    BitmapDataProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

};

class BitmapDataCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];
public:
    BitmapDataCtorFunction (ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    static void LoadBitmap(const FnCall& fn);
    virtual Object* CreateNewObject(Environment*) const { return 0; }

    static FunctionRef Register(GlobalContext* pgc);
};

}}} //SF::GFx::AS2
#endif //#ifdef GFX_AS2_ENABLE_BITMAPDATA

#endif // INC_SF_GFX_BITMAPDATA_H
