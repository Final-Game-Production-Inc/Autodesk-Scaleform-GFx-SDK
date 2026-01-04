/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Button.h
Content     :   Implementation of Button character
Created     :   Dec, 2009
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_BUTTON_H
#define INC_SF_GFX_BUTTON_H

#include "Kernel/SF_RefCount.h"

#include "GFx/GFx_Tags.h"
#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_ButtonDef.h"

namespace Scaleform { namespace GFx {

// ***** Declared Classes
class MouseState;
class ButtonAction;
class SoundSampleImpl;

class LoadProcess;

class ButtonRecord;

class AvmButtonBase : public AvmInteractiveObjBase
{
public:
    // called once when loading
    virtual void CreateCharacters() =0;
    // called on any button event
    virtual bool OnMouseEvent(const EventId& event) =0;
    // called on key press
    virtual bool OnButtonKeyEvent(const EventId& id, int* pkeyMask) =0; 
    // called when state is changed
    virtual void SwitchState(ButtonRecord::MouseState) =0;
};

// Core implementation of Button
class Button : public InteractiveObject
{
public:
    enum ButtonState
    {
        Up,
        Over,
        Down,
        Hit,

        StatesCount
    };
    struct CharToRec
    {
        Ptr<DisplayObjectBase> Char;
        const ButtonRecord*    Record;

        CharToRec() {}
        CharToRec(DisplayObjectBase* ch, const ButtonRecord* r) 
            : Char(ch), Record(r) {}
    };
    typedef ArrayLH<CharToRec>  StateCharArray;
protected:
    ButtonDef*                          pDef;

    //ArrayLH<Ptr<DisplayObjectBase> >    RecordCharacter;

    enum MouseFlags
    {
        IDLE = 0,
        FLAG_OVER = 1,
        FLAG_DOWN = 2,
        OVER_DOWN = FLAG_OVER|FLAG_DOWN,

        // aliases
        OVER_UP = FLAG_OVER,
        OUT_DOWN = FLAG_DOWN
    };
    struct StateCharacters
    {
        Ptr<Render::TreeContainer> pRenNode;
        StateCharArray             Characters;
    }                                   States[StatesCount];

    int                                 LastMouseFlags;
    int                                 mMouseFlags;
    ButtonRecord::MouseState            MouseState;

public:
    Button(ButtonDef* def, 
        MovieDefImpl *pbindingDefImpl,
        ASMovieRootBase* pasRoot, 
        InteractiveObject* parent, 
        ResourceId id);

    ~Button();

    //*** DisplayObjectBase
    virtual void            AdvanceFrame(bool nextFrame, float framePos);
    virtual void            OnEventLoad();

    virtual CharacterDef::CharacterDefType GetType() const { return CharacterDef::Button; }

//    virtual void            Display(DisplayContext& context);
    // Get bounds. This is used,
    // among other things, to calculate button width & height.
    virtual RectF           GetBounds(const Matrix &transform) const;
    virtual CharacterDef*   GetCharacterDef() const
    {
        return pDef;
    }
    // "transform" matrix describes the transform applied to parent and us,
    // including the object's matrix itself. This means that if transform is
    // identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
    virtual RectF           GetRectBounds(const Matrix &transform) const;
    // Return the topmost entity that the given point covers.  NULL if none.
    // I.E. check against ourself.
    virtual TopMostResult   GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);
    virtual void            OnEventUnload();
    virtual bool            PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;
    virtual void            Restart();
    // Set state changed flags
    virtual void            SetStateChangeFlags(UInt8 flags);
    virtual void            PropagateScale9GridExists();
    virtual void            SetScale9Grid(const RectF& rect);

    // *** overloaded methods of InteractiveObject
    // focus rect for buttons is calculated as follows:
    // 1) if "hit" state exists - boundary rect of "hit" shape
    // 2) if "down" state exists  - boundary rect of "down" shape
    // 3) if "over" state exists  - boundary rect of "over" shape
    // 4) otherwise - boundary rect of "up" shape
    virtual int             CheckAdvanceStatus(bool playingNow);
    virtual RectF           GetFocusRect() const;
    //virtual bool            IsTabable() const;
    // returns true, if event fired 
    virtual bool            OnMouseEvent(const EventId& event);
    // invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed)
    virtual void            OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType fmt);
    virtual bool            OnKeyEvent(const EventId& id, int* pkeyMask);
    // invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
    virtual bool            OnLosingKeyboardFocus(InteractiveObject*, unsigned controllerIdx, FocusMovedType);
    virtual void            PropagateMouseEvent(const EventId& event);
    virtual void            PropagateNoAdvanceGlobalFlag();
    virtual void            PropagateNoAdvanceLocalFlag();
    virtual void            SetVisible(bool visible);


    // Button
    void                    CreateCharacters();
    // Obtains an active button record based on state, or -1.
    // Note: not quite correct, since multiple records can apply to a state.
//     int                     GetActiveRecordIndex() const
//     {
//         return GetRecordIndex(MouseState);
//     }
    SF_INLINE AvmButtonBase*  GetAvmButton()
    {
        return GetAvmObjImpl()->ToAvmButttonBase();
    }
    SF_INLINE const AvmButtonBase*  GetAvmButton() const
    {
        return GetAvmObjImpl()->ToAvmButttonBase();
    }
    ButtonDef*              GetButtonDef() const { return pDef; }
    // Return a single character bounds
    RectF                   GetBoundsOfRecord(const Matrix &transform, DisplayObjectBase* pch) const;
    // returns the local boundaries of whole state
    RectF                   GetBoundsOfState(const Matrix &transform, ButtonState state) const;
    //int                     GetRecordIndex(ButtonRecord::MouseState mouseState) const;
//    Ptr<DisplayObjectBase>& GetRecordCharacterRef(UPInt idx) { return RecordCharacter[idx]; }
//    UPInt                   GetRecordCharacterCount() const  { return RecordCharacter.GetSize(); }
    // Return a single character "pure rectangle" bounds (not considering the stroke)
    RectF                   GetRectBounds(const Matrix &transform, DisplayObjectBase* pch) const;

    // Returns characters corresponding to the mouse state
    const StateCharacters&  GetCharacters(ButtonRecord::MouseState mouseState) const
    {
        return States[GetButtonState(mouseState)];
    }
    const StateCharacters&  GetCharacters(ButtonState buttonState) const
    {
        return States[buttonState];
    }
    StateCharacters&        GetCharacters(ButtonState buttonState)
    {
        return States[buttonState];
    }
    Render::TreeContainer*  GetStateRenderContainer(ButtonState buttonState) const
    {
        return States[buttonState].pRenNode;
    }
    Ptr<Render::TreeContainer> CreateStateRenderContainer(ButtonState buttonState);

    virtual void            RemoveDisplayObject(DisplayObjectBase*);

    //void                    RecreateCharacters();
    void                    SetDirtyFlag();
    void                    SwitchState();
    void                    UnloadCharactersForState(ButtonState state);
    void                    ClearRenderTreeForState(ButtonState state);

    ButtonState             GetCurrentState() const { return GetButtonState(MouseState); }
    static ButtonState      GetButtonState(ButtonRecord::MouseState mouseState);
};


}} // namespace Scaleform::GFx


#endif // INC_SF_GFX_BUTTON_H

