/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_MovieClipLoader.h
Content     :   Implementation of MovieClipLoader class
Created     :   March, 2011
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_MOVIECLIPLOADER_H
#define INC_SF_GFX_MOVIECLIPLOADER_H

#include "GFxConfig.h"
#include "GFx/AS2/AS2_Object.h"
#ifdef GFX_AS2_ENABLE_MOVIECLIPLOADER
#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_String.h"
#include "GFx/GFx_StringHash.h"
#include "GFx/GFx_DisplayObject.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class MovieClipLoader;
class MovieClipLoaderProto;
class MovieClipLoaderCtorFunction;

// ***** External Classes
class ArrayObject;
class Environment;



class MovieClipLoader : public Object
{
    friend class MovieClipLoaderProto;

    struct ProgressDesc
    {
        int LoadedBytes;
        int TotalBytes;

        ProgressDesc() {}
        ProgressDesc(int loadedBytes, int totalBytes): LoadedBytes(loadedBytes), TotalBytes(totalBytes) {}
    };
    StringHashLH<ProgressDesc> ProgressInfo;
    void commonInit (Environment* penv);
#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        ProgressInfo.~StringHashLH();
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
protected:
    MovieClipLoader(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    MovieClipLoader(Environment* penv);

    ObjectType      GetObjectType() const   { return Object_MovieClipLoader; }

    virtual void NotifyOnLoadStart(Environment* penv, InteractiveObject* ptarget);
    virtual void NotifyOnLoadComplete(Environment* penv, InteractiveObject* ptarget, int status);
    virtual void NotifyOnLoadInit(Environment* penv, InteractiveObject* ptarget);
    virtual void NotifyOnLoadError(Environment* penv, InteractiveObject* ptarget, const char* errorCode, int status);
    virtual void NotifyOnLoadProgress(Environment* penv, InteractiveObject* ptarget, int loadedBytes, int totalBytes);

    int GetLoadedBytes(InteractiveObject* pch) const;
    int GetTotalBytes(InteractiveObject* pch)  const;
};

class MovieClipLoaderProto : public Prototype<MovieClipLoader>
{
public:
    MovieClipLoaderProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void GetProgress(const FnCall& fn);
    static void LoadClip(const FnCall& fn);
    static void UnloadClip(const FnCall& fn);
};

class MovieClipLoaderCtorFunction : public CFunctionObject
{
public:
    MovieClipLoaderCtorFunction (ASStringContext *psc);

    virtual Object* CreateNewObject(Environment* penv) const;    

    static void GlobalCtor(const FnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#else
namespace Scaleform { namespace GFx { namespace AS2 {
class MovieClipLoader : public Object
{
public:
    MovieClipLoader(ASStringContext * psc) : Object(psc) {}
    void NotifyOnLoadStart(Environment*, InteractiveObject*) {}
    void NotifyOnLoadComplete(Environment*, InteractiveObject*, int) {}
    void NotifyOnLoadInit(Environment*, InteractiveObject*) {}
    void NotifyOnLoadError(Environment*, InteractiveObject*, const char*, int) {}
    void NotifyOnLoadProgress(Environment*, InteractiveObject*, int, int) {}
};
}}} // SF::GFx::AS2

#endif //#ifdef GFX_AS2_ENABLE_MOVIECLIPLOADER


#endif // INC_SF_GFX_MOVIECLIPLOADER_H
