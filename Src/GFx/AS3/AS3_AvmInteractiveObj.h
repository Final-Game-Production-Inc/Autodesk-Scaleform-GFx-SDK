/**************************************************************************

Filename    :   AS3_AvmInteractiveObj.h
Content     :   Implementation of AS3-dependent part of InteractiveObject.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_AvmInteractiveObj_H
#define INC_AS3_AvmInteractiveObj_H

#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_InteractiveObject.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_InteractiveObject.h"
#include "GFx/AS3/AS3_AvmDisplayObj.h"

namespace Scaleform {

// ***** External Classes
namespace Render {
struct TextFilter;
}

namespace GFx {

class MovieImpl;
class MovieDataDef;
class MovieDefImpl;

namespace AS3 {

class MovieRoot;

// *****  InteractiveObject - ActionScript controllable object on stage

// AvmInteractiveObj is a regular character that adds ActionScript control capabilities
// and extra fields, such as _name, which are only available in those objects.

class AvmInteractiveObj : 
    public AvmDisplayObj,
    public AvmInteractiveObjBase
{
    UInt8       MouseOverCnt; // count of mouseovers (for multiple mouse cursors)
    enum
    {
        //Flag_MouseEnabled           = 0x01,
        Flag_DoubleClickEnabled     = 0x02
    };
    UInt8       Flags;
public:

    // Constructor.
    AvmInteractiveObj(InteractiveObject* pdispObj);
    ~AvmInteractiveObj();

    InteractiveObject* GetIntObj() const { return static_cast<InteractiveObject*>(pDispObj); }

    //***** AvmDisplayObjectBase implementation
    virtual class AvmInteractiveObjBase*    ToAvmInteractiveObjBase() { return this; }
    virtual class AvmSpriteBase*            ToAvmSpriteBase()         { return NULL; }
    virtual class AvmButtonBase*            ToAvmButttonBase()        { return NULL; }
    virtual class AvmTextFieldBase*         ToAvmTextFieldBase()      { return NULL; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase(){ return NULL; }

    virtual void            OnEventLoad();
    // Special event handler; ensures that unload is called on child items in timely manner.
    virtual void            OnEventUnload();
    //virtual void            OnRemoved() { AvmDisplayObj::OnRemoved(); }
    virtual bool            OnUnloading(bool mayRemove) { return AvmDisplayObj::OnUnloading(mayRemove); }
    // Determines the absolute path of the character.
    virtual const char*     GetAbsolutePath(String *ppath) const
    {
        return AvmDisplayObj::GetAbsolutePath(ppath);
    }

    //***** AvmInteractiveObjectBase implementation
    virtual bool            ActsAsButton() const;
    // Copies physical properties and reassigns character handle.
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar);
    virtual void            CloneInternalData(const InteractiveObject* src);

    virtual unsigned        GetCursorType() const { return MouseCursorEvent::ARROW; }
    virtual bool            OnEvent(const EventId& id);

    // Focus related stuff
    virtual bool            IsTabable() const;
    // invoked when lose/gain focus
    virtual void            OnFocus(InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
        unsigned controllerIdx, FocusMovedType fmt);

    // invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed).
    virtual void            OnGettingKeyboardFocus(unsigned /*controllerIdx*/, FocusMovedType) { }
    virtual bool            OnKeyEvent(const EventId& , int* ) { return false; }
    // invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
    // if returns false, focus will not be transfered.
    virtual bool            OnLosingKeyboardFocus(InteractiveObject* newFocusCh, unsigned controllerIdx, FocusMovedType fmt = GFx_FocusMovedByKeyboard) 
    { 
        SF_UNUSED3(newFocusCh, controllerIdx, fmt); 
        return true; 
    }

    /////////////////////////////////////////
    bool                    OnFocusChange(InteractiveObject* toBeFocused, unsigned controllerIdx, 
        FocusMovedType fmt, ProcessFocusKeyInfo* pfocusKeyInfo);
    virtual const char*     GetDefaultASClassName() const { return "flash.display.InteractiveObject"; }

    bool                    IsSprite() const {  return GetIntObj()->IsSprite(); }
    virtual bool            IsStage() const { return false; }

    MovieDefImpl*           GetDefImpl() const    { return GetIntObj()->GetDefImpl(); }
    MovieImpl*              GetMovieImpl() const { return GetIntObj()->GetMovieImpl(); }
    Instances::fl_display::InteractiveObject*   GetAS3IntObj() const 
    { 
        return static_cast<Instances::fl_display::InteractiveObject*>(GetAS3Obj()); 
    }
    virtual InteractiveObject*  GetTopParent(bool ignoreLockRoot = false) const;
    virtual AvmInteractiveObj*  GetAvmTopParent(bool ignoreLockRoot = false) const;

    virtual InteractiveObject* FindInsertToPlayList(InteractiveObject* ch);
    void                    InsertChildToPlayList(InteractiveObject* ch);
    void                    MoveBranchInPlayList();

    virtual void            ForceShutdown() { AvmDisplayObj::ForceShutdown(); }

    // *** Parent list manipulation functions (work on all types of characters)

    // Remove *this* object from its parent.
    //void                    RemoveDisplayObject();

//     void                    SetMouseEnabled(bool v = true)
//     {
//         (v) ? Flags |= Flag_MouseEnabled : Flags &= ~Flag_MouseEnabled;
//     }
//     bool                    IsMouseEnabled() const { return (Flags & Flag_MouseEnabled); }
    void                    SetDoubleClickEnabled(bool v = true)
    {
        (v) ? Flags |= Flag_DoubleClickEnabled : Flags &= ~Flag_DoubleClickEnabled;
    }
    bool                    IsDoubleClickEnabled() const { return (Flags & Flag_DoubleClickEnabled) != 0; }

    unsigned IncrementMouseOverCnt() { return MouseOverCnt++; }
    unsigned DecrementMouseOverCnt() { return (MouseOverCnt != 0) ? --MouseOverCnt : ~0u; }
#ifdef SF_OBJ_COLLECTOR_ENABLED
public:
    virtual const char* GetObjectTypeName() const { return "GFx::InteractiveObject"; }
#endif
};

SF_INLINE AvmInteractiveObj* ToAvmInteractiveObj(InteractiveObject* po)
{
    if (!po)
        return NULL;
    SF_ASSERT(po->IsAVM2());
    SF_ASSERT(po->GetAvmObjImpl());
    return static_cast<AvmInteractiveObj*>(po->GetAvmObjImpl()->ToAvmInteractiveObjBase());
}

SF_INLINE const AvmInteractiveObj* ToAvmInteractiveObj(const InteractiveObject* po)
{
    return const_cast<const AvmInteractiveObj*>(ToAvmInteractiveObj(const_cast<InteractiveObject*>(po)));
}

}}} // namespace SF::GFx::AS2

#endif // INC_AS3_AvmInteractiveObj_H

