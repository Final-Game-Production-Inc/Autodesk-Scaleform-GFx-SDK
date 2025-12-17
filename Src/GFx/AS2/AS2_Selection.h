/**************************************************************************

Filename    :   AS2_Selection.h
Content     :   Implementation of Selection class
Created     :   February, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_SELECTION_H
#define INC_SF_GFX_SELECTION_H

#include "GFxConfig.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_ObjectProto.h"


namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class AsBroadcaster;
class AsBroadcasterProto;
class AsBroadcasterCtorFunction;

// ***** External Classes
class ArrayObject;
class Environment;

class Selection : public Object
{
protected:
    Selection(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

    void commonInit (Environment* penv);

    static void DoTransferFocus(const FnCall& fn);
public:
    Selection(Environment* penv);

    static void QueueSetFocus(Environment* penv, InteractiveObject* pNewFocus, 
        unsigned controllerIdx, GFx::FocusMovedType fmt);
    static void BroadcastOnSetFocus(Environment* penv, InteractiveObject* pOldFocus, 
        InteractiveObject* pNewFocus, unsigned controllerIdx);
};

class SelectionProto : public Prototype<Selection>
{
public:
    SelectionProto (ASStringContext* psc, Object* pprototype, const FunctionRef& constructor);

};

#ifdef GFX_AS2_ENABLE_SELECTION

//
// Selection static class
//
// A constructor function object for Object
class SelectionCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void GetFocus(const FnCall& fn);
    static void SetFocus(const FnCall& fn);
    static void SetSelection(const FnCall& fn);
    static void GetCaretIndex(const FnCall& fn);
    static void GetBeginIndex(const FnCall& fn);
    static void GetEndIndex(const FnCall& fn);

    // extension methods
    // one parameter, boolean, true - to capture focus, false - to release one.
    // Returns currently focus character.
    static void CaptureFocus(const FnCall& fn);

    // 1st parameter - strings "up", "down", "left", "right", "tab".
    // 2nd parameter (optional) - the character to begin search from.
    // 3rd parameter (optional) - bool, true to include only focusEnabled = true chars
    // 4th parameter (optional) - keyboard index
    static void MoveFocus(const FnCall& fn);

    // 1st parameter - strings "up", "down", "left", "right", "tab".
    // 2nd parameter - a parent container (panel), or null
    // 3rd           - bool, if true then loop
    // 4th parameter (optional) - the character to begin search from.
    // 5th parameter (optional) - bool, true to include only focusEnabled = true chars
    // 6th parameter (optional) - keyboard index
    static void FindFocus(const FnCall& fn);

    // 1st param - modal clip
    // 2nd param - controller idx (opt)
    static void SetModalClip(const FnCall& fn);

    // 1st param - controller idx (opt)
    static void GetModalClip(const FnCall& fn);

    // 1st param - controller idx
    // 2nd param - focus group idx
    // ret - bool, true, if succ
    static void SetControllerFocusGroup(const FnCall& fn);

    // 1st param - controller id
    // ret - focus group idx
    static void GetControllerFocusGroup(const FnCall& fn);

    // 1st param - a movie clip
    // ret - array of controller indices focused on that movie clip
    static void GetFocusArray(const FnCall& fn);

    // 1st param - a movie clip
    // ret - a bitmask indicating controllers currently focused on that movie clip
    static void GetFocusBitmask(const FnCall& fn);

    // 1st param - an id of logical controller (focus group index)
    // returns a bit-mask where each bit represents a physical controller, 
    // associated with the specified focus group.
    static void GetControllerMaskByFocusGroup(const FnCall& fn);
public:
    SelectionCtorFunction (ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment*) const { return 0; }

    virtual bool GetMember(Environment* penv, const ASString& name, Value* pval);
    virtual bool SetMember(Environment *penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());

    static FunctionRef Register(GlobalContext* pgc);
};

#endif //#ifdef GFX_AS2_ENABLE_SELECTION

}}} // SF::GFx::AS2
#endif // INC_SF_GFX_SELECTION_H
