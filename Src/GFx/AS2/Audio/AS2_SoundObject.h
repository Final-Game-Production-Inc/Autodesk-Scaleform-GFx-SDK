/**************************************************************************

Filename    :   AS2_SoundObject.h
Content     :   GFx audio: AS2 Sound class
Created     :   October 2006
Authors     :   Artyom Bolgar, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_AS2SOUNDOBJECT_H
#define INC_GFX_AS2SOUNDOBJECT_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//////////////////////////////////////////////////////////////////////////
//

class SoundObject : public Object, public ASSoundIntf
{
protected:
    SoundObject(ASStringContext *psc, Object* pprototype) : Object(psc)
    { 
        commonInit();
        Set__proto__(psc, pprototype);
    }
    void commonInit()
    {
        pMovieRoot = NULL;
    }

public:
    ~SoundObject()
    {
        DetachFromTarget();
    }
    Ptr<Sound::SoundSample> pSample;
    Ptr<SoundResource>      pResource;      // Resource which was used to create this sound
    Ptr<CharacterHandle>    pTargetHandle;  // MovieClip which sound object is connected to
    MovieImpl*              pMovieRoot;

    ObjectType GetObjectType() const { return Object_Sound; }

    SoundObject(Environment* penv) : Object(penv)
    {        
        commonInit();
        ASStringContext* psc = penv->GetSC();
        Set__proto__(psc, penv->GetPrototype(ASBuiltin_Sound));
        pMovieRoot = penv->GetMovieImpl();
    }
    
    virtual bool GetMember(Environment* penv, const ASString& name, Value* val);

    void         AttachToTarget(Sprite* psprite);
    void         DetachFromTarget();
    void         ReleaseTarget();

    Sprite*      GetSprite();
    void         ExecuteOnSoundComplete();

#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        DetachFromTarget();
        pSample.~Ptr<Sound::SoundSample>();
        pTargetHandle.~Ptr<CharacterHandle>();
        pResource.~Ptr<SoundResource>();
        Object::Finalize_GC();
    }
#endif // GFX_AS_ENABLE_GC
};

//////////////////////////////////////////////////////////////////////////
//

class SoundProto : public Prototype<SoundObject>
{
public:
    SoundProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void Attach(const FnCall& fn);
    static void GetBytesLoaded(const FnCall& fn);
    static void GetBytesTotal(const FnCall& fn);
    static void GetPan(const FnCall& fn);
    static void GetTransform(const FnCall& fn);
    static void GetVolume(const FnCall& fn);
    static void LoadSound(const FnCall& fn);
    static void SetPan(const FnCall& fn);
    static void SetTransform(const FnCall& fn);
    static void SetVolume(const FnCall& fn);
    static void Start(const FnCall& fn);
    static void Stop(const FnCall& fn);
};

class SoundCtorFunction : public CFunctionObject
{
public:
    SoundCtorFunction(ASStringContext *psc) : CFunctionObject(psc, GlobalCtor) {}

    virtual Object* CreateNewObject(Environment* penv) const;

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

}}} // namespace Scaleform::GFx::AS2

#endif // GFX_ENABLE_SOUND

#endif // INC_GFX_AS2SOUNDOBJECT_H
