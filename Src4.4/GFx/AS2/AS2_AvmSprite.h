/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_AvmSprite.h
Content     :   Implementation of AS2 part of Sprite (MovieClip)
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_AS2_SPRITE_H
#define INC_SF_GFX_AS2_SPRITE_H

#include "GFx/GFx_Sprite.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/AS2/AS2_AvmCharacter.h"

namespace Scaleform {
namespace GFx {
namespace AS2 {

class MovieClipObject;

// This class represents AS2-dependent part of GFx::Sprite
// character class.
class AvmSprite : public AvmCharacter, public AvmSpriteBase
{
    friend class AvmButton;
private:
    Environment         ASEnvironment;
    int                 Level;
    // AS object
    Ptr<MovieClipObject> ASMovieClipObj;
    ArrayLH<bool>       InitActionsExecuted;    // a bit-Array class would be ideal for this
    Bool3W              TabChildren;

    // internal method; will create GASMovieClip object on-demand
    MovieClipObject*    GetMovieClipObject ();

    // used by both GetMember & GetMemberRaw to share common part of code
    bool                GetMember(Environment* penv, ASStringContext *psc, const ASString& name, Value* pval);
    int                 GetHitAreaIndex() const;
public:
    AvmSprite(Sprite* psprite);
    ~AvmSprite();

    //***** AvmDisplayObjBase implementation
    virtual class AvmInteractiveObjBase*    ToAvmInteractiveObjBase() 
    { 
        return static_cast<AvmCharacter*>(this); 
    }
    virtual class AvmSpriteBase*            ToAvmSpriteBase() { return this; }
    virtual class AvmButtonBase*            ToAvmButttonBase() { return NULL; }
    virtual class AvmTextFieldBase*         ToAvmTextFieldBase() { return NULL; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() 
    { 
        return this; 
    }
    virtual void                            OnEventLoad();
    virtual void                            OnEventUnload();
    //virtual void                            OnRemoved() {}
    virtual bool                            OnUnloading(bool mayRemove);
    virtual void                            SetHitAreaNotify(Sprite* phitArea);

    //***** AvmInteractiveObjectBase implementation
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar)
    {
        AvmCharacter::CopyPhysicalProperties(poldChar);
    }
    // Determines the absolute path of the character.
    virtual const char*     GetAbsolutePath(String *ppath) const
    {
        return AvmCharacter::GetAbsolutePath(ppath);
    }
    virtual bool            IsTabable() const 
    { 
        return AvmCharacter::IsTabable();
    }
    virtual void            OnFocus
        (InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
        unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmCharacter::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
    }
    virtual void            OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmCharacter::OnGettingKeyboardFocus(controllerIdx, fmt);
    }
    virtual bool            OnKeyEvent(const EventId& id, int* pkeyMask)
    {
        return AvmCharacter::OnKeyEvent(id, pkeyMask);
    }
    virtual bool            OnLosingKeyboardFocus
        (InteractiveObject* newFocusCh, unsigned controllerIdx, FocusMovedType fmt = GFx_FocusMovedByKeyboard)
    {
        return AvmCharacter::OnLosingKeyboardFocus(newFocusCh, controllerIdx, fmt);
    }

    virtual void            AdvanceFrame(bool nextFrame, float framePos);
    virtual bool            MustBeInPlaylist() const;

    virtual TopMostResult   GetTopMostEntity(const Render::PointF &localPt, TopMostDescr* pdescr,
                                             const ArrayPOD<UByte>& hitTest);

    ASStringContext*        GetSC() const { return ASEnvironment.GetSC(); }
    Environment*            GetASEnvironment()          { return &ASEnvironment; }
    const Environment*      GetASEnvironment() const    { return &ASEnvironment; }
    Object*                 GetASObject ();
    Object*                 GetASObject () const;
    SF_INLINE bool          IsCaseSensitive() const     { return ASEnvironment.IsCaseSensitive(); }
    AvmDisplayObjBase*      ToAvmDispObj() { return static_cast<AvmCharacter*>(this); }
    SF_INLINE Sprite*       GetSprite() const { return static_cast<Sprite*>(pDispObj); }

    DisplayList&            GetDisplayList() 
    { 
        return GetSprite()->GetDisplayList();
    }
    const DisplayList&      GetDisplayList() const
    { 
        return GetSprite()->GetDisplayList();
    }

    bool                    Invoke(const char* methodName, Value* presult, unsigned numArgs);
    bool                    InvokeArgs(const char* methodName, Value *presult, const char* methodArgFmt, va_list args);     

    // Level access.
    inline int              GetLevel() const        { return Level; }
    void                    SetLevel(int level);

    // Execute action buffer
    virtual bool            ExecuteBuffer(ActionBuffer* pactionbuffer);
    // Execute this even immediately (called for processing queued event actions).
    virtual bool            ExecuteEvent(const EventId& id);
    // Execute the actions for the specified frame. 
    void                    CallFrameActions(unsigned frameNumber);

    void                    SetHasButtonHandlers (bool has);
    SpriteDef*              GetSpriteDef() const 
    { 
        SF_ASSERT(pDispObj); 
        return static_cast<Sprite*>(pDispObj)->GetSpriteDef();
    }
    TimelineDef*            GetDef() const
    {
        SF_ASSERT(pDispObj); 
        return GetSprite()->GetDef();
    }
    MovieDefImpl*           GetDefImpl() const    
    { 
        SF_ASSERT(pDispObj); 
        return GetSprite()->GetDefImpl();
    }

    // Character override to indicate which standard members are handled by us.
    virtual UInt32      GetStandardMemberBitMask() const;

    // Handles built-in members. Return 0 if member is not found or not supported.
    virtual bool        SetStandardMember(StandardMember member, const Value& val, bool opcodeFlag);
    virtual bool        GetStandardMember(StandardMember member, Value* val, bool opcodeFlag) const;

    // GASObjectInterface implementation.
    virtual bool        GetMember(Environment* penv, const ASString& name, Value* pval)
    {
        return GetMember(penv, NULL, name, pval);
    }
    virtual bool        DeleteMember(ASStringContext *psc, const ASString& name);
    virtual void        VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, unsigned visitFlags, const ObjectInterface*) const;
    virtual ObjectType  GetObjectType() const { return Object_Sprite; }
    virtual bool        SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());
    virtual bool        GetMemberRaw(ASStringContext *psc, const ASString& name, Value* pval)
    {
        return GetMember(NULL, psc, name, pval);
    }

    virtual InteractiveObject* GetRelativeTarget(const ASString& name, bool first_call);

    bool IsUnloaded() const { return GetSprite()->IsUnloaded(); }
    bool IsEnabledFlagSet() const { return GetSprite()->IsEnabledFlagSet(); }
    void AddActionBuffer(ActionBuffer* a, 
        ActionPriority::Priority prio = ActionPriority::AP_Frame);

#ifndef SF_NO_IME_SUPPORT
	bool CheckCandidateListLevel(int) const;
#endif
    //////////////////////////////////////////////////////////////////////////
    virtual bool ActsAsButton() const;
    virtual bool IsLevelMovie() const;
    virtual void ForceShutdown();
    virtual unsigned GetCursorType() const;

    virtual void ExecuteInitActionFrameTags(unsigned frame);
    // Used for root-level sprites.
    virtual void ExecuteFrame0Events();
    virtual void ExecuteFrameTags(unsigned) {}
    virtual void CloneInternalData(const InteractiveObject* src);

    virtual bool HasEventHandler(const EventId& id) const;

    virtual bool OnEvent(const EventId& id);
    virtual DisplayObjectBase*   AddDisplayObject(
        const CharPosInfo &pos, 
        const ASString& name,
        const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
        const void *pinitSource,
        unsigned createFrame,
        UInt32 addFlags,
        CharacterCreateInfo* pcharCreateOverride,
        InteractiveObject* origChar);

    virtual void        FillTabableArray(InteractiveObject::FillTabableParams* params);

    // Movie Loading support
    virtual bool                ReplaceChildCharacterOnLoad
        (InteractiveObject *poldChar, InteractiveObject *pnewChar);
    virtual bool                ReplaceChildCharacter
        (InteractiveObject *poldChar, InteractiveObject *pnewChar);

    // should return true, if focus may be set to this character by keyboard or
    // ActionScript.
    virtual bool        IsFocusEnabled(FocusMovedType fmt = GFx_FocusMovedByKeyboard) const;

    bool                RemoveCharacter(DisplayObjectBase*);

    unsigned            OnAddingDisplayObject(const CharacterCreateInfo& ccinfo,
                                              InteractiveObject* obj, 
                                              const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
                                              const void* pinitSource,
                                              bool placeObject);
    void                OnAddedDisplayObject(InteractiveObject* obj, unsigned sessionId, bool placeObject);

    // Drawing API
    DrawingContext*     GetDrawingContext() { return GetSprite()->GetDrawingContext(); }
    void                SetNoFill();
    void                SetNoLine();
    void                SetLineStyle(float lineWidth, 
                                     unsigned  rgba, 
                                     bool  hinting, 
                                     unsigned scaling, 
                                     unsigned caps,
                                     unsigned joins,
                                     float miterLimit);
    Render::ComplexFill* BeginFill();
    Render::ComplexFill* CreateLineComplexFill();
    void                BeginFill(unsigned rgba);    
    void                BeginBitmapFill(FillType fillType,
                                        ImageResource* pimageRes, 
                                        const Matrix& mtx);
    void                EndFill();
    void                MoveTo(float x, float y);
    void                LineTo(float x, float y);
    void                CurveTo(float cx, float cy, float ax, float ay);
    DrawingContext*     AcquirePath(bool newShapeFlag);

    //ActionScript methods
    // Need to access depths, etc.
    static void         SpriteSwapDepths(const FnCall& fn);
    static void         SpritePlay(const FnCall& fn);
    static void         SpriteStop(const FnCall& fn);
    static void         SpriteGotoAndStop(const FnCall& fn);
    static void         SpriteGotoAndPlay(const FnCall& fn);
    static void         SpriteNextFrame(const FnCall& fn);
    static void         SpritePrevFrame(const FnCall& fn);
    static void         SpriteGetBytesLoaded(const FnCall& fn);
    static void         SpriteGetBytesTotal(const FnCall& fn);

    static void         SpriteGetBounds(const FnCall& fn);
    static void         SpriteGetRect(const FnCall& fn);
    static void         SpriteLocalToGlobal(const FnCall& fn);
    static void         SpriteGlobalToLocal(const FnCall& fn);
    static void         SpriteHitTest(const FnCall& fn);

    static void         SpriteAttachBitmap(const FnCall& fn);
    static void         SpriteAttachMovie(const FnCall& fn);
    static void         SpriteAttachAudio(const FnCall& fn);

    static void         SpriteDuplicateMovieClip(const FnCall& fn);
    static void         SpriteRemoveMovieClip(const FnCall& fn);
    static void         SpriteCreateEmptyMovieClip(const FnCall& fn);
    static void         SpriteCreateTextField(const FnCall& fn);

    static void         SpriteGetNextHighestDepth(const FnCall& fn);
    static void         SpriteGetInstanceAtDepth(const FnCall& fn);

    static void         SpriteGetTextSnapshot(const FnCall& fn);

    static void         SpriteGetSWFVersion(const FnCall& fn);

    static void         SpriteStartDrag(const FnCall& fn);
    static void         SpriteStopDrag(const FnCall& fn);

    static void         SpriteSetMask(const FnCall& fn);

    static void         SpriteLoadMovie(const FnCall& fn);
    static void         SpriteUnloadMovie(const FnCall& fn);

    static void         SpriteLoadVariables(const FnCall& fn);

    // Drawing API
    static void         SpriteClear(const FnCall& fn);
    static void         SpriteBeginFill(const FnCall& fn);
    static void         SpriteBeginGradientFill(const FnCall& fn);
    static void         SpriteBeginBitmapFill(const FnCall& fn);
    static void         SpriteLineGradientStyle(const FnCall& fn);
    static void         SpriteEndFill(const FnCall& fn);
    static void         SpriteLineStyle(const FnCall& fn);
    static void         SpriteMoveTo(const FnCall& fn);
    static void         SpriteLineTo(const FnCall& fn);
    static void         SpriteCurveTo(const FnCall& fn);
private:
    static void         InitializeClassInstance(const FnCall& fn);
    static void         FindClassAndInitializeClassInstance(const FnCall& fn);
    static void         InitObjectMembers(const FnCall& fn);
    static Sprite*      SpriteGetTarget(const FnCall& fn);
    static InteractiveObject* CharacterGetTarget(const FnCall& fn);
    static void         SpriteCreateGradient(const FnCall& fn, Render::ComplexFill* complexFill);
};

SF_INLINE AvmSprite* ToAvmSprite(InteractiveObject* po)
{
    SF_ASSERT(po->IsAVM1() && po->IsSprite());
    return static_cast<AvmSprite*>(static_cast<AvmCharacter*>(po->GetAvmObjImpl()));
}

//!AB should be separated later.
// Class that represents MovieClip ActionScript class.
class MovieClipObject : public Object
{
    friend class MovieClipProto;
    friend class MovieClipCtorFunction;

    WeakPtr<Sprite> pSprite;    // weak ref on sprite obj

    // Bit mask of all button events set on the movieclip
    enum
    {
        Mask_onPress            = 0x001,
        Mask_onRelease          = 0x002,
        Mask_onReleaseOutside   = 0x004,
        Mask_onRollOver         = 0x008,
        Mask_onRollOut          = 0x010,
        Mask_onDragOver         = 0x020,
        Mask_onDragOut          = 0x040,

        Mask_onPressAux             = 0x080,
        Mask_onReleaseAux           = 0x100,
        Mask_onReleaseOutsideAux    = 0x200,
        Mask_onDragOverAux          = 0x400,
        Mask_onDragOutAux           = 0x800
    };
    UInt16 ButtonEventMask;

#ifdef GFX_AS_ENABLE_GC
protected:
    void Finalize_GC()
    {
        pSprite.~WeakPtr<Sprite>();
        Object::Finalize_GC();
    }
#endif // SF_NO_GC
    void commonInit();
protected:
    // If any of these variables are non-zero, sprite acts as a button,
    // catching all of buttonAction logic and disabling nested buttons
    // or similar clip actions.

    // Set if the sprite has any button event handlers.
    bool                HasButtonHandlers;
    // Count of dynamic handlers installed by SetMember: onPress, onRollOver, etc.
    //UByte               DynButtonHandlerCount;  

    // Updates DynButtonHandlerCount if name is begin added through SetMember,
    // or deleted 
    void            TrackMemberButtonHandler(ASStringContext* psc, const ASString& name, bool deleteFlag = 0);

    MovieClipObject(ASStringContext *psc, Object* pprototype) 
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
        commonInit();
    }
    MovieClipObject(Environment* penv)
        : Object(penv)
    { 
        commonInit();
        Set__proto__(penv->GetSC(), penv->GetGC()->GetActualPrototype(penv, ASBuiltin_MovieClip));
    }
    void    SetMemberCommon(ASStringContext *psc, 
        const ASString& name, 
        const Value& val);
public:
    MovieClipObject(GlobalContext* gCtxt, Sprite* psprite) 
        : Object(gCtxt->GetGC()), pSprite(psprite)
    {
        commonInit ();
        ASStringContext* psc = GFx::AS2::ToAvmSprite(psprite)->GetASEnvironment()->GetSC();
        Set__proto__ (psc, GFx::AS2::ToAvmSprite(psprite)->Get__proto__());
    }

    // Set the named member to the value.  Return true if we have
    // that member; false otherwise.
    virtual bool        SetMemberRaw(ASStringContext *psc, 
        const ASString& name, 
        const Value& val, 
        const PropFlags& flags = PropFlags());
    virtual bool        SetMember(Environment* penv, 
        const ASString& name, 
        const Value& val, 
        const PropFlags& flags = PropFlags());
    //virtual bool      GetMember(Environment* penv, const GASString& name, Value* val);
    virtual bool        DeleteMember(ASStringContext *psc, const ASString& name);

    virtual void        Set__proto__(ASStringContext *psc, Object* protoObj);

    // Returns 1 if sprite acts as a button due to handlers.
    bool                ActsAsButton() const;

    SF_INLINE void      SetHasButtonHandlers(bool has)      { HasButtonHandlers=has; }

    virtual ObjectType  GetObjectType() const   { return Object_MovieClipObject; }

    Ptr<Sprite>         GetSprite() { return pSprite; }

    void                ForceShutdown();

    virtual InteractiveObject*       GetASCharacter() { return Ptr<Sprite>(pSprite).GetPtr(); }
    virtual const InteractiveObject* GetASCharacter() const { return Ptr<Sprite>(pSprite).GetPtr(); }

    Ptr<Object>         Exchange__proto__(Object* newproto)
    {
        Ptr<Object> pproto = pProto;
        pProto = newproto;
        return pproto;
    }

    // Returns mask for the button event name. 0, if not found.
    static UInt16       GetButtonEventNameMask(ASStringContext* psc, const ASString& name);

    // Returns 'true' if the 'name' is a button event name. Actually, the retur
    static bool         IsButtonEventName(ASStringContext* psc, const ASString& name)
    {
        return GetButtonEventNameMask(psc, name) != 0;
    }
};

class MovieClipProto : public Prototype<MovieClipObject>
{
public:
    MovieClipProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

    //static void GlobalCtor(const FnCall& fn);

};

class MovieClipCtorFunction : public CFunctionObject
{
public:
    MovieClipCtorFunction(ASStringContext *psc) : CFunctionObject(psc, GlobalCtor) {}

    virtual Object* CreateNewObject(Environment* penv) const;

    static void GlobalCtor(const FnCall& fn);
};

}}} // SF::GFx::AS2

#endif //INC_SF_GFX_AS2_SPRITE_H
