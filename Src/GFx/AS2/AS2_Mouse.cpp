/**************************************************************************

Filename    :   AS2_Mouse.cpp
Content     :   Implementation of Mouse class
Created     :   November, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_String.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_Mouse.h"
#include "GFx/AS2/AS2_PointObject.h"
#include "Kernel/SF_UTF8Util.h"

namespace Scaleform { namespace GFx { namespace AS2 {

#ifdef GFX_AS2_ENABLE_MOUSE

MouseObject::MouseObject(Environment* penv)
: Object(penv)
{
    commonInit(penv);
}

void MouseObject::commonInit (Environment* penv)
{
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_Mouse));
}
    
//////////////////////////////////////////
//
static void GFx_MouseFuncStub(const FnCall& fn)
{
    fn.Result->SetUndefined();
}

static const NameFunction GAS_MouseFunctionTable[] = 
{
    { "addListener",        &GFx_MouseFuncStub },
    { "removeListener",     &GFx_MouseFuncStub },
    { "show",               &GFx_MouseFuncStub },
    { "hide",               &GFx_MouseFuncStub },
    { 0, 0 }
};

MouseProto::MouseProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<MouseObject>(psc, pprototype, constructor)
{
    InitFunctionMembers (psc, GAS_MouseFunctionTable);
}

//////////////////
const NameFunction MouseCtorFunction::StaticFunctionTable[] = 
{
#ifndef GFX_AS_ENABLE_GC
    { "addListener",        &MouseCtorFunction::AddListener        },
    { "removeListener",     &MouseCtorFunction::RemoveListener     },
#endif // SF_NO_GC
    { "show",               &MouseCtorFunction::Show               },
    { "hide",               &MouseCtorFunction::Hide               },
    { 0, 0 }
};

MouseCtorFunction::MouseCtorFunction (ASStringContext *psc, MovieRoot* proot) :
    CFunctionObject(psc, GlobalCtor)
{
    SF_ASSERT(proot->pASMouseListener == 0); // shouldn't be set!
    proot->pASMouseListener = this;

#ifdef GFX_AS_ENABLE_GC
    AsBroadcaster::Initialize(psc, this);
    UpdateListenersArray(psc);
#endif // SF_NO_GC

    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);

    SetCursorTypeFunc = Value(psc, MouseCtorFunction::SetCursorType).ToFunction(NULL);
    LastClickTime = 0;
}

#ifdef GFX_AS_ENABLE_GC
// CC version
template <class Functor>
void MouseCtorFunction::ForEachChild_GC(Collector* prcc) const
{
    SetCursorTypeFunc.template ForEachChild_GC<Functor>(prcc);
    if (pListenersArray)
        Functor::Call(prcc, pListenersArray);
    CFunctionObject::template ForEachChild_GC<Functor>(prcc);
}
SF_GC_PREGEN_FOR_EACH_CHILD(MouseCtorFunction)

void MouseCtorFunction::ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
{
    ASRefCountBaseType::CallForEachChild<MouseCtorFunction>(prcc, operation);
}

void MouseCtorFunction::Finalize_GC()
{
    CFunctionObject::Finalize_GC();
    SetCursorTypeFunc.Finalize_GC();
}

bool MouseCtorFunction::IsEmpty() const 
{ 
    if (pListenersArray)
        return (pListenersArray->GetSize() == 0);
    return false;
}

void MouseCtorFunction::UpdateListenersArray(ASStringContext *psc, Environment* penv)
{
    Value listenersVal;
    if (GetMemberRaw(psc,
        psc->GetBuiltin(ASBuiltin__listeners), &listenersVal))
    {
        Object* pobj = listenersVal.ToObject(penv);
        if (pobj && pobj->GetObjectType() == Object_Array)
            pListenersArray = static_cast<ArrayObject*>(pobj);
        else
            pListenersArray = NULL;
    }
}

#else 
// non-CC methods
// Remove dead entries in the listeners list.  (Since
// we use WeakPtr's, listeners can disappear without
// notice.)
void    MouseCtorFunction::CleanupListeners()
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

void    MouseCtorFunction::AddListener(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Mouse.addListener needs one argument (the listener object)");
        return;
    }

    Object*          listenerObj     = fn.Arg(0).IsObject() ? fn.Arg(0).ToObject(fn.Env) : 0;
    InteractiveObject*     listenerChar    = fn.Arg(0).ToASCharacter(fn.Env);
    ObjectInterface* listener;
    RefCountWeakSupportImpl* listenerRef;

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
        fn.Env->LogScriptError("Mouse.addListener passed a NULL object; ignored");
        return;
    }

    MouseCtorFunction* mo = static_cast<MouseCtorFunction*>(fn.ThisPtr);
    SF_ASSERT(mo);

    mo->CleanupListeners();

    for (UPInt i = 0, n = mo->Listeners.GetSize(); i < n; i++)
    {
        if (mo->Listeners[i] == listener)
        {
            // Already in the list.
            return;
        }
    }

    mo->Listeners.PushBack(listener);
    mo->ListenerWeakRefs.PushBack(listenerRef);
}

void    MouseCtorFunction::RemoveListener(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Mouse.removeListener needs one Argument (the listener object)");
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
        fn.Env->LogScriptError("Mouse.removeListener passed a NULL object; ignored");
        return;
    }

    MouseCtorFunction* mo = static_cast<MouseCtorFunction*>(fn.ThisPtr);
    SF_ASSERT(mo);

    mo->CleanupListeners();

    for (SPInt i = (SPInt)mo->Listeners.GetSize() - 1; i >= 0; i--)
    {
        if (mo->Listeners[i] == listener)
        {
            mo->Listeners.RemoveAt(i);
            mo->ListenerWeakRefs.RemoveAt(i);
        }
    }
}

bool MouseCtorFunction::IsEmpty() const 
{ 
    return Listeners.GetSize() == 0; 
}
#endif //SF_NO_GC

// Pushes parameters for listeners into stack; returns number of pushed params.
int MouseCtorFunction::PushListenersParams(Environment* penv, 
                                              unsigned mouseIndex, 
                                              ASBuiltinType eventName, 
                                              const Value& eventMethod, 
                                              const ASString* ptargetName, 
                                              unsigned button, 
                                              int delta,
                                              bool dblClick)
{
    SF_ASSERT(penv);

    bool noExtraParams = !penv->CheckExtensions();
    if (penv->CheckExtensions() && button > 0 && 
        (eventName == ASBuiltin_onMouseDown || eventName == ASBuiltin_onMouseUp))
    {
        // check for number of parameters for onMouseDown/onMouseUp handler functions;
        // if more than zero - we can call them for right and middle buttons and we can
        // pass extra parameters for the left button's handler. This is done for compatibility.
        FunctionRef mfref = eventMethod.ToFunction(penv);
        if (!mfref.IsNull())
        {
            if (mfref->GetNumArgs() <= 0)
            {
                if (button > 1)
                    return -1; // do not call handlers for right/middle buttons, since handler has 0 parameters
                noExtraParams = true; // do not pass extra parameters in left button's handler
            }
        }
        else
            return -1; // method is empty! (assertion?)
    }
    int nArgs = 0;
    if (penv->CheckExtensions() && !noExtraParams) 
    {
        if (eventName == ASBuiltin_onMouseDown && dblClick)
        {
            // onMouseDown, the last parameter indicates doubleClick, if true.
            penv->Push(dblClick);
            ++nArgs;
        }
        const MouseState* pms = penv->GetMovieImpl()->GetMouseState(mouseIndex);
        PointF pt = pms->GetLastPosition();
        penv->Push((Number)TwipsToPixels(floor(pt.y + 0.5)));
        penv->Push((Number)TwipsToPixels(floor(pt.x + 0.5)));
        penv->Push((int)mouseIndex);
        nArgs += 3;
    }
    if (eventName != ASBuiltin_onMouseMove)
    {
        if (ptargetName && (eventName == ASBuiltin_onMouseWheel || !noExtraParams))
        {
            // push target for onMouseWheel and onMouseDown/Up, if allowed
            penv->Push(*ptargetName);
            ++nArgs;
        }
        else if (nArgs > 0)
        {
            // push null as a placeholder
            penv->Push(Value::NULLTYPE);
            ++nArgs;
        }
    }
    switch(eventName)
    {
    case ASBuiltin_onMouseWheel:
        penv->Push(delta);
        ++nArgs;
        break;
    case ASBuiltin_onMouseDown:
    case ASBuiltin_onMouseUp:
        if (button && !noExtraParams)
        {
            penv->Push(Number(button));
            ++nArgs;
        }
        else if (nArgs > 0)
        {
            // push null as a placeholder
            penv->Push(Value::NULLTYPE);
            ++nArgs;
        }
        break;

    case ASBuiltin_onMouseMove:
        // pass (x, y, mouseIndex) as parameters of mouse move handler, if extensions
        // are on.
        break;

    default:
        break;
    }
    return nArgs;
}

void MouseCtorFunction::NotifyListeners
    (Environment *penv, unsigned mouseIndex, ASBuiltinType eventName, 
     const ASString* ptargetName, unsigned button, int delta, bool dblClick) const
{
    // GFx extends functionality of mouse listener handlers:
    // onMouseMove(mouseIndex:Number, x:Number, y:Number)
    // onMouseDown(button:Number, target:Object, mouseIndex:Number, x:Number, y:Number)
    // onMouseUp  (button:Number, target:Object, mouseIndex:Number, x:Number, y:Number)
    // onMouseWheel(delta:Number, target:Object, mouseIndex:Number, x:Number, y:Number)
    // Note, onMouseDown/onMouseUp might be invoked for left and middle buttons, but
    // AS functions for these handlers should be declared with parameters. Thus,
    // if you have empty parameter list for onMouseDown/Up it will be invoked only
    // for left mouse button.
#ifndef GFX_AS_ENABLE_GC
    // Non-CC code
    if (Listeners.GetSize() > 0)
    {
        struct ListenersCopyStruct : public NewOverrideBase<StatMV_ActionScript_Mem>
        {
            ArrayLH<WeakPtr<RefCountWeakSupportImpl> >   ListenerWeakRefs;
            ArrayLH<ObjectInterface*>                       Listeners;

            ListenersCopyStruct(const ArrayLH<WeakPtr<RefCountWeakSupportImpl> > &weak,
                                const ArrayLH<ObjectInterface*> &listeners)
                : ListenerWeakRefs(weak), Listeners(listeners) { }
        };

        // Copy of listeners needs to be created in AS heap (not on stack).
        ListenersCopyStruct* plcopy = 
            SF_HEAP_NEW(penv->GetHeap()) ListenersCopyStruct(ListenerWeakRefs, Listeners);

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
                Environment *    penvtemp = penv;
                if (listener->IsASCharacter())
                    penvtemp = listener->ToASCharacter()->GetASEnvironment();
                ASStringContext *  psc = penvtemp->GetSC(); 
                if (listener
                    && listener->GetMemberRaw(psc, psc->GetBuiltin(eventName), &method))
                {
                    SF_ASSERT(penvtemp);

                    int nArgs = PushListenersParams(penvtemp, mouseIndex, eventName, method, ptargetName, button, delta, dblClick);
                    if (nArgs >= 0)
                    {
                        GAS_Invoke(method, NULL, listener, penvtemp, nArgs, penvtemp->GetTopIndex(), NULL);
                        penvtemp->Drop(nArgs);
                    }
                }
            }    
        }

        delete plcopy;
    }
#else // SF_NO_GC
    // CC version
    LocalInvokeCallback Callback;
    Callback.MouseIndex     = mouseIndex;
    Callback.EventName      = eventName;
    Callback.pTargetName    = ptargetName;
    Callback.Button         = button;
    Callback.Delta          = delta;
    Callback.DoubleClick    = dblClick;
    AsBroadcaster::BroadcastMessageWithCallback(penv, const_cast<MouseCtorFunction*>(this), 
        penv->GetBuiltin(eventName), &Callback);
#endif // SF_NO_GC
} 

void MouseCtorFunction::OnMouseMove(Environment *penv, unsigned mouseIndex) const
{
    NotifyListeners(penv, mouseIndex, ASBuiltin_onMouseMove);
}

void MouseCtorFunction::OnMouseDown(Environment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const
{
    bool doubleClicked = false;
    if (penv->CheckExtensions())
    {
        // detect double click
        UInt32 tm = (UInt32)(Timer::GetTicks() / 1000);

        const MouseState* pms = penv->GetMovieImpl()->GetMouseState(mouseIndex);
        PointF pt = pms->GetLastPosition();
        int ax = (int)TwipsToPixels(pt.x);
        int ay = (int)TwipsToPixels(pt.y);

        // detect double click
        if (tm <= LastClickTime + GFX_DOUBLE_CLICK_TIME_MS && LastMousePos.x == ax && LastMousePos.y == ay)
        {
            doubleClicked = true;
            //printf("DOUBLE CLICK!\n");
        }
        //printf("new x = %f    y = %f     tm = %d\n", ax, ay, tm);
        LastMousePos.x = ax;
        LastMousePos.y = ay;
        LastClickTime  = tm;
    }
    if (ptarget)
    {
        ASString stringVal = ptarget->GetCharacterHandle()->GetNamePath();
        NotifyListeners(penv, mouseIndex, ASBuiltin_onMouseDown, &stringVal, button, 0, doubleClicked);
    }
    else
        NotifyListeners(penv, mouseIndex, ASBuiltin_onMouseDown, NULL, button, 0, doubleClicked);
}

void MouseCtorFunction::OnMouseUp(Environment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const
{
    if (ptarget)
    {
        ASString stringVal = ptarget->GetCharacterHandle()->GetNamePath();
        NotifyListeners(penv, mouseIndex, ASBuiltin_onMouseUp, &stringVal, button);
    }
    else
        NotifyListeners(penv, mouseIndex, ASBuiltin_onMouseUp, NULL, button);
}

void MouseCtorFunction::OnMouseWheel(Environment *penv, unsigned mouseIndex, int sdelta, InteractiveObject* ptarget) const
{
    if (ptarget)
    {
        ASString stringVal = ptarget->GetCharacterHandle()->GetNamePath();
        NotifyListeners(penv, mouseIndex, ASBuiltin_onMouseWheel, &stringVal, 0, sdelta);
    }
    else
        NotifyListeners(penv, mouseIndex, ASBuiltin_onMouseWheel, 0, 0, sdelta);
}

void    MouseCtorFunction::Show(const FnCall& fn)
{
    fn.Result->SetUndefined();
    MovieImpl* proot = fn.Env->GetMovieImpl();
    if (proot->pUserEventHandler)
    {
        unsigned mouseIndex = 0;
        if (fn.NArgs >= 1)
            mouseIndex = fn.Arg(0).ToUInt32(fn.Env);
        proot->pUserEventHandler->HandleEvent(proot, MouseCursorEvent(Event::DoShowMouse, mouseIndex));
    }
    else
        fn.Env->LogScriptWarning("No user event handler interface is installed; Mouse.show failed.");
}

void    MouseCtorFunction::Hide(const FnCall& fn)
{
    fn.Result->SetUndefined();
    MovieImpl* proot = fn.Env->GetMovieImpl();
    if (proot->pUserEventHandler)
    {
        unsigned mouseIndex = 0;
        if (fn.NArgs >= 1)
            mouseIndex = fn.Arg(0).ToUInt32(fn.Env);
        proot->pUserEventHandler->HandleEvent(proot, MouseCursorEvent(Event::DoHideMouse, mouseIndex));
    }
    else
        fn.Env->LogScriptWarning("No user event handler interface is installed; Mouse.hide failed.");
}

void    MouseCtorFunction::SetCursorType(const FnCall& fn)
{
    fn.Result->SetUndefined();
    MovieImpl* proot = fn.Env->GetMovieImpl();
    unsigned cursorShapeId = MouseCursorEvent::ARROW;
    if (fn.NArgs > 0)
        cursorShapeId = (unsigned)fn.Arg(0).ToNumber(fn.Env);
    unsigned mouseIndex = 0;
    if (fn.NArgs >= 2)
        mouseIndex = (unsigned)fn.Arg(1).ToNumber(fn.Env);
    if (!SetCursorType(proot, mouseIndex, cursorShapeId))
        fn.Env->LogScriptWarning("No user event handler interface is installed; Mouse.setCursorType failed.");
}

bool    MouseCtorFunction::SetCursorType(MovieImpl* proot, unsigned mouseIndex, unsigned cursorType)
{
    SF_ASSERT(proot);

    if (proot->pUserEventHandler)
    {
        proot->pUserEventHandler->HandleEvent(proot, 
            MouseCursorEvent((MouseCursorEvent::CursorShapeType)cursorType, mouseIndex));
    }
    else
        return false;
    return true;
}

void    MouseCtorFunction::GetTopMostEntity(const FnCall& fn)
{
    fn.Result->SetUndefined();
    MovieImpl* proot = fn.Env->GetMovieImpl();
    PointF mousePos;
    bool testAll = true;
    int params = 0;
    unsigned mouseIndex = 0;

    if (fn.NArgs >= 1)
    {
        if (fn.Arg(0).IsBoolean()) // (testAll:boolean, [mouseIndex:int]) variant
        {
            testAll = fn.Arg(0).ToBool(fn.Env);
            if (fn.NArgs >= 2)
                mouseIndex = (int)fn.Arg(1).ToNumber(fn.Env);
            params  = 2;
        }
        else if (fn.NArgs == 1) // (mouseIndex:int) variant
        {
            mouseIndex = (int)fn.Arg(0).ToNumber(fn.Env);
            params  = 1;
        }
        else if (fn.NArgs >= 2) // (x:Number, y:Number, [testAll:boolean])
        {
            if (fn.NArgs >= 3)
                testAll = fn.Arg(2).ToBool(fn.Env);
            params = 3;
        }
    }

    if (params <= 2)
    {
        // one parameter version: parameter is a boolean value, indicates
        // do we need to test all characters (param is true) or only ones with button event handlers (false).
        // Coordinates - current mouse cursor coordinates
        // optional second parameter is the index of mouse.

        // no parameters version: takes current mouse (index = 0) coordinates, testing all.

        if (mouseIndex >= proot->GetMouseCursorCount())
            return; // invalid index of mouse
        SF_ASSERT(proot->GetMouseState(mouseIndex)); // only first mouse is supported for now
        mousePos = proot->GetMouseState(mouseIndex)->GetLastPosition();
    }
    else 
    {
        // three parameters version: _x and _y in _root coordinates, optional boolean - testAll
        float x = (float)PixelsToTwips(fn.Arg(0).ToNumber(fn.Env));
        float y = (float)PixelsToTwips(fn.Arg(1).ToNumber(fn.Env));

        if (proot->GetMainContainer())
        {
            // x and y vars are in _root coords. Need to transform them
            // into world coords
            Matrix2F m = proot->GetMainContainer()->GetWorldMatrix();
            PointF a(x, y);
            m.Transform(&mousePos, a);
        }
        else
            return;
    }
    InteractiveObject* ptopCh = proot->GetTopMostEntity(mousePos, mouseIndex, testAll);
    if (ptopCh)
    {
        fn.Result->SetAsCharacter(ptopCh);
    }
}

// returns position of mouse cursor, index of mouse is an optional parameter.
// Returning value is Point object (or Object, if Point class is not included).
// Undefined, if error occurred.
void    MouseCtorFunction::GetPosition(const FnCall& fn)
{
    fn.Result->SetUndefined();
    MovieImpl* proot = fn.Env->GetMovieImpl();
    PointF mousePos;
    unsigned mouseIndex = 0;

    if (fn.NArgs > 0)
        mouseIndex = (int)fn.Arg(0).ToNumber(fn.Env);
    if (mouseIndex >= proot->GetMouseCursorCount())
        return; // invalid index of mouse
    mousePos = proot->GetMouseState(mouseIndex)->GetLastPosition();
#ifdef GFX_AS2_ENABLE_POINT
    Ptr<PointObject> ptObj = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);
    GASPoint pt(TwipsToPixels(floor(mousePos.x + 0.5)), 
                TwipsToPixels(floor(mousePos.y + 0.5))); 
    ptObj->SetProperties(fn.Env, pt);
#else
    Ptr<Object> ptObj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
    ASStringContext *psc = fn.Env->GetSC();
    ptObj->SetConstMemberRaw(psc, "x", TwipsToPixels(floor(mousePos.x + 0.5)));
    ptObj->SetConstMemberRaw(psc, "y", TwipsToPixels(floor(mousePos.y + 0.5)));
#endif
    fn.Result->SetAsObject(ptObj);  
}

// returns status of mouse buttons, index of mouse is an optional parameter.
// Returning value is a number, bit 0 indicates state of left button, 
// bit 1 - right one, bit 2 - middle.
// Undefined, if error occurred.
void    MouseCtorFunction::GetButtonsState(const FnCall& fn)
{
    fn.Result->SetUndefined();
    MovieImpl* proot = fn.Env->GetMovieImpl();
    unsigned mouseIndex = 0;

    if (fn.NArgs > 0)
        mouseIndex = (int)fn.Arg(0).ToNumber(fn.Env);
    if (mouseIndex >= proot->GetMouseCursorCount())
        return; // invalid index of mouse
    unsigned state = proot->GetMouseState(mouseIndex)->GetButtonsState();
    fn.Result->SetNumber((Number)state);  
}

bool    MouseCtorFunction::SetMember
    (Environment* penv, const ASString& name, const Value& val, const PropFlags& flags)
{
    // Extensions are always case-insensitive
    if (name == penv->GetBuiltin(ASBuiltin_setCursorType))
    {
        if (penv->CheckExtensions())
        {
            SetCursorTypeFunc = val.ToFunction(penv);
            G_SetFlag<MovieImpl::Flag_SetCursorTypeFuncOverloaded>
                (penv->GetMovieImpl()->Flags, HasOverloadedCursorTypeFunction(penv->GetSC()));
        }
    }
#ifdef GFX_AS_ENABLE_GC
    else if (name == penv->GetBuiltin(ASBuiltin__listeners))
    {
        // need to intercept assignment of _listeners (in the case, if 
        // somebody assigns a new array to _listeners) in order to report
        // correctly is the array empty or not (IsEmpty)
        bool rv = FunctionObject::SetMember(penv, name, val, flags);
        UpdateListenersArray(penv->GetSC(), penv);
        return rv;
    }
#endif // SF_NO_GC
    return FunctionObject::SetMember(penv, name, val, flags);
}

bool    MouseCtorFunction::GetMember(Environment* penv, const ASString& name, Value* pval)
{
    if (penv->CheckExtensions())
    {
        // Extensions are always case-insensitive
        if (name == penv->GetBuiltin(ASBuiltin_setCursorType))
        {
            pval->SetAsFunction(SetCursorTypeFunc);
            return true;
        }
        else if (name == penv->GetBuiltin(ASBuiltin_LEFT))
        {
            pval->SetNumber(Mouse_LEFT);
        }
        else if (name == penv->GetBuiltin(ASBuiltin_RIGHT))
        {
            pval->SetNumber(Mouse_RIGHT);
        }
        else if (name == penv->GetBuiltin(ASBuiltin_MIDDLE))
        {
            pval->SetNumber(Mouse_MIDDLE);
        }
        else if (name == penv->GetBuiltin(ASBuiltin_ARROW))
        {
            pval->SetNumber(MouseCursorEvent::ARROW);
        }
        else if (name == penv->GetBuiltin(ASBuiltin_HAND))
        {
            pval->SetNumber(MouseCursorEvent::HAND);
        }
        else if (name == penv->GetBuiltin(ASBuiltin_IBEAM))
        {
            pval->SetNumber(MouseCursorEvent::IBEAM);
        }
        else if (name == "getTopMostEntity")
        {
            *pval = Value(penv->GetSC(), MouseCtorFunction::GetTopMostEntity);
            return true;
        }
        else if (name == "getPosition")
        {
            *pval = Value(penv->GetSC(), MouseCtorFunction::GetPosition);
            return true;
        }
        else if (name == "getButtonsState")
        {
            *pval = Value(penv->GetSC(), MouseCtorFunction::GetButtonsState);
            return true;
        }
    }
    return Object::GetMember(penv, name, pval);
}

bool MouseCtorFunction::HasOverloadedCursorTypeFunction(ASStringContext* psc) const
{
    return SetCursorTypeFunc != Value(psc, MouseCtorFunction::SetCursorType).ToFunction(NULL);
}

void MouseCtorFunction::GlobalCtor(const FnCall& fn)
{
    fn.Result->SetNull();
}

FunctionRef MouseCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) MouseCtorFunction(&sc, pgc->GetAS2Root()));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) MouseProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Mouse, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Mouse), Value(ctor));
    return ctor;
}
#else

void GASMouse_DummyFunction() {}   // Exists to quelch compiler warning

#endif // SF_NO_FXPLAYER_AS_MOUSE

}}} //SF::GFx::AS2

