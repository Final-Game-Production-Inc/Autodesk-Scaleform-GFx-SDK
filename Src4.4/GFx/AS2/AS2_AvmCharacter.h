/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_AvmCharacter.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_AS2CHARACTER_H
#define INC_SF_GFX_AS2CHARACTER_H

#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_InteractiveObject.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/GFx_ShapeSwf.h"

// ***** External Classes

class MovieImpl;
class MovieDataDef;
class MovieDefImpl;
class Environment;
struct TextFilter;

namespace Scaleform {
namespace GFx {
namespace AS2 {

class MovieRoot;


// *****  DisplayObject - ActionScript controllable object on stage

// AvmCharacter is a regular character that adds ActionScript control capabilities
// and extra fields, such as _name, which are only available in those objects.

class AvmCharacter : public NewOverrideBase<StatMV_ActionScript_Mem>,
    public AvmInteractiveObjBase, 
//    public AvmDisplayObjBase, 
    public ObjectInterface
{
protected:
    InteractiveObject*            pDispObj;

    typedef ArrayLH<Value, StatMV_ActionScript_Mem> EventsArray;
    HashLH<EventId, EventsArray, EventIdHashFunctor, StatMV_ActionScript_Mem> EventHandlers;


private:
    // implementations of ASInteractiveObjBase
    virtual void Bind(DisplayObject* po)
    {
        SF_ASSERT(po->IsInteractiveObject() && static_cast<InteractiveObject*>(po)->IsAVM1());
        pDispObj = static_cast<InteractiveObject*>(po);
    }

public:

    typedef ObjectInterface::ObjectType  ObjectType;

    // Constructor.
    AvmCharacter(InteractiveObject* pdispObj);
    ~AvmCharacter();

    SF_INLINE InteractiveObject* GetDispObj() const { return pDispObj; }

    // Retrieves a global AS context - there is only one in root.
    // DisplayObject will implement this as { return GetMovieImpl()->pGlobalContext; }
    // This is short because it is used frequently in expressions.
    virtual GlobalContext*   GetGC() const;

    //***** AvmDisplayObjectBase implementation
    virtual void            OnEventLoad()               { OnEvent(EventId(EventId::Event_Load)); }
    // Special event handler; ensures that unload is called on child items in timely manner.
    virtual void            OnEventUnload();
    //virtual void            OnRemoved() {}

    //***** AvmInteractiveObjectBase implementation
    // Determines the absolute path of the character.
    // Copies physical properties and reassigns character handle.
    virtual void            CloneInternalData(const InteractiveObject* src);
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar);
    virtual bool            IsFocusEnabled(FocusMovedType = GFx_FocusMovedByKeyboard) const  { return false; }
    virtual const char*     GetAbsolutePath(String *ppath) const;
    virtual unsigned        GetCursorType() const { return MouseCursorEvent::ARROW; }

    //     // Focus related stuff
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

    //////////////////////////////////////////////////////////////////////////
    // Event handler accessors.
    bool                    HasClipEventHandler(const EventId& id) const;
    bool                    InvokeClipEventHandlers(Environment* penv, const EventId& id);
    void                    SetSingleClipEventHandler(const EventId& id, const Value& method);
    void                    SetClipEventHandlers(const EventId& id, const Value& method);

    // From MovieSub: necessary?
    InteractiveObject*      GetParent() const { return pDispObj->GetParent(); }
    bool                    IsSprite() const {  return pDispObj->IsSprite(); }
    ASString                GetName() const  { return pDispObj->GetName(); }

    virtual Object*         GetASObject ()          { return NULL; }
    virtual Object*         GetASObject () const    { return NULL; }    

    virtual InteractiveObject* GetRelativeTarget(const ASString& name, bool first_call)    
    { SF_UNUSED2(name, first_call); return 0; } // for buttons and edit box should just return NULL


    // Called to substitute poldChar child of this control from new child.
    // This is done to handle loadMovie only; physical properties of the character are copied.
    virtual bool            ReplaceChildCharacterOnLoad(InteractiveObject *poldChar, InteractiveObject *pnewChar) 
    { SF_UNUSED2(poldChar, pnewChar); return 0; }
    // Called to substitute poldChar child of this control from new child.
    // This is done to handle loadMovie with progressive loading only; 
    // physical properties of the character are copied; actions processing is not call
    // because the first frame may not be loaded yet.
    virtual bool            ReplaceChildCharacter(InteractiveObject *poldChar, InteractiveObject *pnewChar)
    { SF_UNUSED2(poldChar, pnewChar); return 0; }

    // *** Shared ActionScript methods.

    // Depth implementation - same in MovieClip, Button, TextField.
    static void CharacterGetDepth(const FnCall& fn);

    // ASEnvironment - sometimes necessary for strings.
    MovieDefImpl*               GetDefImpl() const    { return pDispObj->GetDefImpl(); }
    MovieImpl*                  GetMovieImpl() const  { return pDispObj->GetMovieImpl(); }
    virtual Environment*        GetASEnvironment();
    virtual const Environment*  GetASEnvironment() const;
    MovieRoot*                  GetAS2Root() const;
    //MovieImpl*                   GetMovieImpl() const;
    virtual InteractiveObject*  GetTopParent(bool ignoreLockRoot = false) const;
    virtual AvmCharacter*       GetAvmTopParent(bool ignoreLockRoot = false) const;

    //     SF_INLINE AvmCharacter*        GetASParent() const { return static_cast<AvmCharacter*>(GetParent()); }

    // *** Standard ActionScript property support


    // StandardMember property constants are used to support built-in properties,
    // to provide common implementation for those properties for which it is
    // identical in multiple character types, and to enable the use of switch()
    // statement in GetMember/SetMember implementations instead of string compares.

    // Standard properties are divided into several categories based on range.
    //
    //  1). [0 - 21] - These are properties directly used by SetProperty and
    //                 GetProperty opcodes in ActionScript. Thus, they cannot 
    //                 be renumbered.
    //
    //  2). [0 - 31] - These properties have conditional flags indicating
    //                 whether default implementation of AvmCharacter is
    //                 usable for the specific character type.
    //
    //  3). [31+]    - These properties can be in a list, however, they
    //                 cannot have a default implementation.

    enum StandardMember
    {
        M_InvalidMember = -1,

        // Built-in property constants. These must come in specified order since
        // they are used by ActionScript opcodes.
        M_x             = 0,
        M_BuiltInProperty_Begin = M_x,
        M_y             = 1,
        M_xscale        = 2,
        M_yscale        = 3,
        M_currentframe  = 4,
        M_totalframes   = 5,
        M_alpha         = 6,
        M_visible       = 7,
        M_width         = 8,
        M_height        = 9,
        M_rotation      = 10,
        M_target        = 11,
        M_framesloaded  = 12,
        M_name          = 13,
        M_droptarget    = 14,
        M_url           = 15,
        M_highquality   = 16,
        M_focusrect     = 17,
        M_soundbuftime  = 18,
        // SWF 5+.
        M_quality       = 19,
        M_xmouse        = 20,
        M_ymouse        = 21,
        M_BuiltInProperty_End= M_ymouse,

        // Extra shared properties which can have default implementation
        M_parent        = 22,       
        M_blendMode     = 23,
        M_cacheAsBitmap = 24,   // B, M
        M_filters       = 25,
        M_enabled       = 26,   // B, M
        M_trackAsMenu   = 27,   // B, M
        M_lockroot      = 28,   // M, however, it can stick around
        M_tabEnabled    = 29,       
        M_tabIndex      = 30,       
        M_useHandCursor = 31,   // B, M
        
        M_edgeaaMode,
        
        M_SharedPropertyEnd = M_edgeaaMode,

        // Properties used by some characters.
        M_menu, // should be shared, not enough bits.

        // Movie clip.
        M_focusEnabled,
        M_tabChildren,
        M_transform,
        M_scale9Grid,
        M_hitArea,
        M_scrollRect,

        // Dynamic text.
        M_text,
        M_textWidth,
        M_textHeight,
        M_textColor,
        M_length,
        M_html,
        M_htmlText,
        M_styleSheet,
        M_autoSize,
        M_wordWrap,
        M_multiline,
        M_border,
        M_variable,
        M_selectable,
        M_embedFonts,
        M_antiAliasType,
        M_hscroll,
        M_scroll,
        M_maxscroll,
        M_maxhscroll,
        M_background,
        M_backgroundColor,
        M_borderColor,
        M_bottomScroll,
        M_type,
        M_maxChars,
        M_condenseWhite,
        M_mouseWheelEnabled,
        // Input text
        M_password,

        // GFx extensions
        M_shadowStyle,
        M_shadowColor,
        M_hitTestDisable,
        M_noTranslate,
        M_caretIndex,
        M_numLines,
        M_verticalAutoSize,
        M_fontScaleFactor,
        M_verticalAlign,
        M_textAutoSize,
        M_useRichTextClipboard,
        M_alwaysShowSelection, 
        M_selectionBeginIndex,
        M_selectionEndIndex,
        M_selectionBkgColor,
        M_selectionTextColor,
        M_inactiveSelectionBkgColor,
        M_inactiveSelectionTextColor,
        M_noAutoSelection,
        M_disableIME,
        M_topmostLevel,
        M_noAdvance,
        M_focusGroupMask,
        M_bidirectionalTextEnabled,

        // Dynamic Text
        M_autoFit,
        M_blurX,
        M_blurY,
        M_blurStrength,
        M_outline,
        M_fauxBold,
        M_fauxItalic,
        M_restrict,

        // Dynamic Text Shadow
        M_shadowAlpha,
        M_shadowAngle,
        M_shadowBlurX,
        M_shadowBlurY,
        M_shadowDistance,
        M_shadowHideObject,
        M_shadowKnockOut,
        M_shadowQuality,
        M_shadowStrength,
        M_shadowOutline,

        M_z,
        M_zscale,
        M_xrotation,
        M_yrotation,
        M_matrix3d,
        M_fov,
        M__version,

        M_StandardMemberCount,


        // Standard bit masks.

        // Physical transform related members.
        M_BitMask_PhysicalMembers   = (1 << M_x) | (1 << M_y) | (1 << M_xscale) | (1 << M_yscale) |
        (1 << M_rotation) | (1 << M_width) | (1 << M_height) | (1 << M_alpha),        
        // Members shared by Button, MovieClip, TextField and Video.
        M_BitMask_CommonMembers     = (1 << M_visible) | (1 << M_parent) | (1 << M_name)        
    };
    struct MemberTableType
    {
        const char *                    pName;
        AvmCharacter::StandardMember  Id;
        bool                            CaseInsensitive;
    };
    static MemberTableType  MemberTable[];

    // Returns a bit mask of supported standard members withing this character type.
    // GetStandardMember/SetStandardMember methods will fail if the requested constant is not in the bit mask.
    virtual UInt32          GetStandardMemberBitMask() const { return 0; }

    // Looks up a standard member and returns M_InvalidMember if it is not found.
    StandardMember          GetStandardMemberConstant(const ASString& newname) const;
    // Initialization helper - called to initialize member hash in GlobalContext
    static void             InitStandardMembers(GlobalContext *pcontext);

    static bool             IsStandardMember(const ASString& memberName, ASString* pcaseInsensitiveName = NULL);


    // Handles built-in members. Return 0 if member is not found or not supported.
    virtual bool            SetStandardMember(StandardMember member, const Value& val, bool opcodeFlag);
    virtual bool            GetStandardMember(StandardMember member, Value* val, bool opcodeFlag) const;



    // ObjectInterface stuff - for now.
    virtual bool            SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());
    virtual bool            GetMember(Environment* penv, const ASString& name, Value* val)
    { SF_UNUSED3(penv, name, val); SF_ASSERT(0); return false; }
    virtual void            VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, unsigned visitFlags, const ObjectInterface*) const;
    virtual bool            DeleteMember(ASStringContext *psc, const ASString& name);
    virtual bool            SetMemberFlags(ASStringContext *psc, const ASString& name, const UByte flags);
    virtual bool            HasMember(ASStringContext *psc, const ASString& name, bool inclPrototypes);              

    virtual bool            SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val, const PropFlags& flags = PropFlags())
    {   
        SF_UNUSED(psc);
        return SetMember(GetASEnvironment(), name, val, flags);
    }
    virtual bool            GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val)
    {   
        SF_UNUSED(psc);
        return GetMember(GetASEnvironment(), name, val);
    }

    virtual bool            FindMember(ASStringContext *psc, const ASString& name, Member* pmember);
    virtual bool            InstanceOf(Environment* penv, const Object* prototype, bool inclInterfaces = true) const;
    virtual bool            Watch(ASStringContext *psc, const ASString& prop, const FunctionRef& callback, const Value& userData);
    virtual bool            Unwatch(ASStringContext *psc, const ASString& prop);

    virtual void            Set__proto__(ASStringContext *psc, Object* protoObj);
    //virtual Object*      Get__proto__();

    // set this.__proto__ = psrcObj->prototype
    void                    SetProtoToPrototypeOf(ObjectInterface* psrcObj);

    // Override these so they don't cause conflicts due to being present in both of our bases.
    virtual ObjectType      GetObjectType() const   { return ObjectInterface::GetObjectType(); }

    void                    SetFilters(Ptr<FilterSet> filters)    { pDispObj->SetFilters(filters); }
    const FilterSet*        GetFilters() const                    { return pDispObj->GetFilters(); }


    // *** Parent list manipulation functions (work on all types of characters)

    // Duplicate *this* object with the specified name and add it with a new name 
    // at a new depth in our parent (work only if parent is a sprite).
    InteractiveObject*      CloneDisplayObject(const ASString& newname, int depth, const ObjectInterface *psource);
    // Remove *this* object from its parent.
    void                    RemoveDisplayObject();


    // *** Movie Loading support

    // Execute action buffer
    virtual bool            ExecuteBuffer(ActionBuffer* pactionBuffer) 
    { SF_UNUSED(pactionBuffer); return false; }
    // Execute this even immediately (called for processing queued event actions).
    virtual bool            ExecuteEvent(const EventId& id);
    // Execute function
    virtual bool            ExecuteFunction(const FunctionRef& function, const ValueArray& params);
    // Execute ?function
    virtual bool            ExecuteCFunction(const CFunctionPtr function, const ValueArray& params);

    virtual bool            OnEvent(const EventId& id) { SF_UNUSED(id); return false; }

    unsigned                GetVersion() const { return pDispObj->GetVersion(); }

    SF_INLINE void*   operator new        (UPInt , void *ptr)   { return ptr; }
    SF_INLINE void    operator delete     (void *, void *) { }
    SF_INLINE void    operator delete     (void *) { SF_ASSERT(false); }
};

// ***** GFxShapeObj - character implementation with no extra data

// GFxShapeObj is used for non ActionScriptable characters that don't store
// unusual state in their instances. It is used for shapes, morph shapes and static
// text fields.

class GenericDisplayObj : public DisplayObjectBase
{
public:
    Ptr<ShapeBaseCharacterDef>    pDef;

    GenericDisplayObj(CharacterDef* pdef, ASMovieRootBase* pasRoot, InteractiveObject* pparent, ResourceId id);

    virtual CharacterDef* GetCharacterDef() const
    {
        return pDef;
    }


    // These are used for finding bounds, width and height.
    virtual RectF  GetBounds(const Matrix &transform) const
    {       
        return transform.EncloseTransform(pDef->GetBoundsLocal(GetRatio()));
    }   
    virtual RectF  GetRectBounds(const Matrix &transform) const
    {       
        return transform.EncloseTransform(pDef->GetRectBoundsLocal(GetRatio()));
    }   

    virtual bool    PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;

    // Override this to hit-test shapes and make Button-Mode sprites work.
    virtual TopMostResult GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);

    virtual float           GetRatio() const;
    virtual void            SetRatio(float);

protected:
    virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;
};

// SF_INLINE AvmCharacter* DisplayObject::CharToASCharacter()
// {
//     return IsInteractiveObject() ? static_cast<AvmCharacter*>(this) : 0;
// }
// 
// SF_INLINE AvmCharacter* DisplayObject::CharToInteractiveObject_Unsafe()
// {
//     return static_cast<AvmCharacter*>(this);
// }
// 
// SF_INLINE const GASString& AvmCharacter::GetName() const
// {
//     CharacterHandle* pnameHandle = GetCharacterHandle();
//     if (pnameHandle)
//         return pnameHandle->GetName();
//     return GetASEnvironment()->GetBuiltin(ASBuiltin_empty_);
// }

SF_INLINE AvmCharacter* ToAvmCharacter(InteractiveObject* po)
{
    if (!po)
        return NULL;
    SF_ASSERT(po->IsAVM1());
    SF_ASSERT(po->GetAvmObjImpl());
    return static_cast<AvmCharacter*>(po->GetAvmObjImpl()->ToAvmInteractiveObjBase());
}

SF_INLINE const AvmCharacter* ToAvmCharacter(const InteractiveObject* po)
{
    return const_cast<const AvmCharacter*>(ToAvmCharacter(const_cast<InteractiveObject*>(po)));
}

}}} // namespace SF::GFx::AS2


#endif // INC_SF_GFX_AS2CHARACTER_H
