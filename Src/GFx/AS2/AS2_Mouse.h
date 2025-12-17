/**************************************************************************

Filename    :   AS2_Mouse.h
Content     :   Implementation of Mouse class
Created     :   November, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_MOUSE_H
#define INC_SF_GFX_MOUSE_H

#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/AS2_MovieRoot.h"

namespace Scaleform { namespace GFx { namespace AS2 {

#ifdef GFX_AS2_ENABLE_MOUSE
// ***** Declared Classes
class AsBroadcaster;
class AsBroadcasterProto;
class AsBroadcasterCtorFunction;

// ***** External Classes
class ArrayObject;
class Environment;



class MouseObject : public Object
{
protected:

    void commonInit (Environment* penv);
    
    MouseObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    MouseObject (Environment* penv);
};

class MouseProto : public Prototype<MouseObject>
{
public:
    MouseProto (ASStringContext* psc, Object* pprototype, const FunctionRef& constructor);
};

class MouseCtorFunction : public CFunctionObject, public GFx::AS2::MouseListener
{
    static int   PushListenersParams(Environment* penv, 
                                     unsigned mouseIndex, 
                                     ASBuiltinType eventName, 
                                     const Value& eventMethod, 
                                     const ASString* ptargetName, 
                                     unsigned button, 
                                     int delta,
                                     bool dblClick);

    void NotifyListeners(Environment *penv, unsigned mouseIndex, 
                         ASBuiltinType eventName, const ASString* ptargetName = 0, 
                         unsigned button = 0, int delta = 0, bool dblClick = false) const;
#ifdef GFX_AS_ENABLE_GC
protected:
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor> void ForEachChild_GC(Collector* prcc) const;
    virtual                  void ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const;

    virtual void Finalize_GC();

    void UpdateListenersArray(ASStringContext *psc, Environment* penv = NULL);
#endif //SF_NO_GC
protected:
    enum 
    {
        Mouse_LEFT = 1,
        Mouse_RIGHT = 2,
        Mouse_MIDDLE = 3
    };
#ifndef GFX_AS_ENABLE_GC
    // Hack to store both WeakPtrs and correct interfaces.
    // If weak ptr is null, the interface pointer is unusable.
    // This is not necessary if cycle collector is used. In this case AsBroadcaster mechanism 
    // is used.
    ArrayLH<WeakPtr<RefCountWeakSupportImpl> >  ListenerWeakRefs;
    ArrayLH<ObjectInterface*>                      Listeners;
#else
    // array of listeners; it is actually stored as a member of the Mouse object
    // (populated by AsBroadcaster). This member is necessary only to implement 
    // method "IsEmpty" to avoid unnecessary processing if there are no
    // listeners. It is updated by UpdateListenersArray method.
    Ptr<ArrayObject>                    pListenersArray;

    class LocalInvokeCallback : public AsBroadcaster::InvokeCallback
    {
    public:
        int             MouseIndex;
        ASBuiltinType   EventName;
        const ASString* pTargetName; 
        unsigned        Button; 
        int             Delta;
        bool            DoubleClick;

        virtual void Invoke(Environment* penv, ObjectInterface* pthis, const FunctionRef& method)
        {
            int nArgs = PushListenersParams(penv, MouseIndex, EventName, method, pTargetName, Button, Delta, DoubleClick);
            if (nArgs >= 0)
            {
                Value result;
                method.Invoke(FnCall(&result, pthis, penv, nArgs, penv->GetTopIndex()));
                penv->Drop(nArgs);
            }
        }
    };
#endif // SF_NO_GC
    mutable Point<int>  LastMousePos;
    mutable UInt32      LastClickTime;
    FunctionRef         SetCursorTypeFunc;
    bool                MouseExtensions;

    virtual void OnMouseMove(Environment *penv, unsigned mouseIndex) const;
    virtual void OnMouseDown(Environment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const;
    virtual void OnMouseUp(Environment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const;
    virtual void OnMouseWheel(Environment *penv, unsigned mouseIndex, int sdelta, InteractiveObject* ptarget) const;
    virtual bool IsEmpty() const;

    static const NameFunction StaticFunctionTable[];

#ifndef GFX_AS_ENABLE_GC
    void CleanupListeners();
    static void AddListener(const FnCall& fn);
    static void RemoveListener(const FnCall& fn);
#endif // SF_NO_GC
    static void Hide(const FnCall& fn);
    static void Show(const FnCall& fn);

    // extension methods
    static void SetCursorType(const FnCall& fn);
    // Returns a character at the specified location. Four versions of this method: no params, 
    // 1 param, 2 params and 3 params. Method with no params just look for ANY character under 
    // the mouse pointer (with or without button handlers). Method with 1 parameter takes a boolean 
    // value as the parameter. It also look for character under the mouse pointer, though the 
    // parameter indicates type of characters: true - ANY character, false - only with button 
    // handlers. Version with two parameters takes custom X and Y coordinates in _root coordinate 
    // space and look for ANY character at these coordinates. Version with three params is same 
    // as version with two, the third parameter (boolean) indicates the type of character
    // (any/with button handler).
    static void GetTopMostEntity(const FnCall& fn);

    // returns position of mouse cursor, index of mouse is an optional parameter.
    // Returning value is Point object (or Object, if Point class is not included).
    // Undefined, if error occurred.
    static void GetPosition(const FnCall& fn);

    // returns status of mouse buttons, index of mouse is an optional parameter.
    // Returning value is a number, bit 0 indicates state of left button, 
    // bit 1 - right one, bit 2 - middle.
    // Undefined, if error occurred.
    static void GetButtonsState(const FnCall& fn);

    static void         GlobalCtor(const FnCall& fn);
    virtual Object*  CreateNewObject(Environment*) const { return 0; }
public:
    MouseCtorFunction (ASStringContext *psc, MovieRoot* proot);

    virtual bool    GetMember(Environment *penv, const ASString& name, Value* val);
    virtual bool    SetMember(Environment *penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());

    static bool     SetCursorType(MovieImpl* proot, unsigned mouseIndex, unsigned cursorType);
    bool            HasOverloadedCursorTypeFunction(ASStringContext* psc) const;

    static FunctionRef Register(GlobalContext* pgc);
};

#endif //SF_NO_FXPLAYER_AS_MOUSE

}}} //SF::GFx::AS2
#endif // INC_SF_GFX_MOUSE_H
