/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_DisplayObject.h
Content     :   Implementation of DisplayObjectBase
Created     :   
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_DisplayObject_H 
#define INC_SF_GFX_DisplayObject_H
#include "Kernel/SF_Array.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Debug.h"

#include "Render/Render_States.h"
#include "Render/Render_Matrix3x4.h"
#include "GFx/GFx_Log.h"

#include "Render/Render_Types2D.h"
#include "Render/Render_TreeNode.h"
#include "Render/Render_Scale9Grid.h"

#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_ASString.h"
#include "GFx/GFx_ASMovieRootBase.h"
#include "GFx/GFx_ASUtils.h"

namespace Scaleform { 

namespace Render { namespace Text {
    struct TextFilter;
}}

namespace GFx {

using Render::FilterSet;
using Render::Text::TextFilter;
class MovieImpl;
class EventId;
class FontManager;

class DisplayObjectBase;
class DisplayObject;
class DisplayObjContainer;
class InteractiveObject;
class DrawingContext;
class Sprite;
class ResourceBinding;


// Interface for implementing VM-specific part of DisplayObjectBase
class AvmDisplayObjBase
{
public:
    virtual ~AvmDisplayObjBase() {}

    virtual class AvmInteractiveObjBase*    ToAvmInteractiveObjBase() =0;
    virtual class AvmSpriteBase*            ToAvmSpriteBase() =0;
    virtual class AvmButtonBase*            ToAvmButttonBase() =0;
    virtual class AvmTextFieldBase*         ToAvmTextFieldBase() =0;
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() =0;

    virtual const char*                     GetAbsolutePath(String *ppath) const =0;
    virtual bool                            HasEventHandler(const EventId& id) const =0;
    virtual bool                            OnEvent(const EventId& id) =0;
    virtual void                            OnEventLoad()   =0;
    // called if displaylist removes the obj and onEventLoad was called for it.
    virtual void                            OnEventUnload() =0;
    // called if displaylist removes the obj but onEventLoad wasn't called for it
    //virtual void                            OnRemoved(bool byTimeline) =0;
    virtual bool                            OnUnloading(bool mayRemove)   =0;

    // convenience "const" methods
    SF_INLINE const class AvmInteractiveObjBase* ToAvmInteractiveObjBase() const
    {
        return const_cast<const class AvmInteractiveObjBase*>(
            const_cast<AvmDisplayObjBase*>(this)->ToAvmInteractiveObjBase());
    }
    SF_INLINE const class AvmSpriteBase*         ToAvmSpriteBase() const
    {
        return const_cast<const class AvmSpriteBase*>(
            const_cast<AvmDisplayObjBase*>(this)->ToAvmSpriteBase());
    }
    SF_INLINE const class AvmButtonBase*         ToAvmButttonBase() const
    {
        return const_cast<const class AvmButtonBase*>(
            const_cast<AvmDisplayObjBase*>(this)->ToAvmButttonBase());
    }
    SF_INLINE const class AvmTextFieldBase*      ToAvmTextFieldBase() const
    {
        return const_cast<const class AvmTextFieldBase*>(
            const_cast<AvmDisplayObjBase*>(this)->ToAvmTextFieldBase());
    }

#ifdef GFX_ENABLE_VIDEO
    virtual class AvmVideoCharacterBase* ToAvmVideoCharacterBase() { return NULL; }
    SF_INLINE const class AvmVideoCharacterBase* ToAvmVideoCharacterBase() const
    {
        return const_cast<const class AvmVideoCharacterBase*>(
            const_cast<AvmDisplayObjBase*>(this)->ToAvmVideoCharacterBase());
    }
#endif
};

// DisplayObjectBase is a live, active state instance of a CharacterDef.
// It represents a single active element in a display list and thus includes
// the physical properties necessary for all objects on stage.
// However, it does not include the properties necessary for scripting
// or external access - that is the job of InteractiveObject.

class DisplayObjectBase : public RefCountBaseWeakSupport<DisplayObjectBase, StatMV_MovieClip_Mem>,
                          public LogBase<DisplayObjectBase>
{
    friend class MovieImpl;
public:
    enum
    {
        INVALID_DEPTH                   = -1
    };

    // Public typedefs
    typedef Matrix2F                    Matrix;
    typedef Render::Cxform              Cxform;
    typedef Render::BlendMode           BlendType;

    struct GeomDataType : public NewOverrideBase<StatMV_MovieClip_Mem>
    {
        int     X, Y; // stored in twips
        Double  XScale, YScale;
        Double  Rotation;
        Matrix  OrigMatrix;
        Double  Z;       
        Double  ZScale;                 // this is a percentage, ie. 100
        Double  XRotation, YRotation;   // in degrees

        GeomDataType() { X=Y=0; Rotation = 0; XScale=YScale = 100; Z=XRotation=YRotation=0; ZScale=100; }
    };

    struct IndirectTransformDataType : public NewOverrideBase<StatMV_MovieClip_Mem> 
    {
        Matrix3F OrigTransformMatrix; // matrix that would be on treenode 
                                      // if there is no indirect transform.
        bool     IsOrig3D;
    };  

    // Holds variables for Flash Perspective settings.
    // This struct will not be allocated (NULL) for 2D objects and 
    // 3D objects which do not override their parents perspective settings.
    // struct members can be set to 0 to indicate that the particular member is not used.
	// except in the case of ProjectionCenter, which should be set to NaN (0,0 is a valid value).
    struct PerspectiveDataType : public NewOverrideBase<StatMV_MovieClip_Mem>
    {
        Double                  FieldOfView;	    // 3D perspective field of view in degrees
        Double                  FocalLength;		// 3D perspective focal length, in TWIPS
        PointF				    ProjectionCenter;	// 3D center of projection (vanishing point) in stage coord TWIPS
        Matrix3F                ViewMatrix3D;       // This is the ViewMatrix3D without the viewport adjustment (for Flash hit test).
        PerspectiveDataType() : FieldOfView(0), FocalLength(0)
        {
            ProjectionCenter.x = (float)NumberUtil::NaN();
            ProjectionCenter.y = (float)NumberUtil::NaN();
        }

        // Returns whether the the View and Perspective matrices should be calculated from the FOV/FL, or
        // whether the values set using the DAPI (SetViewMatrix3D/SetProjectionMatrix3D) should be used.
        bool UpdateViewAndPerspectiveFromData() const
        {
            return FieldOfView != 0 || FocalLength != 0;
        }
    };

protected:
    enum FlagMasks
    {
        Mask_Scale9GridExists   = 0x01,
        Mask_TopmostLevel       = 0x02,
        Mask_HitTestNegative    = 0x04,
        Mask_HitTestPositive    = 0x08,
        Mask_HitTest            = Mask_HitTestNegative | Mask_HitTestPositive,
        Mask_Unloaded           = 0x10, // Set once Unload event has been called.
        Mask_JustLoaded         = 0x20, // Set once char is loaded
        Mask_MarkedForRemove    = 0x40, // Char is marked for remove from the DisplayList 
        Mask_InteractiveObject  = 0x80,
        Mask_ScriptableObject   = 0x100,
        Mask_DisplayObjContainer= 0x200,
        Mask_Sprite             = 0x400, // doesn't have timeline
        Mask_MovieClip          = 0x800, // have timeline
        Mask_Unloading          = 0x1000,// Set when OnEventUnload is about to be called.
        Mask_Loaded             = 0x2000, // Set when onEventLoad was called
        Mask_Visible            = 0x4000,
        Mask_IndirectTransform  = 0x8000
    };

    void SetInteractiveObjectFlag()     { Flags |= Mask_InteractiveObject; }
    void SetScriptableObjectFlag()      { Flags |= Mask_ScriptableObject; }
    void SetDisplayObjContainerFlag()   { Flags |= Mask_DisplayObjContainer; }
    void SetSpriteFlag()                { Flags |= Mask_Sprite; }

    void SetVisibleFlag(bool v = true);
    void ClearVisibleFlag()            { SetVisibleFlag(false); }
    bool IsVisibleFlagSet() const      { return (Flags & Mask_Visible) != 0; }

    virtual Ptr<Render::TreeNode>   CreateRenderNode(Render::Context& context) const =0;
    Render::TreeContainer*          ConvertToTreeContainer();

    // Sets indirect transform for 'this' obj that will use its current parent as transform parent.
    // The TreeNode for 'this' obj is removed from its current location in render tree, 
    // and it is not re-inserted into it; it is caller's responsibility to re-insert it into the render tree
    // at appropriate position. Returns the detached render node.
    // 'newParent' node is used only for safety check, to avoid circular references. Can be NULL.
    Ptr<Render::TreeNode>  SetIndirectTransform(Render::TreeNode* newParent);
    void                   RemoveIndirectTransform(bool readdToDisplayList=true);
public:

    // Constructor
    DisplayObjectBase(ASMovieRootBase* pasRoot, InteractiveObject* pparent, ResourceId id);
    ~DisplayObjectBase();

    void                    BindAvmObj(AvmDisplayObjBase* p);
    bool                    HasAvmObject() const { return AvmObjOffset != 0; }
    AvmDisplayObjBase*      GetAvmObjImpl();
    const AvmDisplayObjBase* GetAvmObjImpl() const;

    void                    SetTopmostLevelFlag(bool v = true) 
    { 
        (v) ? Flags |= Mask_TopmostLevel : Flags &= (~Mask_TopmostLevel); 
    }
    void                    ClearTopmostLevelFlag()            
    { 
        SetTopmostLevelFlag(false); 
    }
    //void SetDirtyFlag();

    // *** Accessors for physical display info

    ResourceId              GetId() const                       { return Id; }
    InteractiveObject*      GetParent() const                   { return pParent; }
    void                    SetParent(InteractiveObject* parent)   { pParent = parent; }  // for extern GFxMovieSub
    int                     GetDepth() const                    { return Depth; }
    void                    SetDepth(int d)                     { Depth = d; }  
    unsigned                GetCreateFrame() const              { return CreateFrame;}
    void                    SetCreateFrame(unsigned frame)          { CreateFrame = frame; }
    virtual const Matrix&   GetMatrix() const;
    virtual void            SetMatrix(const Matrix& m);
    virtual const Matrix3F& GetMatrix3D() const;
    virtual void            SetMatrix3D(const Matrix3F& m);
    virtual bool            GetProjectionMatrix3D(Matrix4F *m, bool bInherit=false) const;
    virtual void            SetProjectionMatrix3D(const Matrix4F& m);
    virtual void            ClearProjectionMatrix3D();
    virtual bool            GetViewMatrix3D(Matrix3F *m, bool bInherit=false) const;
    virtual void            SetViewMatrix3D(const Matrix3F& m);
    virtual void            ClearViewMatrix3D();
    virtual void            UpdateViewAndPerspective();
    virtual void            UpdateTransform3D();
    virtual bool            Has3D() { return Is3D(); }          // checks object and descendants
    bool                    Is3D(bool bInherit) const;          // checks object and optionally ancestors
    bool                    Is3D() const                        { return (pRenNode && pRenNode->Is3D()); }
    void                    Clear3D(bool bInherit=false);
    const Cxform&           GetCxform() const;
    void                    SetCxform(const Cxform& cx);
    void                    ConcatenateCxform(const Cxform& cx);
    void                    ConcatenateMatrix(const Matrix& m);
	virtual BlendType       GetBlendMode() const;
	virtual void            SetBlendMode(BlendType blend);
    virtual float           GetRatio() const { return 0; }
    virtual void            SetRatio(float)  {}
    UInt16                  GetClipDepth() const                { return ClipDepth; }
    void                    SetClipDepth(UInt16 d)              { ClipDepth = d; }

    bool                    IsAncestor(DisplayObjectBase* ch) const;
    void                    SetScale9GridExists(bool v)      
    { 
        (v) ? Flags |= Mask_Scale9GridExists : Flags &= (~Mask_Scale9GridExists); 
    }
    bool                    DoesScale9GridExist() const         { return (Flags & Mask_Scale9GridExists) != 0; }
    virtual void            SetScale9Grid(const RectF& rect);
    RectF                   GetScale9Grid() const;
    bool                    HasScale9Grid() const { return !GetScale9Grid().IsEmpty(); }
    virtual void            PropagateScale9GridExists() {}
    Render::Scale9GridInfo* CreateScale9Grid() const;

    bool                    IsTopmostLevelFlagSet() const       { return (Flags & Mask_TopmostLevel) != 0; }

    // sets coordinate X, specified in pixels.
    virtual void            SetX(Double x);
    virtual Double          GetX() const;
    // sets coordinate Y, specified in pixels.
    virtual void            SetY(Double y);
    virtual Double          GetY() const;
    // sets coordinate Z, specified in world units.
    virtual void            SetZ(Double yz);
    virtual Double          GetZ() const;
    // sets X scale, in percents [0..100%...].
    virtual void            SetXScale(Double xscale);
    virtual Double          GetXScale() const;
    // sets Y scale, in percents [0..100%...].
    virtual void            SetYScale(Double yscale);
    virtual Double          GetYScale() const;
    // sets Z scale, in percents [0..100%...].
    virtual void            SetZScale(Double zscale);
    virtual Double          GetZScale() const;
    // sets (Z) rotation, in degrees [-180..0..180]
    virtual void            SetRotation(Double rotation);
    virtual Double          GetRotation() const;
    // sets X rotation, in degrees [-180..0..180]
    virtual void            SetXRotation(Double xrotation);
    virtual Double          GetXRotation() const;
    // sets Y rotation, in degrees [-180..0..180]
    virtual void            SetYRotation(Double yrotation);
    virtual Double          GetYRotation() const;
    // set 3D perspective field of view
    virtual void            SetFOV(Double fovdeg);
    virtual Double          GetFOV() const;
    // set 3D perspective focal length (in TWIPS)
    virtual void            SetFocalLength(Double focalLength);
    virtual Double          GetFocalLength() const;
    // set 3D perspective center of projection (in TWIPS)
    virtual void            SetProjectionCenter(PointF projCenter);
    virtual PointF          GetProjectionCenter() const;
    // sets width, in pixels
    virtual void            SetWidth(Double width);
    virtual Double          GetWidth() const;
    // sets height, in pixels
    virtual void            SetHeight(Double height);
    virtual Double          GetHeight() const;
    // sets alpha, in percents [0..100].
    void                    SetAlpha(Double alpha);
    Double                  GetAlpha() const;

    // returns mouse x coordinate, in displayobject's coordinate space, in pixels
    virtual Double          GetMouseX() const { return NumberUtil::NaN(); }
    // returns mouse y coordinate, in displayobject's coordinate space, in pixels
    virtual Double          GetMouseY() const { return NumberUtil::NaN(); }

    // Implemented only in GASCharacter. Return None/empty string by default on other characters.
    //virtual const GASString& GetName() const;
    //virtual const GASString& GetOriginalName() const            { return GFxCharacter::GetName(); }     
    virtual bool            GetAcceptAnimMoves() const          { return true;  }
    virtual bool            GetContinueAnimationFlag() const    { return false; }
    // If any of below ASSERTs is hit, it is either because (1) their states are being set by tags
    // and therefore should be moved from InteractiveObject here, or (2) because there is a bug
    // in external logic.
    virtual void            SetAcceptAnimMoves(bool accept)     { SF_UNUSED(accept); SF_ASSERT(0); }
  
    virtual bool            GetCacheAsBitmap() const;
    virtual void            SetCacheAsBitmap(bool enable);

    virtual void            SetFilters(const FilterSet* filters);
    virtual const FilterSet*GetFilters() const;

    virtual void            SetVisible(bool visible)            { SetVisibleFlag(visible); SetDirtyFlag(); }
    virtual bool            GetVisible() const                  { return IsVisibleFlagSet(); }

    virtual void            ForceShutdown () {}

    // Name should not be necessary for text; however, it was seen for static text - needs research.

// Do we need them here? (AB)
//     virtual void            SetName(const ASString& name)
//     { SF_DEBUG_WARNING1(1, "GFxCharacter::SetName('%s') called on a non-scriptable character", name.ToCStr()); SF_UNUSED(name); }
//     virtual void            SetOriginalName(const ASString& name) { SetName(name); }

    bool                    CheckLastHitResult(float x, float y) const; 
    bool                    WasLastHitPositive() const { return (Flags & Mask_HitTestPositive) != 0; }
    void                    SetLastHitResult(float x, float y, bool result) const; // TO DO: Revise "const"
    void                    InvalidateHitResult() const;                           // TO DO: Revise "const"

    void                    SetLoaded(bool v = true)   
    { 
        (v) ? (Flags |= Mask_Loaded)  : (Flags &= (~Mask_Loaded)); 
    }
    void                    SetUnloaded(bool v = true)   
    { 
        if (!v) SetUnloading(false); // clear "unloading" if clearing "unloaded"
        (v) ? (Flags |= Mask_Unloaded)  : (Flags &= (~Mask_Unloaded)); 
    }
    void                    SetJustLoaded(bool v = true) 
    { 
        (v) ? (Flags |= Mask_JustLoaded): (Flags &= (~Mask_JustLoaded)); 
    }
    bool                    IsLoaded() const             { return (Flags & Mask_Loaded) != 0; }    
    bool                    IsUnloaded() const           { return (Flags & Mask_Unloaded) != 0; }    
    bool                    IsJustLoaded() const         { return (Flags & Mask_JustLoaded) != 0; }    

    // Unloading flag is set when OnUnloading is called, thus, OnEventUnload will have this
    // flag already set.
    void                    SetUnloading(bool v = true)   
    { 
        (v) ? (Flags |= Mask_Unloading)  : (Flags &= (~Mask_Unloading)); 
    }
    bool                    IsUnloading() const          { return (Flags & Mask_Unloading) != 0 || IsUnloadQueuedUp(); }    
    // If sprite is unloading (onUnload is queued up) then it is transferred to the depth = -32769 - curDepth
    // which equals to -1 - curDepth in GFx (we shift depth by +16384)
    bool                    IsUnloadQueuedUp() const     { return (Depth < -1); }    

    // *** Inherited transform access.

    // Get our concatenated matrix (all our ancestor transforms, times our matrix).
    // Maps from our local space into "world" space (i.e. root movie clip space).
    void                    GetWorldMatrix(Matrix *pmat) const;
    void                    GetWorldMatrix3D(Matrix3F *pmat) const;     // returns complete 2D + 3D world matrix
    void                    GetWorldCxform(Cxform *pcxform) const;      // returns complete 2D + 3D local matrix
    void                    GetLevelMatrix(Matrix *pmat) const;
    void                    GetLocalMatrix3D(Matrix3F *pmat) const;
    // Convenience versions.
    Matrix                  GetWorldMatrix() const              { Matrix m; GetWorldMatrix(&m); return m; }
    Matrix3F                GetWorldMatrix3D() const            { Matrix3F m; GetWorldMatrix3D(&m); return m; }
    Cxform                  GetWorldCxform() const              { Cxform m; GetWorldCxform(&m); return m; }
    Matrix                  GetLevelMatrix() const              { Matrix m; GetLevelMatrix(&m); return m; }
    Matrix3F                GetLocalMatrix3D() const            { Matrix3F m; GetLocalMatrix3D(&m); return m; }

	// Converts a [3D] point of the [3d] display object's (local) coordinates to 
	// a 2D point in the Stage (global) coordinates. Same as AS DisplayObject:localToGlobal.
	Render::PointF Local3DToGlobal(const Render::Point3F &ptIn) const;
	Render::PointF LocalToGlobal(const Render::PointF &ptIn) const		
		{ return Local3DToGlobal(Render::Point3F(ptIn.x, ptIn.y, 0)); }

	// Converts a 2D point from the Stage (global) coordinates to a [3D] display 
	// object's (local) coordinates.  See AS DisplayObject::globalToLocal
	Render::PointF GlobalToLocal(const Render::PointF &ptIn) const;		
	Render::Point3F GlobalToLocal3D(const Render::PointF &ptIn) const;	

    // Temporary - used until blending logic is improved.
    BlendType               GetActiveBlendMode() const;


    // *** Geometry query methods, applicable to all characters.

    // Return character bounds in specified coordinate space.
    virtual RectF           GetBounds(const Matrix &t) const     { SF_UNUSED(t); return RectF(0); }
    RectF                   GetBoundsIn3D() const;
    // Returns character bounds excluding stroke
    virtual RectF           GetRectBounds(const Matrix &t) const { SF_UNUSED(t); return RectF(0); }
    // Return transformed bounds of character in root movie space.
    SF_INLINE RectF         GetWorldBounds() const              { return GetBounds(GetWorldMatrix()); }
    SF_INLINE RectF         GetLevelBounds() const              { return GetBounds(GetLevelMatrix()); }

    // Hit-testing of shape/sprite.
    enum HitTestMask
    {
        HitTest_TestShape       = 1,
        HitTest_IgnoreInvisible = 2
    };
    // Test if point is inside of the movieclip. Coordinate is in child's coordspace.
    virtual bool            PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const 
    { SF_UNUSED2(pt, hitTestMask); return false; }      

    struct TopMostDescr
    {
        // "out" 
        InteractiveObject*          pResult;
        Render::PointF              LocalPt; // local point; set only if TopMost_Continue

        // "in"
        const InteractiveObject*    pIgnoreMC;
        const DisplayObject*        pHitArea;
        unsigned                    ControllerIdx;
        bool                        TestAll;

        TopMostDescr():pIgnoreMC(NULL), pHitArea(NULL), ControllerIdx(0), TestAll(false) {}
    };
    enum TopMostResult
    {
        TopMost_Found           = 1,// pResult is set
        TopMost_FoundNothing    = 2,// pResult is null, no further search is allowed
        TopMost_Continue        = 3 // pResult is null, but further search is allowed
                                    // if pResult is not null then this is a potential target. It continues search,
                                    // but if nothing is found then this result might be used.
    };
    // Will always find action scriptable objects (parent is returned for regular shapes).
    // Coords are in parent's frame.
    virtual TopMostResult       GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr) 
    { SF_UNUSED(pt); pdescr->pResult = 0; return TopMost_FoundNothing; }

    // Transforms a point into local space for hit-testing, takes into account 3D objects. 
    void                        TransformPointToLocal(Render::PointF *p, const Render::PointF & pt, 
                                                      bool bPtInParentSpace = true, const Matrix2F *mat = NULL) const;

    // *** Resource/Asset Accessors

    // Returns the absolute root object, which corresponds to movie view and contains levels.
    MovieImpl*              FindMovieImpl() const;               

    // Obtains character definition relying on us. Must be overridden.
    virtual CharacterDef*   GetCharacterDef() const            = 0;

    // Returns SWF nested movie definition from which we access movie resources.
    // This definition may have been imported or loaded with 'loadMovie', so
    // it can be a nested movie (it does not need to correspond to root).
    virtual MovieDefImpl*   GetResourceMovieDef() const { return (pParentChar) ? pParentChar->GetResourceMovieDef() : NULL; }

    // Obtain the font manager. Font managers exist in sprites with their own MovieDefImpl.
    virtual FontManager*    GetFontManager() const      { return (pParentChar) ? pParentChar->GetFontManager() : NULL; }

    // Returns a movie for a certain level.
//    virtual InteractiveObject* GetLevelMovie(int level) const     { return pParentChar->GetLevelMovie(level); }

    // Returns _root movie, which may or many not be the same as _level0.
    // The meaning of _root may change if _lockroot is set on a clip into which
    // a nested movie was loaded.
    virtual InteractiveObject* GetTopParent(bool ignoreLockRoot = false) const 
    { 
        if (!pParentChar)
            return NULL;
        return pParentChar->GetTopParent(ignoreLockRoot);
    }

    // Helper that gets movie-resource relative versions.
    // Necessary because version of SWF execution depends on loadMovie nesting.
    unsigned                GetVersion() const;
    SF_INLINE bool          IsCaseSensitive() const { return GetVersion() > 6; }

    // *** GFxDisplayList required methods  

    // Returns a renderer tree node associated with the display object. If there is no
    // renderer node created it will create one using CreateRenderNode method
    Render::TreeNode*       GetRenderNode() const;
    // Returns a ptr to TreeContainer node, if possible; NULL, otherwise.
    // Buttons, Sprites return pRenNode casted to TreeContainer. Shapes return NULL.
    virtual Render::TreeContainer* GetRenderContainer() { return NULL; }
    // Returns render context
    Render::Context&        GetRenderContext() const;

    // Called from DisplayList on ReplaceDisplayObject.
    virtual void            Restart()                               { }

    // Advance Frame / "tick" functionality.
    // Set nextFrame = 1 if the frame is advanced by one step, 0 otherwise.
    // framePos must be in range [0,1) indicating where in a frame we are;
    // that could be useful for some animation optimizations.
    virtual void            AdvanceFrame(bool nextFrame, float framePos = 0.0f) 
    { SF_UNUSED2(nextFrame, framePos); }

    // ActionScript event handler.  Returns true if a handler was called.
    virtual bool            OnEvent(const EventId& ) { return false; }
    // Special event handler; sprites also execute their frame1 actions on this event.
    virtual void            OnEventLoad()               { SetLoaded(); }
    // Special event handler; ensures that unload is called on child items in timely manner.
    virtual bool            OnUnloading();
protected:
    virtual void            OnEventUnload();
    //void                    OnRemoved() { GetAvmObjImpl()->OnRemoved(); }

public:
    void                    EnsureGeomDataCreated();
    void                    OnUnload() 
    { 
        OnEventUnload();
        //(Flags & Mask_Loaded) ? OnEventUnload() : OnRemoved();
    }


    // *** Log support.

    // Implement logging through the root delegation.
    // (needs to be implemented in .cpp, so that MovieImpl is visible)
    virtual Log*            GetLog() const;
    virtual bool            IsVerboseAction() const;
    virtual bool            IsVerboseActionErrors() const;

    // Dynamic casting support - necessary to support ToASCharacter.
//    virtual ObjectType      GetObjectType() const   { return GASObjectInterface::Object_BaseCharacter; }

    // Methods IsInteractiveObject and CharToASCharacter are similar to 
    // GASObjectInterface's IsASCharacter and ToASCharacter, but they are not virtual in 
    // faster., CharToASCharacter_Un
    bool                        IsInteractiveObject() const   
    { 
        return (Flags & Mask_InteractiveObject) != 0; 
    }
    InteractiveObject*          CharToInteractiveObject(); 
    const InteractiveObject*    CharToInteractiveObject() const; 
    InteractiveObject*          CharToInteractiveObject_Unsafe(); // doesn't check for IsInteractiveObject
    const InteractiveObject*    CharToInteractiveObject_Unsafe() const; // doesn't check for IsInteractiveObject

    bool                        IsScriptableObject() const   
    { 
        return (Flags & Mask_ScriptableObject) != 0; 
    }
    DisplayObject*              CharToScriptableObject(); 
    const DisplayObject*        CharToScriptableObject() const; 
    DisplayObject*              CharToScriptableObject_Unsafe(); // doesn't check for IsScriptableObject
    const DisplayObject*        CharToScriptableObject_Unsafe() const; // doesn't check for IsScriptableObject

    bool                        IsDisplayObjContainer() const   
    { 
        return (Flags & Mask_DisplayObjContainer) != 0; 
    }
    DisplayObjContainer*        CharToDisplayObjContainer(); 
    const DisplayObjContainer*  CharToDisplayObjContainer() const; 
    DisplayObjContainer*        CharToDisplayObjContainer_Unsafe(); // doesn't check for IsDisplayObjContainer
    const DisplayObjContainer*  CharToDisplayObjContainer_Unsafe() const; // doesn't check for IsDisplayObjContainer

    bool                        IsSprite() const   
    { 
        return (Flags & Mask_Sprite) != 0; 
    }

    Sprite*                     CharToSprite(); 
    const Sprite*               CharToSprite() const; 
    Sprite*                     CharToSprite_Unsafe(); // doesn't check for IsSprite
    const Sprite*               CharToSprite_Unsafe() const; // doesn't check for IsSprite

    void SetMovieClipFlag()     { SF_ASSERT(IsSprite()); Flags |= Mask_MovieClip; }
    bool                        IsMovieClip() const   
    { 
        SF_ASSERT(IsSprite());
        return (Flags & Mask_MovieClip) != 0; 
    }

    virtual bool                IsUsedAsMask() const { return false; }

    enum StateChangedFlags
    {
        StateChanged_FontLib       = 0x1,
        StateChanged_FontMap       = 0x2,
        StateChanged_FontProvider  = 0x4,
        StateChanged_Translator    = 0x8
    };
    virtual void SetStateChangeFlags(UInt8 flags) { SF_UNUSED(flags); }
    virtual UInt8 GetStateChangeFlags() const { return 0; }

    const IndirectTransformDataType* GetIndirectTransformData() const { return pIndXFormData; }

    bool HasIndirectTransform() const { return (Flags & Mask_IndirectTransform) != 0; }

    SF_INLINE bool          IsMarkedForRemove() const   
    { 
        return ( Flags & Mask_MarkedForRemove) != 0; 
    }
    // Only advance characters if they are not just loaded and not marked for removal.
    SF_INLINE bool          CanAdvanceChar() const      
    { 
        return !(Flags & Mask_MarkedForRemove); 
    }

    SF_INLINE void          MarkForRemove(bool remove)
    {
        if (remove) Flags |= Mask_MarkedForRemove;
        else        Flags &= ~Mask_MarkedForRemove;
    }
    SF_INLINE void          ClearMarkForRemove() { MarkForRemove(false); }

    GeomDataType&           GetGeomData(GeomDataType&) const;
    void                    SetGeomData(const GeomDataType&);
    GeomDataType*           GetGeomDataPtr() { return pGeomData; }
    
    const PerspectiveDataType*    GetPerspectiveDataPtr() const { return pPerspectiveData; }

    virtual CharacterDef::CharacterDefType GetType() const { return CharacterDef::Shape; }

    // Override this to hit-test shapes and make Button-Mode sprites work.
    InteractiveObject*  GetTopMostMouseEntityDef
        (CharacterDef* pdef, const Render::PointF &pt, bool testAll = false, const InteractiveObject* ignoreMC = NULL);

    MovieImpl*        GetMovieImpl() const    { return pASRoot->GetMovieImpl(); }
    ASMovieRootBase*  GetASMovieRoot() const  { return pASRoot; }

    int GetAVMVersion() const { return pASRoot->GetAVMVersion(); }
    bool IsAVM1() const { return GetAVMVersion() == 1; }
    bool IsAVM2() const { return GetAVMVersion() == 2; }

    void                    SetDirtyFlag();

    void            SetRendererString(const char*);
    const char*     GetRendererString();
    void            SetRendererFloat(float);
    float           GetRendererFloat();
    void            DisableBatching(bool);
    bool            IsBatchingDisabled();

    SF_MEMORY_DEFINE_PLACEMENT_NEW;
    SF_MEMORY_REDEFINE_NEW(DisplayObjectBase, StatMV_MovieClip_Mem);                                  \

protected: // data
    ASMovieRootBase*        pASRoot;
    // These are physical properties of all display list entries.
    // Properties that belong only to scriptable objects, such as _name are in InteractiveObject.
    ResourceId              Id;
    int                     Depth;
    unsigned                CreateFrame;
    union   
    {
        InteractiveObject*  pParent;        // Parent is always InteractiveObject.     
        DisplayObjectBase*  pParentChar;    // So that we can access it directly.
    };

    mutable float           LastHitTestX; // TO DO: Revise "mutable". 
    mutable float           LastHitTestY; // TO DO: Revise "mutable". 
    mutable Ptr<Render::TreeNode> pRenNode;

    GeomDataType*           pGeomData;
    PerspectiveDataType*    pPerspectiveData;
    IndirectTransformDataType* pIndXFormData;

    UInt16                  ClipDepth;
    mutable UInt16          Flags; // TO DO: Revise "mutable". 
    UInt8                   BlendMode;
    UInt8                   AvmObjOffset;	// offset*4 from the beginning of the obj to AvmDisplayObjBase

};

class CharacterHandle : public NewOverrideBase<StatMV_MovieClip_Mem>
{
    friend class DisplayObject;
    friend class InteractiveObject;
    friend class Sprite;

    int                         RefCount;       // Custom NTS ref-count implementation to save space.   
    DisplayObject*              pCharacter;     // Character, can be null if it was destroyed!

    // This name is the same as object reference for all levels except _levelN.
    ASString                    Name;
    // Full Path from root, including _levelN. Used to resolve dead characters.
    ASString                    NamePath;
    // Original placement name; Used to correctly handle life cycle of the character,
    // since the 'Name' might be changed via ActionScript. See GFxSprite::AddDisplayObject.
    ASString                    OriginalName; 

    // Release a character reference, used when character dies
    void                        ReleaseCharacter();

public:

    CharacterHandle(const ASString& pname, DisplayObject *pparent, DisplayObject* pcharacter = 0);
    // Must remove from parent hash and clean parent up.
    ~CharacterHandle();

    const ASString&             GetName() const         { return Name; }
    const ASString&             GetNamePath() const     { return NamePath; }
    DisplayObject*              GetCharacter() const    { return pCharacter; }
    const ASString&             GetOriginalName() const { return OriginalName; }
    void                        SetOriginalName(const ASString& on) { OriginalName = on; }

    // Resolve the character, considering path if necessary.
    DisplayObject*              ResolveCharacter(MovieImpl *poot) const; 
    DisplayObject*              ForceResolveCharacter(MovieImpl *poot) const;

    // Ref-count implementation.
    void                        AddRef()                { RefCount++; }
    SF_INLINE void              Release(unsigned flags=0)   
    { 
        SF_UNUSED(flags); 
        RefCount--; 
        if (RefCount<=0) 
            delete this; 
    }

    // Changes the name.
    void                        ChangeName(const ASString& pname, DisplayObject* pparent);

    // Resets names to blank ones
    void                        ResetName(ASStringManager* psm);
};

class DisplayObject : public DisplayObjectBase
{
protected:
    struct DisplayContextStates
    {
        Matrix2F*           pParentMatrix;
        Cxform*             pParentCxform;
        ResourceBinding*    pResourceBinding;

        Matrix2F            CurMatrix;
        Cxform              CurCxform;

        bool                Masked;
    };

    mutable Ptr<CharacterHandle>    pNameHandle;
    union
    {
        DisplayObject*              pMaskCharacter;  // a refcnt ptr to mask clip
        DisplayObject*              pMaskOwner;     // a weak ptr to mask owner clip
    };
    // ScrollRectInfo contains info necessary to implement 'scrollRect'.
    struct ScrollRectInfo : public NewOverrideBase<StatMV_MovieClip_Mem> 
    {
        Render::RectD               Rectangle;
        Ptr<DrawingContext>         Mask;
        Matrix3F OrigTransformMatrix; // matrix that would be on treenode 
                                      // if there is no scrollRect.
        bool     IsOrig3D;
    };
    ScrollRectInfo*                 pScrollRect;

private:
    enum FlagMasks
    {
        Mask_TimelineObject     = 0x01,
        Mask_InstanceBasedName  = 0x02,
        Mask_UsedAsMask         = 0x04,  // movie is used as a mask for setMask method
        Mask_AcceptAnimMoves    = 0x08,
        Mask_ContinueAnimation  = 0x10, // cached extension flag "continueAnimation"
                                        // (it continues timeline anim after AS touch)
        Mask_ExecutionAborted   = 0x20  // execution is aborted (due to exception)
    };
    UInt16                          DOFlags; 

protected:
    virtual ASString        CreateNewInstanceName() const;
    CharacterHandle*        CreateCharacterHandle() const;

    // If the object has ClipDepth this means it is used as timeline mask. 
    // Remove its render node from the render tree, set clip depth to 0 and
    // reinsert back as a regular node (as non-mask).
    void                    ResetClipDepth();

public:
    DisplayObject(ASMovieRootBase* pasRoot, InteractiveObject* pparent, ResourceId id);
    ~DisplayObject();

    // This method may create drawing context on demand. Used for
    // drawing API.
    virtual DrawingContext* GetDrawingContext() { return NULL; }

    virtual const Matrix&   GetMatrix() const;
    virtual void            SetMatrix(const Matrix& m);
    virtual const Matrix3F& GetMatrix3D() const;
    virtual void            SetMatrix3D(const Matrix3F& m);
    void                    GetWorldMatrixWithScroll(Matrix* m) const;

    // Determines the absolute path of the character.
    const char*             GetAbsolutePath(String *ppath) const;

    // Once moved by script, don't accept tag moves.
    void SetAcceptAnimMovesFlag(bool v = true) { (v) ? DOFlags |= Mask_AcceptAnimMoves : DOFlags &= (~Mask_AcceptAnimMoves); }
    void ClearAcceptAnimMovesFlag()            { SetAcceptAnimMovesFlag(false); }
    bool IsAcceptAnimMovesFlagSet() const      { return (DOFlags & Mask_AcceptAnimMoves) != 0; }

    void SetContinueAnimationFlag(bool v = true)  { (v) ? DOFlags |= Mask_ContinueAnimation : DOFlags &= (~Mask_ContinueAnimation); }
    void ClearContinueAnimationFlag()             { SetContinueAnimationFlag(false); }
    bool IsContinueAnimationFlagSet() const       { return (DOFlags & Mask_ContinueAnimation) != 0; }
    virtual bool GetContinueAnimationFlag() const { return IsContinueAnimationFlagSet(); }

    // Set if the instance name was assigned dynamically.
    void SetInstanceBasedNameFlag(bool v = true) { (v) ? DOFlags |= Mask_InstanceBasedName : DOFlags &= (~Mask_InstanceBasedName); }
    void ClearInstanceBasedNameFlag()            { SetInstanceBasedNameFlag(false); }
    bool IsInstanceBasedNameFlagSet() const      { return (DOFlags & Mask_InstanceBasedName) != 0; }

    void SetUsedAsMask(bool v = true) { (v) ? DOFlags |= Mask_UsedAsMask : DOFlags &= (~Mask_UsedAsMask); }
    void ClearUsedAsMask()            { SetUsedAsMask(false); }
    bool IsUsedAsMask() const         { return (DOFlags & Mask_UsedAsMask) != 0; }

    void SetExecutionAborted()              { DOFlags |= Mask_ExecutionAborted; }
    bool IsExecutionAborted() const         { return (DOFlags & Mask_ExecutionAborted) != 0; }

    MovieImpl*        GetMovieImpl() const    { SF_ASSERT(pASRoot); return pASRoot->GetMovieImpl(); }
    ASMovieRootBase*  GetASMovieRoot() const  { return pASRoot; }

    int GetAVMVersion() const { SF_ASSERT(pASRoot); return pASRoot->GetAVMVersion(); }
    bool IsAVM1() const { return GetAVMVersion() == 1; }
    bool IsAVM2() const { return GetAVMVersion() == 2; }

    SF_INLINE CharacterHandle* GetCharacterHandle() const
    {
        if (!pNameHandle)
            return CreateCharacterHandle();
        return pNameHandle;
    }
    bool                    HasEventHandler(const EventId& id) const;

    // inherited from DisplayObjectBase methods
    virtual void            OnEventLoad();
    virtual void            OnEventUnload();
    virtual bool            OnEvent(const EventId& );

    DisplayObject*          GetMask() const;
    DisplayObject*          GetMaskOwner() const;
    void                    SetMask(DisplayObject* ch, bool readdMaskToDisplayList=true);
    void                    SetMaskOwner(DisplayObject* ch);

    // if parameter NULL, scrollRect is removed
    void                    SetScrollRect(const Render::RectD*);
    const Render::RectD*    GetScrollRect() const  
    { 
        return (pScrollRect) ? &pScrollRect->Rectangle : NULL; 
    }
    const ScrollRectInfo*   GetScrollRectInfo() const { return pScrollRect; }
    bool                    HasScrollRect() const { return pScrollRect != 0; }

    // returns mouse x coordinate, in displayobject's coordinate space, in pixels
    virtual Double          GetMouseX() const;
    // returns mouse y coordinate, in displayobject's coordinate space, in pixels
    virtual Double          GetMouseY() const;

    void                    SetName(const ASString& name);
    void                    SetOriginalName(const ASString& name);
    ASString                GetName() const;
    ASString                GetOriginalName() const;
    bool                    HasInstanceBasedName() const        { return IsInstanceBasedNameFlagSet(); }

    // Implementation for DisplayObjectBase optional data.
    virtual bool            GetAcceptAnimMoves() const          { return IsAcceptAnimMovesFlagSet(); }
    virtual void            SetAcceptAnimMoves(bool accept);

    void                    SetTimelineObjectFlag(bool v)      
    { 
        (v) ? DOFlags |= Mask_TimelineObject : DOFlags &= (~Mask_TimelineObject);
    }
    bool                    IsTimelineObjectFlagSet() const     { return (DOFlags & Mask_TimelineObject) != 0; }

    virtual void            DoDisplayCallback() {}

    // Transforms a point into local space for hit-testing, takes into account 3D objects.
    // Also, checks if the point is inside the bounds of pScrollRect: returns true if it is, false
    // otherwise.
    bool                    TransformPointToLocalAndCheckBounds
        (Render::PointF *p, const Render::PointF & pt, bool bPtInParentSpace = true, const Matrix2F *mat = NULL) const;
};

SF_INLINE
DisplayObject* DisplayObjectBase::CharToScriptableObject()
{
    return IsScriptableObject() ? static_cast<DisplayObject*>(this) : 0;
}

SF_INLINE
DisplayObject* DisplayObjectBase::CharToScriptableObject_Unsafe()
{
    return static_cast<DisplayObject*>(this);
}

SF_INLINE
const DisplayObject* DisplayObjectBase::CharToScriptableObject() const 
{
    return IsScriptableObject() ? static_cast<const DisplayObject*>(this) : 0;
}

SF_INLINE
const DisplayObject* DisplayObjectBase::CharToScriptableObject_Unsafe() const 
{
    return static_cast<const DisplayObject*>(this);
}


SF_INLINE 
const AvmDisplayObjBase* DisplayObjectBase::GetAvmObjImpl() const 
{ 
    SF_ASSERT(HasAvmObject());
    return reinterpret_cast<const AvmDisplayObjBase*>(reinterpret_cast<const UInt32*>(this) + AvmObjOffset); 
}

SF_INLINE 
AvmDisplayObjBase* DisplayObjectBase::GetAvmObjImpl()
{ 
    SF_ASSERT(HasAvmObject());
    return reinterpret_cast<AvmDisplayObjBase*>(reinterpret_cast<UInt32*>(this) + AvmObjOffset); 
}

// Needed externally by ValueImpl
void ASCharacter_MatrixScaleAndRotate2x2(Render::Matrix2F& m, float sx, float sy, float radians);

}} // namespace Scaleform::GFx

#endif
