/**************************************************************************

Filename    :   AS2_KeyObject.cpp
Content     :   Implementation of Key class
Created     :   December, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS2/AS2_KeyObject.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/AS2_AvmSprite.h" // to get Environment from level0 movie
#include "GFx/AS2/AS2_MovieRoot.h" // to get Environment from level0 movie

namespace Scaleform { namespace GFx { namespace AS2 {

#ifdef GFX_ENABLE_KEYBOARD
//////////////////////////////////////////
//
static void GFx_KeyFuncStub(const FnCall& fn)
{
    fn.Result->SetUndefined();
}

static const NameFunction GAS_KeyFunctionTable[] = 
{
    {"addListener",     &GFx_KeyFuncStub},
    {"getAscii",        &GFx_KeyFuncStub},
    {"getCode",         &GFx_KeyFuncStub},
    {"isDown",          &GFx_KeyFuncStub},
    {"isToggled",       &GFx_KeyFuncStub},
    {"removeListener",  &GFx_KeyFuncStub},
    { 0, 0 }
};

KeyProto::KeyProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor)
: Prototype<KeyObject>(psc, pprototype, constructor)
{
    InitFunctionMembers (psc, GAS_KeyFunctionTable);
}

//////////////////////////////////////////
//
const NameFunction KeyCtorFunction::StaticFunctionTable[] =
{
#ifndef GFX_AS_ENABLE_GC
    {"addListener",     &KeyCtorFunction::KeyAddListener},
    {"removeListener",  &KeyCtorFunction::KeyRemoveListener},
#endif // SF_NO_GC
    {"getAscii",        &KeyCtorFunction::KeyGetAscii},
    {"getCode",         &KeyCtorFunction::KeyGetCode},
    {"isDown",          &KeyCtorFunction::KeyIsDown},
    {"isToggled",       &KeyCtorFunction::KeyIsToggled},
    {0, 0}
};

KeyCtorFunction::KeyCtorFunction(ASStringContext *psc, MovieImpl* proot)
:   CFunctionObject(psc, GlobalCtor), 
    pMovieRoot(proot)
{
#ifdef GFX_AS_ENABLE_GC
    AsBroadcaster::Initialize(psc, this);
#endif // SF_NO_GC

    // constants
#define KEY_CONST(kname, key) SetConstMemberRaw(psc, #kname, Key::key)
    KEY_CONST(BACKSPACE, Backspace);
    KEY_CONST(CAPSLOCK, CapsLock);
    KEY_CONST(CONTROL, Control);
    KEY_CONST(DELETEKEY, Delete);
    KEY_CONST(DOWN, Down);
    KEY_CONST(END, End);
    KEY_CONST(ENTER, Return);
    KEY_CONST(ESCAPE, Escape);
    KEY_CONST(HOME, Home);
    KEY_CONST(INSERT, Insert);
    KEY_CONST(LEFT, Left);
    KEY_CONST(PGDN, PageDown);
    KEY_CONST(PGUP, PageUp);
    KEY_CONST(RIGHT, Right);
    KEY_CONST(SHIFT, Shift);
    KEY_CONST(SPACE, Space);
    KEY_CONST(TAB, Tab);
    KEY_CONST(UP, Up);

    // methods
    NameFunction::AddConstMembers(this, psc, StaticFunctionTable);

    proot->SetKeyboardListener(this);
}

void KeyCtorFunction::Update(const EventId& evt)
{
    SF_ASSERT(evt.KeyboardIndex < GFX_MAX_CONTROLLERS_SUPPORTED);
    States[evt.KeyboardIndex].LastKeyCode         = evt.KeyCode;
    States[evt.KeyboardIndex].LastAsciiCode       = (evt.AsciiCode) ? 
        evt.AsciiCode : evt.ConvertKeyCodeToAscii();
    States[evt.KeyboardIndex].LastWcharCode       = evt.WcharCode;
}

void KeyCtorFunction::NotifyListeners(InteractiveObject *, const EventId& evt)
{
    States[evt.KeyboardIndex].LastKeyCode         = evt.KeyCode;
    States[evt.KeyboardIndex].LastAsciiCode       = (evt.AsciiCode) ? 
        evt.AsciiCode : evt.ConvertKeyCodeToAscii();
    States[evt.KeyboardIndex].LastWcharCode       = evt.WcharCode;

#ifndef GFX_AS_ENABLE_GC
    // Non-CC version
    if (Listeners.GetSize() > 0)
    {            
        struct ListenersCopyStruct : public NewOverrideBase<StatMV_ActionScript_Mem>
        {
            ArrayLH<WeakPtr<RefCountWeakSupportImpl> >   ListenerWeakRefs;
            ArrayLH<ObjectInterface* >                  Listeners;

            ListenersCopyStruct(ArrayLH<WeakPtr<RefCountWeakSupportImpl> > &weak,
                ArrayLH<ObjectInterface*> &listeners)
                : ListenerWeakRefs(weak), Listeners(listeners) { }
        };

        // Copy of listeners needs to be created in AS heap (not on stack).
        ListenersCopyStruct* plcopy = SF_HEAP_NEW(psc->GetHeap()) ListenersCopyStruct(ListenerWeakRefs, Listeners);

        // Notify listeners.
        UPInt i;
        UPInt n = plcopy->Listeners.GetSize();
        for (i = 0; i < n; i++)
        {
            Ptr<RefCountWeakSupportImpl>  listernerRef = plcopy->ListenerWeakRefs[i];

            if (listernerRef)
            {
                Value            method;
                ObjectInterface* listener = plcopy->Listeners[i];
                ASStringContext *  psctemp = psc;
                if (listener->IsASCharacter())
                    psctemp = listener->ToASCharacter()->GetASEnvironment()->GetSC();

                if (listener
                    && listener->GetMemberRaw(psctemp, EventId(UByte(eventId)).GetFunctionName(psctemp), &method))
                {
                    // MA: Warning: environment should no longer be null.
                    // May be ok because function will substitute its own environment.
                    GAS_Invoke(method, NULL, listener, NULL /* or root? */, 0, 0, NULL);
                }
            }       
        }

        delete plcopy;
    }
#else // SF_NO_GC
    // CC version
    ASString handlerName = EventId_GetFunctionName(ToAS2Root(pMovieRoot)->GetStringManager(), EventId(UByte(evt.Id)));
    
    if (pMovieRoot)
    {
        Sprite* plevel = ToAS2Root(pMovieRoot)->GetLevelMovie(0);
        if (plevel)
        {
            Environment* penv = GFx::AS2::ToAvmSprite(plevel)->GetASEnvironment();
            if (penv)
            {
                int nargs = 0;
                if (penv->CheckExtensions())
                {
                    // push index of controller
                    penv->Push(evt.KeyboardIndex);
                    ++nargs;
                }
                AsBroadcaster::BroadcastMessage(penv, this, handlerName, nargs, penv->GetTopIndex());
                if (nargs)
                    penv->Drop(nargs);
            }
        }    
    }    
#endif // SF_NO_GC
} 

bool    KeyCtorFunction::IsKeyDown(int code, unsigned keyboardIndex)
{    
    KeyboardState* pkeyboard = pMovieRoot->GetKeyboardState(keyboardIndex);
    if (pkeyboard)
    {
        return pkeyboard->IsKeyDown(code);
    }
    return false;
}

bool    KeyCtorFunction::IsKeyToggled(int code, unsigned keyboardIndex)
{    
    KeyboardState* pkeyboard = pMovieRoot->GetKeyboardState(keyboardIndex);
    if (pkeyboard)
    {
        return pkeyboard->IsKeyToggled(code);
    }
    return false;
}

#ifndef GFX_AS_ENABLE_GC
// Non-CC methods

// Remove dead entries in the listeners list.  (Since
// we use WeakPtr's, listeners can disappear without
// notice.)
void    KeyCtorFunction::CleanupListeners()
{
    for (int i = (int)ListenerWeakRefs.GetSize() - 1; i >= 0; i--)
    {
        if (ListenerWeakRefs[i] == NULL)
        {
            Listeners.RemoveAt(i);
            ListenerWeakRefs.RemoveAt(i);
        }
    }
}

void    KeyCtorFunction::AddListener(RefCountWeakSupportImpl *pref, ObjectInterface* listener)
{
    CleanupListeners();

    for (UPInt i = 0, n = Listeners.GetSize(); i < n; i++)
    {
        if (Listeners[i] == listener)
        {
            // Already in the list.
            return;
        }
    }

    Listeners.PushBack(listener);
    ListenerWeakRefs.PushBack(pref);
}

void    KeyCtorFunction::RemoveListener(RefCountWeakSupportImpl *pref, ObjectInterface* listener)
{
    SF_UNUSED(pref);

    CleanupListeners();

    for (int i = (int)Listeners.GetSize() - 1; i >= 0; i--)
    {
        if (Listeners[i] == listener)
        {
            Listeners.RemoveAt(i);
            ListenerWeakRefs.RemoveAt(i);
        }
    }
}

// support for _listeners
bool KeyCtorFunction::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if (penv && (name == penv->GetBuiltin(ASBuiltin__listeners)))
    {
        // Create a temporary ArrayObject...
        Ptr<Object> obj = *penv->OperatorNew(penv->GetBuiltin(ASBuiltin_Array));
        if (obj)
        {
            ArrayObject* arrObj = static_cast<ArrayObject*>(obj.GetPtr());

            // Iterate through listeners.
            UPInt i;
            UPInt n = Listeners.GetSize();
            for (i = 0; i < n; i++)
            {
                Ptr<RefCountWeakSupportImpl>  listernerRef = ListenerWeakRefs[i];

                if (listernerRef)
                {
                    ObjectInterface* listener = Listeners[i];
                    Value v;
                    if (!listener->IsASCharacter())
                    {
                        Object* obj = static_cast<Object*>(listener);
                        v.SetAsObject(obj);
                    }
                    else
                    {
                        v.SetAsCharacter(listener->ToASCharacter());
                    }
                    arrObj->PushBack(v);
                }       
            }

        }
        val->SetAsObject(obj);
    }
    return Object::GetMember(penv, name, val);
}

// Add a Listener (first arg is object reference) to our list.
// Listeners will have "onKeyDown" and "onKeyUp" methods
// called on them when a key changes state.
void    KeyCtorFunction::KeyAddListener(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("KeyAddListener needs one Argument (the listener object)");
        return;
    }

    Object*          listenerObj     = fn.Arg(0).IsObject() ? fn.Arg(0).ToObject(fn.Env) : 0;
    InteractiveObject*     listenerChar    = fn.Arg(0).ToASCharacter(fn.Env);
    ObjectInterface* listener;
    RefCountWeakSupportImpl*  listenerRef;

    if (listenerObj)
    {
        listener    = listenerObj;
        listenerRef = listenerObj;  
    }
    else
    {
        listener    = listenerChar;
        listenerRef = listenerChar; 
    }

    if (listener == NULL)
    {
        fn.Env->LogScriptError("KeyAddListener passed a NULL object; ignored");
        return;
    }

    KeyCtorFunction* ko = (KeyCtorFunction*)fn.ThisPtr;
    SF_ASSERT(ko);

    ko->AddListener(listenerRef, listener);
}

// Remove a previously-added listener.
void    KeyCtorFunction::KeyRemoveListener(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("KeyRemoveListener needs one Argument (the listener object)");
        return;
    }

    Object*          listenerObj     = fn.Arg(0).IsObject() ? fn.Arg(0).ToObject(fn.Env) : 0;
    InteractiveObject*     listenerChar    = fn.Arg(0).ToASCharacter(fn.Env);
    ObjectInterface* listener;
    RefCountWeakSupportImpl*  listenerRef;

    if (listenerObj)
    {
        listener    = listenerObj;
        listenerRef = listenerObj;  
    }
    else
    {
        listener    = listenerChar;
        listenerRef = listenerChar; 
    }

    if (listener == NULL)
    {
        fn.Env->LogScriptError("KeyRemoveListener passed a NULL object; ignored");
        return;
    }

    KeyCtorFunction* ko = (KeyCtorFunction*)fn.ThisPtr;
    SF_ASSERT(ko);

    ko->RemoveListener(listenerRef, listener);
}
#endif // SF_NO_GC

// Return the ascii value of the last key pressed.
void    KeyCtorFunction::KeyGetAscii(const FnCall& fn)
{
    KeyCtorFunction* ko = (KeyCtorFunction*)fn.ThisPtr;
    SF_ASSERT(ko);

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 1)
            controllerIdx = fn.Arg(0).ToUInt32(fn.Env);
    }

    fn.Result->SetInt(ko->GetLastAsciiCode(controllerIdx));
}

// Returns the keycode of the last key pressed.
void    KeyCtorFunction::KeyGetCode(const FnCall& fn)
{
    KeyCtorFunction* ko = (KeyCtorFunction*)fn.ThisPtr;
    SF_ASSERT(ko);

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 1)
            controllerIdx = fn.Arg(0).ToUInt32(fn.Env);
    }

    fn.Result->SetInt(ko->GetLastKeyCode(controllerIdx));
}

// Return true if the Specified (first arg keycode) key is pressed.
void    KeyCtorFunction::KeyIsDown(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("KeyIsDown needs one Argument (the key code)");
        return;
    }

    int code = fn.Arg(0).ToInt32(fn.Env);

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 2)
            controllerIdx = fn.Arg(1).ToUInt32(fn.Env);
    }

    KeyCtorFunction* ko = (KeyCtorFunction*)fn.ThisPtr;
    SF_ASSERT(ko);

    fn.Result->SetBool(ko->IsKeyDown(code, controllerIdx));
}

// Given the keycode of NUM_LOCK or CAPSLOCK, returns true if
// the associated state is on.
void    KeyCtorFunction::KeyIsToggled(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("KeyIsToggled needs one Argument (the key code)");
        return;
    }

    int code = fn.Arg(0).ToInt32(fn.Env);

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 2)
            controllerIdx = fn.Arg(1).ToUInt32(fn.Env);
    }

    KeyCtorFunction* ko = (KeyCtorFunction*)fn.ThisPtr;
    SF_ASSERT(ko);

    fn.Result->SetBool(ko->IsKeyToggled(code, controllerIdx));
}

void KeyCtorFunction::GlobalCtor(const FnCall& fn)
{
    fn.Result->SetNull();
}
#else

void GASKey_DummyFunction() {}   // Exists to quelch compiler warning

#endif // SF_NO_KEYBOARD_SUPPORT

}}} //SF::GFx::AS2
