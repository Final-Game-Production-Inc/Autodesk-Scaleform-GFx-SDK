/**************************************************************************

Filename    :   AS3_AvmDisplayObj.h
Content     :   Implementation of AS3-dependent part of DisplayObject and Shape.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_AvmDisplayObj_H
#define INC_AS3_AvmDisplayObj_H

#include "GFx/GFx_ShapeSwf.h"
#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_DisplayObject.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_Event.h"
#include "GFx/AS3/AS3_ObjCollector.h"

namespace Scaleform {

// ***** External Classes
namespace Render {
struct TextFilter;
}
namespace GFx {

class MovieImpl;
class MovieDataDef;
class MovieDefImpl;
class DisplayObjContainer;

namespace AS3 {

class MovieRoot;
class Stage;
class AvmDisplayObjContainer;
class AvmInteractiveObj;
class ASVM;
class VMAppDomain;

// An AS3-dependable part of displaylist's DisplayObject.
class AvmDisplayObj : 
    public NewOverrideBase<StatMV_ActionScript_Mem>,
    public AvmDisplayObjBase
{
private:
    // either one of the pAS3CollectiblePtr and pAS3RawPtr should be not NULL if
    // AS3 object is linked with the display object.
    // If the display object is inserted to display list then pAS3CollectablePtr should be not-NULL;
    // if the display object is not in the display list and it was created as a consequence of 
    // AS3 'new' operator then pAS3RawPtr should be set and pAS3CollectiblePtr should be NULL.
    SPtr<Instances::fl_display::DisplayObject>  pAS3CollectiblePtr;
    Instances::fl_display::DisplayObject*       pAS3RawPtr;

protected:
    DisplayObject*                  pDispObj;
    const char*                     pClassName; // a classname used to create the obj; Id == Invalid in this case.
    VMAppDomain*                    AppDomain;

    enum {
        Flag_RemovedFromStage   = 0x01,
        Flag_FirstFrameCalled   = 0x02
    };
    UByte                       Flags;

private:
    // implementations of ASInteractiveObjBase
    virtual void Bind(DisplayObject* po)
    {
        //SF_ASSERT(po->IsInteractiveObject() && static_cast<InteractiveObject*>(po)->IsAVM1());
        pDispObj = po;
    }

    void                                    SetAS3ObjRaw(Instances::fl_display::DisplayObject* p)
    {
        pAS3RawPtr          = p;
        pAS3CollectiblePtr  = NULL;
    }
    void                                    SetAS3ObjCollectible(Instances::fl_display::DisplayObject* p)
    {
        pAS3CollectiblePtr  = p;
        pAS3RawPtr          = NULL;
    }

#ifdef SF_OBJ_COLLECTOR_ENABLED
public:
    virtual void CollectChildren(class ObjectCollector&) const;
    virtual const char* GetObjectTypeName() const { return "GFx::DisplayObject"; }
#endif

public:
    // Constructor.
    AvmDisplayObj(DisplayObject* pdispObj);
    ~AvmDisplayObj();

    virtual class AvmInteractiveObjBase*    ToAvmInteractiveObjBase() { return NULL; }
    virtual class AvmSpriteBase*            ToAvmSpriteBase()         { return NULL; }
    virtual class AvmButtonBase*            ToAvmButttonBase()        { return NULL; }
    virtual class AvmTextFieldBase*         ToAvmTextFieldBase()      { return NULL; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase(){ return NULL; }

    // Determines the absolute path of the character.
    virtual const char*                     GetAbsolutePath(String *ppath) const;
    virtual bool                            HasEventHandler(const EventId& id) const;
    // method called on any event: enterframe, mouse, key.
    // This method should only put event in appropriate level
    // of action queue, DO NOT execute code right in it!
    virtual bool                            OnEvent(const EventId& id);
    // This method is called when display object is added to display list
    virtual void                            OnEventLoad();
    // This method is called when object is removed from display list and onEventLoad was called
    virtual void                            OnEventUnload();
    // OnRemoved is called when object is removed from the stage by ActionScript 
    // (using removeChild/removeChildAt)
    // It is also called when object is removed from display list and onEventLoad wasn't called
    // (instead, OnAdded was).
    virtual void                            OnRemoved(bool byTimeline);

    virtual bool                            OnUnloading(bool m);

    DisplayObject*                          GetDispObj() const { return pDispObj; }
    MovieRoot*                              GetAS3Root() const;
    MemoryHeap*                             GetHeap() const;

    InteractiveObject*                      GetParent() const 
    { 
        SF_ASSERT(!pDispObj->GetParent() || pDispObj->GetParent()->IsInteractiveObject());
        return pDispObj->GetParent(); 
    }
    AvmInteractiveObj*                      GetAvmParent() const;
    Instances::fl_display::DisplayObject*   GetAS3Parent() const;
    ASString                                GetName() const  { return pDispObj->GetName(); }

    // creates instance of AS class for the object
    // returns 1 if needs ExecuteCode, 0 - otherwise
    virtual unsigned                        CreateASInstance(bool execute);
    bool                                    CreateASInstanceNoCtor();
    unsigned                                CallCtor(bool execute);
    virtual const char*                     GetDefaultASClassName() const { return "flash.display.DisplayObject"; }
    virtual String&                         GetASClassName(String* className) const;

    inline Instances::fl_display::DisplayObject* GetAS3Obj() const;
    // will return a correct pointer even from inside of collector (that might set lower bits to some garbage)
    Instances::fl_display::DisplayObject*   GetAS3ObjSafe() const { return GetAS3Obj(); }
    SPtr<Instances::fl_display::DisplayObject>* GetCollectibleAS3Ptr() { return &pAS3CollectiblePtr; }
    // automatically detects how to store AS3Obj
    void                                    SetAS3Obj(Instances::fl_display::DisplayObject* p);
    enum AS3ObjPtrType { Raw, Collectible };
    void                                    SwitchAS3ObjPtrType(AS3ObjPtrType);
    bool                                    HasAS3Obj() const 
    { 
        return pAS3RawPtr || pAS3CollectiblePtr.GetPtr(); 
    }

    virtual void                            ForceShutdown();

    ASVM*                                   GetAVM();
    void                                    ReleaseAS3Obj();
    void                                    OnDetachFromTimeline();
    void                                    AssignAS3Obj(Instances::fl_display::DisplayObject*);

    DisplayObject*                          GetRoot();
    Stage*                                  GetStage();
    bool                                    IsStageAccessible() const;

    // actually executes the event
    void                                    FireEvent(const EventId& id);
    // propagates events to all children (including itself).
    virtual void                            PropagateEvent(const Instances::fl_events::Event&, bool inclChildren = true);

    // OnAdded is called when object is added to the stage by ActionScript 
    // (using addChild/addChildAt). See also: OnRemoved
    virtual void                            OnAdded(bool byTimeline);

    void                                    InitClassName(const char*);
    bool                                    HasClassName() const { return !!pClassName; }
    const char*                             GetClassName() const { return pClassName;  }
    VMAppDomain&                            GetAppDomain() const { return *AppDomain; }
    void                                    SetAppDomain(VMAppDomain& appDomain);

    virtual bool                            GetObjectsUnderPoint
        (ArrayDH<Ptr<DisplayObjectBase> >* destArray, const PointF& pt) const;

    // Returns true, if an AS3 part (ABC code) of the display object belongs to the
    // provided MovieDefImpl.
    bool                                    DoesAbcBelongTo(MovieDefImpl* defImpl);

    void SetFirstFrameCalledFlag(bool v = true)
    {
        (v) ? Flags |= Flag_FirstFrameCalled : Flags &= ~Flag_FirstFrameCalled;
    }
    bool IsFirstFrameCalledFlagSet() const
    {
        return (Flags & Flag_FirstFrameCalled) != 0;
    }

    // Detaches the child from its parent. It is used by a button to re-insert states.
    virtual bool                            DetachChild(DisplayObjectBase*) { return false; }

    void*   operator new        (UPInt , void *ptr)   { return ptr; }
    void    operator delete     (void *, void *) { }
    void    operator delete     (void *) { SF_ASSERT(false); }
};

// This class represents a DisplayList's object "Shape"
// (analog of GenericCharacter in AS2). AVM-part is AvmDisplayObj.
class ShapeObject : public DisplayObject, public AvmDisplayObj
{
    Ptr<ShapeBaseCharacterDef>  pDef;
    Ptr<DrawingContext>         pDrawing;

    ShapeObject* GetThis() { return this; }
public:
    // Constructor.
    ShapeObject(        
        CharacterDef* pdef,  
        ASMovieRootBase* pasRoot,
        InteractiveObject* pparent, 
        ResourceId id);
    ~ShapeObject();

    virtual CharacterDef* GetCharacterDef() const
    {
        return pDef;
    }

    // These are used for finding bounds, width and height.
    virtual RectF  GetBounds(const Matrix &transform) const;
    virtual RectF  GetRectBounds(const Matrix &transform) const
    {       
        return transform.EncloseTransform(pDef->GetRectBoundsLocal());
    }   

    virtual bool    PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;

    // Override this to hit-test shapes and make Button-Mode sprites work.
    virtual TopMostResult GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);

    virtual DrawingContext* GetDrawingContext();

    virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;

    // AvmDisplayObj methods
    virtual const char*                     GetDefaultASClassName() const { return "flash.display.Shape"; }
    InteractiveObject*                      GetParent() const 
    { 
        return DisplayObject::GetParent(); 
    }

    virtual float           GetRatio() const;
    virtual void            SetRatio(float);

    SF_MEMORY_REDEFINE_NEW(ShapeObject, StatMV_ActionScript_Mem)

#ifdef SF_OBJ_COLLECTOR_ENABLED
public:
    virtual const char* GetObjectTypeName() const { return "GFx::Shape"; }
#endif
};

SF_INLINE AvmDisplayObj* ToAvmDisplayObj(DisplayObject* po)
{
    if (!po)
        return NULL;
    SF_ASSERT(po->IsAVM2());
    SF_ASSERT(po->GetAvmObjImpl());
    return static_cast<AvmDisplayObj*>(po->GetAvmObjImpl());
}

SF_INLINE const AvmDisplayObj* ToAvmDisplayObj(const DisplayObject* po)
{
    return const_cast<const AvmDisplayObj*>(ToAvmDisplayObj(const_cast<DisplayObject*>(po)));
}

SF_INLINE Instances::fl_display::DisplayObject* AvmDisplayObj::GetAS3Obj() const
{
    union
    {
        UPInt v;
        Instances::fl_display::DisplayObject* p;
    } u;
    u.p = (pAS3RawPtr)?pAS3RawPtr:pAS3CollectiblePtr.GetPtr();
    if (u.v & 1)
        u.v -= 1;
    return u.p;
}

}}} // namespace SF::GFx::AS3

#endif // INC_AS3_AvmDisplayObj_H
