/**************************************************************************

Filename    :   AS2_KeyObject.h
Content     :   Implementation of Key class
Created     :   December, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_AS2_KEYOBJECT_H
#define INC_SF_GFX_AS2_KEYOBJECT_H

#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

#ifdef GFX_ENABLE_KEYBOARD
//
// Key AS class implementation
//
class KeyObject : public Object
{
protected:

    void commonInit (Environment* penv);

    KeyObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
    ObjectType          GetObjectType() const   { return Object_Key; }
public:
};

class KeyProto : public Prototype<KeyObject>
{
public:
    KeyProto (ASStringContext* psc, Object* pprototype, const FunctionRef& constructor);
};

class KeyCtorFunction : public CFunctionObject, public KeyboardState::IListener
{
protected:
#ifndef GFX_AS_ENABLE_GC
    // Hack to store both WeakPtrs and correct interfaces.
    // If weak ptr is null, the interface pointer is unusable.
    // TBD: This should be an array of GASValues, or at least a union of { Object, CharacterHandle }.
    // This is not necessary if cycle collector is used. In this case AsBroadcaster mechanism 
    // is used.
    ArrayLH<WeakPtr<RefCountWeakSupportImpl> >  ListenerWeakRefs;
    ArrayLH<ObjectInterface*>                   Listeners;
#endif //SF_NO_GC
    struct State
    {
        int                         LastKeyCode;
        UByte                       LastAsciiCode;
        UInt32                      LastWcharCode;

        State():LastKeyCode(0), LastAsciiCode(0), LastWcharCode(0) {}
    } States[GFX_MAX_CONTROLLERS_SUPPORTED];
    MovieImpl*                  pMovieRoot;


    // KeyboardState::IListener methods
    virtual void OnKeyDown(InteractiveObject *pmovie, const EventId& evt, int /*keyMask*/) 
    {
        NotifyListeners(pmovie, evt);
    }       
    virtual void OnKeyUp(InteractiveObject *pmovie, const EventId& evt, int /*keyMask*/) 
    {
        NotifyListeners(pmovie, evt);
    }
    virtual void Update(const EventId& evt);

#ifdef GFX_AS_ENABLE_GC
    void Finalize_GC()
    {    
        pMovieRoot = 0;
        Object::Finalize_GC();
    }
#endif // SF_NO_GC

    static const NameFunction StaticFunctionTable[];

    virtual Object* CreateNewObject(Environment*) const { return 0; }
    static void     GlobalCtor(const FnCall& fn);
public:

    KeyCtorFunction(ASStringContext *psc, MovieImpl* proot);

    void NotifyListeners(InteractiveObject *pmovie, const EventId& evt);

    bool IsKeyDown(int code, unsigned keyboardIndex);
    bool IsKeyToggled(int code, unsigned keyboardIndex);

    // If there is no cycle collector (CC) we need to implement custom listeners logic
    // to avoid circular references. If CC is used then we use just regular AsBroadcaster
    // mechanism.
#ifndef GFX_AS_ENABLE_GC
    // Remove dead entries in the listeners list.  (Since
    // we use WeakPtr's, listeners can disappear without
    // notice.)
    void CleanupListeners();
    void AddListener(RefCountWeakSupportImpl *pref, ObjectInterface* listener);
    void RemoveListener(RefCountWeakSupportImpl *pref, ObjectInterface* listener);

    // support for _listeners
    bool GetMember(Environment* penv, const ASString& name, Value* val);

    static void KeyAddListener(const FnCall& fn);
    static void KeyRemoveListener(const FnCall& fn);
#endif // SF_NO_GC

    static void KeyGetAscii(const FnCall& fn);
    static void KeyGetCode(const FnCall& fn);
    static void KeyIsDown(const FnCall& fn);
    static void KeyIsToggled(const FnCall& fn);

    inline int GetLastKeyCode(unsigned controllerIdx) const 
    { 
        SF_ASSERT(controllerIdx < GFX_MAX_CONTROLLERS_SUPPORTED);
        return States[controllerIdx].LastKeyCode; 
    }
    inline int GetLastAsciiCode(unsigned controllerIdx) const 
    { 
        SF_ASSERT(controllerIdx < GFX_MAX_CONTROLLERS_SUPPORTED);
        return States[controllerIdx].LastAsciiCode; 
    }
};
#endif // GFX_ENABLE_KEYBOARD

}}} //SF::GFx::AS2
#endif // INC_SF_GFX_AS2_KEYOBJECT_H
