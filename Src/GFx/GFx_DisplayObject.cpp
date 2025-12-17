/**************************************************************************

Filename    :   GFx_DisplayObject.cpp
Content     :   Implementation of DisplayObjectBase
Created     :   
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_ASUtils.h"
#include "GFx/GFx_DrawingContext.h"
#include "Render/Render_TreeLog.h"

namespace Scaleform { namespace GFx {

// ***** DisplayObjectBase   

DisplayObjectBase::DisplayObjectBase
    (ASMovieRootBase* pasRoot, InteractiveObject* parent, ResourceId id)
:
    pASRoot(pasRoot),
    Id(id),
    Depth(INVALID_DEPTH),
    CreateFrame(0),
    pParent(parent),
    pGeomData(0),
    pPerspectiveData(0),
    pIndXFormData(0),
    ClipDepth(0),
    Flags(0),
    BlendMode((UInt8)Render::Blend_None),
    AvmObjOffset(0)
{
    SF_ASSERT(pASRoot);
    SF_ASSERT((parent == NULL /*&& (Id == ResourceId::InvalidId || Id == 0)*/) ||
        (parent != NULL));
    SetVisibleFlag();
}

DisplayObjectBase::~DisplayObjectBase()
{
    if (pGeomData)
        delete pGeomData;
    if (pPerspectiveData)
        delete pPerspectiveData;
    if (pIndXFormData)
        delete pIndXFormData;
    if (HasAvmObject())
        GetAvmObjImpl()->~AvmDisplayObjBase();
}

Render::Context& DisplayObjectBase::GetRenderContext() const 
{ 
    return GetMovieImpl()->GetRenderContext(); 
}

Render::TreeNode* DisplayObjectBase::GetRenderNode() const
{
    if (!pRenNode)
    {
        pRenNode = CreateRenderNode(GetMovieImpl()->GetRenderContext());
        SF_ASSERT(pRenNode);
        pRenNode->SetVisible(IsVisibleFlagSet());
    }
    return pRenNode;
}

Render::TreeContainer* DisplayObjectBase::ConvertToTreeContainer() 
{
    if (!pRenNode) GetRenderNode();
    SF_ASSERT(pRenNode);

    Render::TreeContainer* cont = static_cast<Render::TreeContainer*>(pRenNode->GetParent());

    Ptr<DisplayObject> mask;

    // create a container....
    Ptr<Render::TreeContainer> newnode = *GetRenderContext().CreateEntry<Render::TreeContainer>();
    if (cont)
    {
        if (!pRenNode->IsMaskNode())
        {
            // Dynamic mask should not be set on the node being converted!
            // Refer to Sprite::GetDrawingContext() and ShapeObject::GetDrawingContext().
            SF_ASSERT(!IsScriptableObject() || !CharToScriptableObject_Unsafe()->GetMask());
           
            // find the index of pRenNode first
            UPInt i = 0;
            for (UPInt n = cont->GetSize(); i < n; ++i)
            {
                Render::TreeNode* nd = cont->GetAt(i);
                if (nd == pRenNode)
                    break;
            }
            SF_ASSERT(i < cont->GetSize());

            cont->Remove(i, 1);

            // reinsert a new node at appropriate position and add old node as its child....
            cont->Insert(i, newnode.GetPtr());
        }
        else
        {
            // this is a mask node. Just set newnode as a mask.
            cont->SetMaskNode(NULL);
            cont->SetMaskNode(newnode.GetPtr());
        }
    }
    // need to transfer matrix and color transform from old to new node;
    // old node should be set to Identity for both matrix and cxform.
    // Also, do this for other properties (visible, blendMode, s9g, 3d, etc)
    if (Is3D())
    {
        newnode->SetMatrix3D(GetMatrix3D());
        Matrix4F m4;
        if (GetProjectionMatrix3D(&m4))
        {
            newnode->SetProjectionMatrix3D(m4);
        }
        Matrix3F m3;
        if (GetViewMatrix3D(&m3))
        {
            newnode->SetViewMatrix3D(m3);
        }
        pRenNode->SetMatrix3D(Matrix3F::Identity);
    }
    else
    {
        newnode->SetMatrix(GetMatrix());
        pRenNode->SetMatrix(Matrix2F::Identity);
    }

    newnode->SetCxform(GetCxform());
    pRenNode->SetCxform(Cxform::Identity);

    newnode->SetVisible(GetVisible());
    pRenNode->SetVisible(true);

    newnode->SetBlendMode(pRenNode->GetBlendMode());
    pRenNode->SetBlendMode(Render::Blend_None);

    Render::RectF s9g = pRenNode->GetScale9Grid();
    if (!s9g.IsEmpty())
    {
        newnode->SetScale9Grid(s9g);
        pRenNode->SetScale9Grid(RectF(0));
    }

    newnode->Add(pRenNode);

    pRenNode = newnode;

    return newnode;
}

const Matrix& DisplayObjectBase::GetMatrix() const                   
{ 
    if (!pIndXFormData)
    {
        if (pRenNode)
            return pRenNode->M2D();
    }
    else
    {
        return *(Matrix2F *)(void*)&pIndXFormData->OrigTransformMatrix;
    }
    return Matrix::Identity; 
}

void DisplayObjectBase::SetMatrix(const Matrix& m)          
{ 
    SF_ASSERT(m.IsValid()); 
    if (!pIndXFormData)
        GetRenderNode()->SetMatrix(m);
    else
    {
        pIndXFormData->OrigTransformMatrix = m;
        
        // mark node for renderer as 'changed'
        GetRenderNode()->GetWritableData(Render::Change_Matrix);
    }
    DisplayObjectBase::UpdateViewAndPerspective();  // don't need to update children
}

const Matrix3F& DisplayObjectBase::GetMatrix3D() const                   
{ 
    if (!pIndXFormData)
    {
        if (pRenNode)       // check Is3D() first?
            return pRenNode->M3D();
    }
    else
    {
        return pIndXFormData->OrigTransformMatrix;
    }

    return Matrix3F::Identity; 
}

void DisplayObjectBase::SetMatrix3D(const Matrix3F& m)          
{ 
    SF_ASSERT(m.IsValid()); 
    if (!pIndXFormData)
        GetRenderNode()->SetMatrix3D(m);
    else
    {
        pIndXFormData->OrigTransformMatrix = m;
        pIndXFormData->IsOrig3D = true;

        // mark node for renderer as 'changed'
        GetRenderNode()->GetWritableData(Render::Change_Matrix);
    }
    DisplayObjectBase::UpdateViewAndPerspective();  // don't need to update children
}

bool DisplayObjectBase::GetViewMatrix3D(Matrix3F *m, bool bInherit) const
{ 
    if (pRenNode && pRenNode->HasViewMatrix3D())
    {
        SF_ASSERT(pPerspectiveData);
        *m = pPerspectiveData->ViewMatrix3D;    // return view matrix (without viewport adjustment)
        return true;
    }

    if (!bInherit)
        return false;
    
    if (!pParent)
    {
        MovieImpl * pmovie = GetMovieImpl();
        if (pmovie)
            return pmovie->GetRenderRoot()->GetViewMatrix3D(m);

        SF_ASSERT(false);
        return false;
    }

    return pParent->GetViewMatrix3D(m, bInherit);
}

// strange rule/behavior where perspective on an object is ignored in Adobe player 
// if an object has a 3D parent without perspective.
bool IgnoreProjection(DisplayObjectBase* pdispObj)
{
    return (pdispObj->GetParent() && pdispObj->GetParent()->Is3D() && !pdispObj->GetParent()->GetPerspectiveDataPtr());
}

void DisplayObjectBase::SetViewMatrix3D(const Matrix3F& m)          
{ 
    SF_ASSERT(m.IsValid()); 

    // adobe strange exception case - MOOSE
    bool bHasViewMat = (pRenNode && pRenNode->HasViewMatrix3D());
    if (IgnoreProjection(this) && !bHasViewMat)
        return;

    if (!pPerspectiveData)
        pPerspectiveData = SF_HEAP_AUTO_NEW(this) PerspectiveDataType;

    pPerspectiveData->ViewMatrix3D = m;     // save view matrix (without viewport adjustment), for Flash hittest

    // account for 2D viewport mat set at the root
    MovieImpl * pmovie = GetMovieImpl();
    Matrix3F viewportInv(Matrix3F::Translation(pmovie->VisibleFrameRect.x1, pmovie->VisibleFrameRect.y1, 0), 
        Matrix3F::Scaling(1.0f / (pmovie->mViewport.Width / pmovie->VisibleFrameRect.Width()),
        1.f / (pmovie->mViewport.Height / pmovie->VisibleFrameRect.Height()), 1.f));
    Matrix3F view(m, viewportInv);

    // store view matrix with viewport adjustment added in
    GetRenderNode()->SetViewMatrix3D(view);

    // if we are at the root or stage, then set it at the render and topmost roots also
    if (pmovie && pmovie->MovieLevels[0].pSprite == this)
    {
        pmovie->GetRenderRoot()->SetViewMatrix3D(view);
        pmovie->pTopMostRoot->SetViewMatrix3D(view);
    }
}

void DisplayObjectBase::ClearViewMatrix3D()
{
    GetRenderNode()->ClearViewMatrix3D();
}

bool DisplayObjectBase::GetProjectionMatrix3D(Matrix4F *m, bool bInherit) const
{ 
    if (pRenNode && pRenNode->GetProjectionMatrix3D(m))
        return true;

    if (!bInherit)
        return false;

    if (!pParent)
    {
        MovieImpl * pmovie = GetMovieImpl();
        if (pmovie)
            return pmovie->GetRenderRoot()->GetProjectionMatrix3D(m);

        SF_ASSERT(false);
        return false;
    }

    return pParent->GetProjectionMatrix3D(m, bInherit);
}

void DisplayObjectBase::SetProjectionMatrix3D(const Matrix4F& m)          
{ 
    SF_ASSERT(m.IsValid()); 

    // adobe strange exception case - MOOSE
    bool bHasProjMat = (pRenNode && pRenNode->HasProjectionMatrix3D());
    if (IgnoreProjection(this) && !bHasProjMat)
        return;

    GetRenderNode()->SetProjectionMatrix3D(m);

    // if we are at the root or stage, then set it at the render and topmost root also
    MovieImpl *pmovie = GetMovieImpl();
    if (pmovie && pmovie->MovieLevels[0].pSprite == this)
    {
        pmovie->GetRenderRoot()->SetProjectionMatrix3D(m);
        pmovie->pTopMostRoot->SetProjectionMatrix3D(m);
    }
}

void DisplayObjectBase::ClearProjectionMatrix3D()
{
    GetRenderNode()->ClearProjectionMatrix3D();
}

void DisplayObjectBase::UpdateViewAndPerspective()
{
    MovieImpl * pMovie = GetMovieImpl();
    if (pMovie && pPerspectiveData && pPerspectiveData->UpdateViewAndPerspectiveFromData())
    {
        const RectF &vfr = pMovie->GetVisibleFrameRectInTwips();
        if (!vfr.IsNull())
        {
            Matrix3F view;
            Matrix4F proj;
            
            // compute center of projection if necessary
            PointF Center;    
            if (!NumberUtil::IsNaN(pPerspectiveData->ProjectionCenter.x) && !NumberUtil::IsNaN(pPerspectiveData->ProjectionCenter.y) )
            {
                Center = pPerspectiveData->ProjectionCenter;

                // NOTE: the ProjectionCenter is only offset by the object's position if it is a 2D object, or its 
                // parent has a projectionCenter. Otherwise, (eg. it is a 3D object) it is relative to the stage.                
                bool is3D = GetRenderNode()->Is3D();
                bool parentPC = false;
                if (GetParent())
                {
                    const PerspectiveDataType* parentPD = GetParent()->GetPerspectiveDataPtr();
                    if (parentPD)
                    {
                        parentPC = !NumberUtil::IsNaN(parentPD->ProjectionCenter.x) && 
                            !NumberUtil::IsNaN(parentPD->ProjectionCenter.y);
                    }
                }

                if (pGeomData && (!is3D || parentPC))
                {
                    // Transform the ProjectionCenter by the object's world matrix, because
					// it is apparently specified in world space, not in local space.
                    const Matrix& world = GetWorldMatrix();                
                    Center = world * Center;
                }
            }
            else
            {
                Center.x = (vfr.x1 + vfr.x2)/2.f;
                Center.y = (vfr.y1 + vfr.y2)/2.f;
            }

            float fieldOfView = pPerspectiveData->FieldOfView ? (float)pPerspectiveData->FieldOfView : DEFAULT_FLASH_FOV;
            float focalLength = pPerspectiveData->FocalLength ? (float)pPerspectiveData->FocalLength : 0;

            MovieImpl::MakeViewAndPersp3D(&view, &proj, pMovie->VisibleFrameRect, Center, fieldOfView, focalLength);
            SetViewMatrix3D(view);
            SetProjectionMatrix3D(proj);
        }
    }
}

// set perspective field of view
void DisplayObjectBase::SetFOV(Double fovdeg)
{
    if (NumberUtil::IsNaN(fovdeg))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(fovdeg) || NumberUtil::IsPOSITIVE_INFINITY(fovdeg))
        fovdeg = 0;

    if (!pPerspectiveData)
        pPerspectiveData = SF_HEAP_AUTO_NEW(this) PerspectiveDataType;

    if (pPerspectiveData->FieldOfView != fovdeg)
    {
        pPerspectiveData->FieldOfView = fovdeg;
        DisplayObjectBase::UpdateViewAndPerspective();  // don't need to update children
    }
}

/// get perspective field of view
Double DisplayObjectBase::GetFOV() const
{
    return pPerspectiveData ? pPerspectiveData->FieldOfView : Double(0.0);
}

// set perspective focal length (in TWIPS)
void DisplayObjectBase::SetFocalLength(Double focalLength)
{
    if (NumberUtil::IsNaN(focalLength))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(focalLength) || NumberUtil::IsPOSITIVE_INFINITY(focalLength))
        focalLength = 0;

    if (!pPerspectiveData)
        pPerspectiveData = SF_HEAP_AUTO_NEW(this) PerspectiveDataType;

    if (pPerspectiveData->FocalLength != focalLength)
    {
        pPerspectiveData->FocalLength = focalLength;
        DisplayObjectBase::UpdateViewAndPerspective();  // don't need to update children
    }
}

// get perspective focal length (in TWIPS)
Double DisplayObjectBase::GetFocalLength() const
{
    return pPerspectiveData ? pPerspectiveData->FocalLength : Double(0.0);
}

// set perspective center of projection (in TWIPS)
void DisplayObjectBase::SetProjectionCenter(PointF projCenter)
{
    if (NumberUtil::IsNaN(projCenter.x) || NumberUtil::IsNaN(projCenter.y))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(projCenter.x) || NumberUtil::IsPOSITIVE_INFINITY(projCenter.x))
        projCenter.x = 0;
    if (NumberUtil::IsNEGATIVE_INFINITY(projCenter.y) || NumberUtil::IsPOSITIVE_INFINITY(projCenter.y))
        projCenter.y = 0;

    if (!pPerspectiveData)
        pPerspectiveData = SF_HEAP_AUTO_NEW(this) PerspectiveDataType;

    if (pPerspectiveData->ProjectionCenter != projCenter)
    {
        pPerspectiveData->ProjectionCenter = projCenter;
        DisplayObjectBase::UpdateViewAndPerspective();  // don't need to update children
    }
}

// get perspective center of projection (in TWIPS)
PointF DisplayObjectBase::GetProjectionCenter() const
{
    return pPerspectiveData ? pPerspectiveData->ProjectionCenter : PointF(0,0);
}

const Cxform& DisplayObjectBase::GetCxform() const                   
{ 
    if (pRenNode)
        return pRenNode->GetCxform();
    return Cxform::Identity; 
}

void DisplayObjectBase::SetCxform(const Cxform& cx)         
{ 
    GetRenderNode()->SetCxform(cx);
}

void DisplayObjectBase::ConcatenateCxform(const Cxform& cx) 
{ 
    Cxform cx1(GetCxform());
    cx1.Prepend(cx); 
    SetCxform(cx1);
}

void DisplayObjectBase::ConcatenateMatrix(const Matrix& m)  
{ 
    if (Is3D())
    {
        Matrix3F m1(GetMatrix3D());
        m1.Prepend(m);
        SetMatrix3D(m1); 
    }
    else
    {
        Matrix m1(GetMatrix());
        m1.Prepend(m);
        SetMatrix(m1); 
    }
}

void DisplayObjectBase::SetBlendMode(BlendType blend)
{ 
    // "Normal" and "None" blend modes are treated the same in flash (they defer to their
    // parent's behavior). However, the render can differentiate them. Flash has no ability
    // to have a node with a modified blend mode, and have a child with "Normal" blending.
    GetRenderNode()->SetBlendMode(blend == Blend_Normal ? Blend_None : blend);
	BlendMode = (UInt8)blend; 
}

DisplayObjectBase::BlendType DisplayObjectBase::GetBlendMode() const
{
	if (GetRenderNode())
		return GetRenderNode()->GetBlendMode();
	return (BlendType)BlendMode;
}

bool DisplayObjectBase::GetCacheAsBitmap() const
{
    if (GetRenderNode())
    {
        const FilterSet* filters = GetRenderNode()->GetFilters();
        if (!filters)
            return false;
        return filters->GetCacheAsBitmap();
    }
    return false;
}

void DisplayObjectBase::SetCacheAsBitmap(bool enable)
{
    if (GetRenderNode())
    {
        const FilterSet* filters = GetRenderNode()->GetFilters();
        if (!filters && enable)
        {
            Ptr<FilterSet> tempSet = *SF_NEW FilterSet();
            tempSet->SetCacheAsBitmap(true);
            SetFilters(tempSet);
        }
        else if (filters && enable != filters->GetCacheAsBitmap())
        {
            Ptr<FilterSet> newSet = *filters->Clone(false);
            newSet->SetCacheAsBitmap(enable);
            SetFilters(newSet);
        }
    }
}

void DisplayObjectBase::SetFilters(const Render::FilterSet* filters)
{
#ifdef SF_RENDER_ENABLE_FILTERS
    if ( GetRenderNode() )
        GetRenderNode()->SetFilters(filters);
#else
    SF_UNUSED(filters);
#endif
}

const Render::FilterSet* DisplayObjectBase::GetFilters() const
{
#ifdef SF_RENDER_ENABLE_FILTERS
    if ( GetRenderNode() )
        return GetRenderNode()->GetFilters();
#endif
    return 0;
}

void DisplayObjectBase::SetVisibleFlag(bool v) 
{ 
    if (v)
    {
        Flags |= Mask_Visible;
        if (pRenNode)
            pRenNode->SetVisible(v);
    }
    else
    {
        Flags &= (~Mask_Visible);
        GetRenderNode()->SetVisible(false);
    }
}

void DisplayObjectBase::BindAvmObj(AvmDisplayObjBase* p) 
{ 
    SF_ASSERT(AvmObjOffset == 0);
    SF_ASSERT((size_t)((UByte*)p - (UByte*)this) < (size_t)1024); 
    SF_ASSERT((((size_t)p) & 0x3) == 0); // 4-bytes aligned 

    AvmObjOffset = UInt8(((size_t)((UByte*)p - (UByte*)this) + 3)/4);
}


// Character transform implementation.
void DisplayObjectBase::GetWorldMatrix(Matrix *pmat) const
{           
    if (pParent)
    {
        pParent->GetWorldMatrix(pmat);
        pmat->Prepend(GetMatrix());
    }
    else
    {
        *pmat = GetMatrix();
    }
}
 
void DisplayObjectBase::GetWorldMatrix3D(Matrix3F *pmat) const
{
    if (pParent)
    {
        pParent->GetWorldMatrix3D(pmat);
        pmat->Prepend(GetMatrix3D());
    }
    else
    {
        *pmat = GetMatrix3D();
    }
}

void DisplayObjectBase::Clear3D(bool bInherit)                           
{ 
    if (bInherit)
        if (pParent)
            pParent->Clear3D(bInherit);

    // clear geom data
    GeomDataType geomData;
    SetGeomData(geomData);

    if (pRenNode) 
        pRenNode->Clear3D(); 

    UpdateViewAndPerspective(); // Need to update children.
}

bool DisplayObjectBase::Is3D(bool bInherit) const     
{
    bool b3D = Is3D();
    if (bInherit)
    {
        if (b3D)
            return true;

        if (pParent)
            return pParent->Is3D(bInherit);
    }

    return b3D;
}

// Character transform implementation.
void DisplayObjectBase::GetLevelMatrix(Matrix *pmat) const
{           
    if (pParent)
    {
        pParent->GetLevelMatrix(pmat);
        pmat->Prepend(GetMatrix());
    }
    else
    {
        *pmat = Matrix();
    }
}

// Character color transform implementation.
void DisplayObjectBase::GetWorldCxform(Cxform *pcxform) const
{
    if (pParent)
    {
        pParent->GetWorldCxform(pcxform);
        pcxform->Prepend(GetCxform());
    }
    else
    {
        *pcxform = GetCxform();     
    }
}

// Converts a 3D point of the three-dimensional display object's (local) TWIPS coordinates to 
// a 2D point in the Stage (global) TWIPS coordinates. Same as AS DisplayObject:localToGlobal3D.
PointF DisplayObjectBase::Local3DToGlobal(const Render::Point3F &ptIn) const
{
    if (Is3D(true))
    {
        Matrix3F view;
        GetViewMatrix3D(&view, true);

        Matrix4F proj;
        GetProjectionMatrix3D(&proj, true);

        Matrix3F world;
        GetWorldMatrix3D(&world);

        Matrix4F mvp = proj * view * world;

        Point3F p1 = mvp.TransformHomogeneous(ptIn);

        SF_ASSERT(pASRoot);
        Viewport vp;
        pASRoot->GetMovieImpl()->GetViewport(&vp);

        // convert to window coords
        p1.x = vp.scaleX(vp.Left, vp.Width,  p1.x);
        p1.y = vp.scaleY(vp.Top,  vp.Height, p1.y);

        // map from window to stage TWIPS
        const MovieImpl *pmovie = pASRoot ? pASRoot->GetMovieImpl() : NULL;
        SF_ASSERT(pmovie);
        if (pmovie)
        {
            p1.x = PixelsToTwips(pmovie->ViewOffsetX + (p1.x * pmovie->ViewScaleX));
            p1.y = PixelsToTwips(pmovie->ViewOffsetY + (p1.y * pmovie->ViewScaleY));
        }
        return PointF(p1.x, p1.y);
    }
    else
    {
        return GetWorldMatrix().Transform(PointF(ptIn.x, ptIn.y));
    }
}

Render::PointF DisplayObjectBase::GlobalToLocal(const Render::PointF &ptIn) const		
{ 
    Render::PointF ptOut; 

    MovieImpl *proot = GetMovieImpl();
    if (proot)
    {
        // convert ptIn from stage to window
        float wx = (ptIn.x - proot->ViewOffsetX) / proot->ViewScaleX;
        float wy = (ptIn.y - proot->ViewOffsetY) / proot->ViewScaleY;

        proot->SetNormalizedScreenCoords(Render::PointF(wx, wy));

        TransformPointToLocal(&ptOut, ptIn, false); 
    }    
    
    return ptOut; 
}

Render::Point3F DisplayObjectBase::GlobalToLocal3D(const Render::PointF &ptIn) const	
{ 
    Render::PointF ptOut = GlobalToLocal(ptIn); 
    return Render::Point3F(ptOut.x, ptOut.y, 0); 
}

// Transform point (in TWIPS) to object's local space (in TWIPS), handles both 2D and 3D
void DisplayObjectBase::TransformPointToLocal(Render::PointF *p, const Render::PointF & pt, bool bPtInParentSpace, 
                                              const Matrix2F *mat) const
{
    // if bPtInParentSpace then we are being called recursively so don't need Is3D(inherit) 
    bool b3D = bPtInParentSpace ? Is3D() : Is3D(true /* check ancestors */);
    if ( !b3D )
    {
        if (bPtInParentSpace)
        {
            if (mat)
                mat->TransformByInverse(p, pt);
            else
                GetMatrix().TransformByInverse(p, pt);
        }
        else
        {
            Matrix2F worldMat;
            GetWorldMatrix(&worldMat);
            worldMat.TransformByInverse(p, pt);
        }
    }
    else
    {
        Matrix3F view;
        Matrix4F proj;
        Matrix3F world = GetWorldMatrix3D();
        Render::ScreenToWorld & stw = GetMovieImpl()->ScreenToWorld;
        
        if (GetProjectionMatrix3D(&proj, true /* inherit */))
            stw.SetProjection(proj);

        if (GetViewMatrix3D(&view, true /* inherit */))
            stw.SetView(view);

        stw.SetWorld(world);
        stw.GetWorldPoint(p);       
    }
}

// Temporary - used until blending logic is improved.
Render::BlendMode DisplayObjectBase::GetActiveBlendMode() const
{
    Render::BlendMode blend = Render::Blend_None;
    const DisplayObjectBase*    pchar = this;

    while (pchar)
    {
        blend = pchar->GetBlendMode();
        if (blend > Render::Blend_Layer)
            return blend;
        pchar = pchar->GetParent();
    }
    // Return last blend mode.
    return blend;
}


// This is not because of MovieDefImpl dependency not available in the header.
unsigned DisplayObjectBase::GetVersion() const
{
    return GetResourceMovieDef()->GetVersion();
}

// (needs to be implemented in .cpp, so that MovieImpl is visible)
Log*     DisplayObjectBase::GetLog() const
{
    // By default, GetMovieImpl will delegate to parent.
    // GFxSprite should override GetMovieImpl to return the correct object.
    return FindMovieImpl()->GetCachedLog();
}
bool        DisplayObjectBase::IsVerboseAction() const
{
    return FindMovieImpl()->IsVerboseAction();
}

bool        DisplayObjectBase::IsVerboseActionErrors() const
{
    return !FindMovieImpl()->IsSuppressActionErrors();
}

void DisplayObjectBase::OnEventUnload()
{
    SetUnloading();
    // should it be before Event_Unload or after? (AB)
    if (IsTopmostLevelFlagSet())
    {
        FindMovieImpl()->RemoveTopmostLevelCharacter(this);
    }
    if (!IsUnloaded())
    {
        OnEvent(EventId::Event_Unload);
        SetUnloaded();
    }
}

bool DisplayObjectBase::OnUnloading()
{
    return true;
}

bool DisplayObjectBase::CheckLastHitResult(float x, float y) const
{
    return 
        (Flags & Mask_HitTest) != 0 &&
        x == LastHitTestX && y == LastHitTestY;
}

void DisplayObjectBase::SetLastHitResult(float x, float y, bool result) const // TO DO: Revise "const"
{
    LastHitTestX = x;
    LastHitTestY = y;
    Flags &= ~Mask_HitTest;
    Flags |= result ? Mask_HitTestPositive : Mask_HitTestNegative;
}

void DisplayObjectBase::InvalidateHitResult() const                           // TO DO: Revise "const"
{
    Flags &= ~Mask_HitTest;
}

// MovieImpl*   DisplayObjectBase::GetMovieImpl() const                
// { 
//     return (pParentChar) ? pParentChar->GetMovieImpl() : NULL; 
// }

DisplayObjectBase::GeomDataType& DisplayObjectBase::GetGeomData(GeomDataType& geomData) const
{
    if (!pGeomData)
    {
        // fill GeomData using Matrix_1
        const Matrix& m = GetMatrix();
        geomData.X = int(m.Tx());
        geomData.Y = int(m.Ty());
        geomData.XScale = m.GetXScaleDouble()*(Double)100.;
        geomData.YScale = m.GetYScaleDouble()*(Double)100.;
        geomData.Rotation = (m.GetRotationDouble()*(Double)180.)/SF_MATH_PI;
        geomData.OrigMatrix = GetMatrix();
    }
    else
    {
        geomData = *pGeomData;
    }
    return geomData;
}

void    DisplayObjectBase::SetGeomData(const GeomDataType& gd)
{
    if (pGeomData)
        *pGeomData = gd;
    else 
        pGeomData = SF_HEAP_AUTO_NEW(this) GeomDataType(gd);
}

void    DisplayObjectBase::EnsureGeomDataCreated()
{
    if (!pGeomData)
    {
        GeomDataType geomData;
        SetGeomData(GetGeomData(geomData));
    }
}

void ASCharacter_MatrixScaleAndRotate2x2(Render::Matrix2F& m, float sx, float sy, float radians)
{
    float   cosAngle = cosf(radians);
    float   sinAngle = sinf(radians);
    float   x00 = m.Sx();
    float   x01 = m.Shx();
    float   x10 = m.Shy();
    float   x11 = m.Sy();

    m.Sx() = (x00*cosAngle-x10*sinAngle)*sx;
    m.Shx() = (x01*cosAngle-x11*sinAngle)*sy;
    m.Shy() = (x00*sinAngle+x10*cosAngle)*sx;
    m.Sy() = (x01*sinAngle+x11*cosAngle)*sy;
}

void DisplayObjectBase::SetX(Double x)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(x))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(x) || NumberUtil::IsPOSITIVE_INFINITY(x))
        x = 0;
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Matrix  m = GetMatrix();
    pGeomData->X = int(floor(PixelsToTwips(x)));
    m.Tx() = (float) pGeomData->X;
    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetY(Double y)
{
    if (NumberUtil::IsNaN(y))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(y) || NumberUtil::IsPOSITIVE_INFINITY(y))
        y = 0;
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Matrix  m = GetMatrix();
    pGeomData->Y = int(floor(PixelsToTwips(y)));
    m.Ty() = (float) pGeomData->Y;
    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetZ(Double z)
{
    if (NumberUtil::IsNaN(z))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(z) || NumberUtil::IsPOSITIVE_INFINITY(z))
        z = 0;

    if (pASRoot && pASRoot->GetMovieImpl()->AcceptAnimMovesWith3D())
        EnsureGeomDataCreated();
    else
        SetAcceptAnimMoves(0);      // default for AS3 is this, to not allow timeline

    SF_ASSERT(pGeomData);

    pGeomData->Z = z;
    UpdateTransform3D();
}

void DisplayObjectBase::SetXScale(Double xscale)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(xscale) ||
        NumberUtil::IsNEGATIVE_INFINITY(xscale) || NumberUtil::IsPOSITIVE_INFINITY(xscale))
    {
        return;
    }
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    pGeomData->XScale = xscale;
    if (Is3D())
        UpdateTransform3D();
    else
    {
        const Matrix& chm = GetMatrix();
        Matrix m = pGeomData->OrigMatrix;
        m.Tx() = chm.Tx();
        m.Ty() = chm.Ty();

        Double origXScale = m.GetXScaleDouble();
        if (origXScale == 0 || xscale > 1E+16)
        {
            xscale = 0;
            origXScale = 1;
        }

        ASCharacter_MatrixScaleAndRotate2x2(m,
            float(xscale/(origXScale*100.)), 
            float(pGeomData->YScale/(m.GetYScaleDouble()*100.)),
            float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

        if (m.IsValid())
            SetMatrix(m);
    }
}

void DisplayObjectBase::SetYScale(Double yscale)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(yscale) ||
        NumberUtil::IsNEGATIVE_INFINITY(yscale) || NumberUtil::IsPOSITIVE_INFINITY(yscale))
    {
        return;
    }
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    pGeomData->YScale = yscale;
    if (Is3D())
        UpdateTransform3D();
    else
    {
        const Matrix& chm = GetMatrix();
        Matrix m = pGeomData->OrigMatrix;
        m.Tx() = chm.Tx();
        m.Ty() = chm.Ty();

        Double origYScale = m.GetYScaleDouble();
        if (origYScale == 0 || yscale > 1E+16)
        {
            yscale = 0;
            origYScale = 1;
        }

        ASCharacter_MatrixScaleAndRotate2x2(m,
            float(pGeomData->XScale/(m.GetXScaleDouble()*100.)), 
            float(yscale/(origYScale* 100.)),
            float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

        if (m.IsValid())
            SetMatrix(m);
    }
}

void DisplayObjectBase::SetZScale(Double zscale)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(zscale) ||
        NumberUtil::IsNEGATIVE_INFINITY(zscale) || NumberUtil::IsPOSITIVE_INFINITY(zscale))
    {
        return;
    }

    if (pASRoot && pASRoot->GetMovieImpl()->AcceptAnimMovesWith3D())
        EnsureGeomDataCreated();
    else
        SetAcceptAnimMoves(0);      // default for AS3 is this, to not allow timeline

    SF_ASSERT(pGeomData);

    pGeomData->ZScale = zscale;
    UpdateTransform3D();
}

void DisplayObjectBase::SetRotation(Double rotation)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(rotation))
        return;
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Double r = fmod((Double)rotation, (Double)360.);
    if (r > 180)
        r -= 360;
    else if (r < -180)
        r += 360;
    if (pASRoot->GetAVMVersion() == 1) // AS 1.0 & 2.0
        pGeomData->Rotation = r;
    else
        pGeomData->Rotation = rotation;

    if (Is3D())
        UpdateTransform3D();
    else
    {
        const Matrix& chm = GetMatrix();
        Matrix m = pGeomData->OrigMatrix;
        m.Tx() = chm.Tx();
        m.Ty() = chm.Ty();

        Double origRotation = m.GetRotationDouble();

        // remove old rotation by rotate back and add new one

        ASCharacter_MatrixScaleAndRotate2x2(m,
            float(pGeomData->XScale/(m.GetXScaleDouble()*100.)), 
            float(pGeomData->YScale/(m.GetYScaleDouble()*100.)),
            float(-origRotation + r * SF_MATH_PI / 180.));

        if (m.IsValid())
            SetMatrix(m);
    }
}

void DisplayObjectBase::SetXRotation(Double rotation)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(rotation))
        return;
    
    if (pASRoot && pASRoot->GetMovieImpl()->AcceptAnimMovesWith3D())
        EnsureGeomDataCreated();
    else
        SetAcceptAnimMoves(0);      // default for AS3 is this, to not allow timeline
    
    SF_ASSERT(pGeomData);

    if (pASRoot->GetAVMVersion() == 1) // AS 1.0 & 2.0
    {
        Double r = fmod((Double)rotation, (Double)360.);
        if (r > 180)
            r -= 360;
        else if (r < -180)
            r += 360;
        pGeomData->XRotation = r;
    }
    else
        pGeomData->XRotation = rotation;
    UpdateTransform3D();
}

void DisplayObjectBase::SetYRotation(Double rotation)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(rotation))
        return;
    
    if (pASRoot && pASRoot->GetMovieImpl()->AcceptAnimMovesWith3D())
        EnsureGeomDataCreated();
    else
        SetAcceptAnimMoves(0);      // default for AS3 is this, to not allow timeline
    
    SF_ASSERT(pGeomData);

    if (pASRoot->GetAVMVersion() == 1) // AS 1.0 & 2.0
    {
        Double r = fmod((Double)rotation, (Double)360.);
        if (r > 180)
            r -= 360;
        else if (r < -180)
            r += 360;
        pGeomData->YRotation = r;
    }
    else
        pGeomData->YRotation = rotation;
    UpdateTransform3D();
}

void DisplayObjectBase::SetWidth(Double width)
{
    // MA: Width/Height modification in Flash is unusual in that it performs
    // relative axis scaling in the x local axis (width scaling) and y local
    // axis (height scaling). The resulting width after scaling does not
    // actually equal the original, instead, it is related to rotation!
    // AB: I am second on that! Looks like a bug in Flash.

    // NOTE: Although it works for many cases, this is still not correct. Modification 
    // of width seems very strange (if not buggy) in Flash.
    if (NumberUtil::IsNaN(width) || NumberUtil::IsNEGATIVE_INFINITY(width))
        return;
    if (NumberUtil::IsPOSITIVE_INFINITY(width))
        width = 0;

    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Matrix m = pGeomData->OrigMatrix;
    const Matrix& chm = GetMatrix();
    m.Tx() = chm.Tx();
    m.Ty() = chm.Ty();

    Matrix im = m;
    im.AppendRotation(float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    float oldWidth      = GetBounds(im).Width(); // width should be in local coords!
    float newWidth      = float(PixelsToTwips(width));
    float multiplier    = (fabsf(oldWidth) > 1e-6f) ? (newWidth / oldWidth) : 0.0f;

    Double origXScale = m.GetXScaleDouble();
    Double newXScale = origXScale * multiplier * 100;
    pGeomData->XScale = newXScale;
    if (origXScale == 0)
    {
        newXScale = 0;
        origXScale = 1;
    }

    ASCharacter_MatrixScaleAndRotate2x2(m,
        float(Alg::Abs(newXScale/(origXScale*100.))), 
        float(Alg::Abs(pGeomData->YScale/(m.GetYScaleDouble()*100.))),
        float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    pGeomData->XScale = Alg::Abs(pGeomData->XScale);
    pGeomData->YScale = Alg::Abs(pGeomData->YScale);

    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetHeight(Double height)
{
    // MA: Width/Height modification in Flash is unusual in that it performs
    // relative axis scaling in the x local axis (width scaling) and y local
    // axis (height scaling). The resulting width after scaling does not
    // actually equal the original, instead, it is related to rotation!
    // AB: I am second on that! Looks like a bug in Flash.

    // NOTE: Although it works for many cases, this is still not correct. Modification 
    // of height seems very strange (if not buggy) in Flash.

    if (NumberUtil::IsNaN(height) || NumberUtil::IsNEGATIVE_INFINITY(height))
        return;
    if (NumberUtil::IsPOSITIVE_INFINITY(height))
        height = 0;

    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Matrix m = pGeomData->OrigMatrix;
    const Matrix& chm = GetMatrix();
    m.Tx() = chm.Tx();
    m.Ty() = chm.Ty();

    Matrix im = m;
    im.AppendRotation(float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    float oldHeight     = GetBounds(im).Height(); // height should be in local coords!
    float newHeight     = float(PixelsToTwips(height));
    float multiplier    = (fabsf(oldHeight) > 1e-6f) ? (newHeight / oldHeight) : 0.0f;

    Double origYScale = m.GetYScaleDouble();
    Double newYScale = origYScale * multiplier * 100;;
    pGeomData->YScale = newYScale;
    if (origYScale == 0)
    {
        newYScale = 0;
        origYScale = 1;
    }

    ASCharacter_MatrixScaleAndRotate2x2(m,
        float(Alg::Abs(pGeomData->XScale/(m.GetXScaleDouble()*100.))), 
        float(Alg::Abs(newYScale/(origYScale* 100.))),
        float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    pGeomData->XScale = Alg::Abs(pGeomData->XScale);
    pGeomData->YScale = Alg::Abs(pGeomData->YScale);

    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetAlpha(Double alpha)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(alpha))
        return;

    // Set alpha modulate, in percent.
    Cxform  cx = GetCxform();
    cx.M[0][3] = float(alpha / 100.);
    SetCxform(cx);
    SetAcceptAnimMoves(0);
}

Double DisplayObjectBase::GetX() const
{
    GeomDataType geomData;
    return TwipsToPixels(Double(GetGeomData(geomData).X));
}

Double DisplayObjectBase::GetY() const
{
    GeomDataType geomData;
    return TwipsToPixels(Double(GetGeomData(geomData).Y));
}

Double DisplayObjectBase::GetZ() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).Z);
}

Double DisplayObjectBase::GetXScale() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).XScale);
}

Double DisplayObjectBase::GetYScale() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).YScale);
}

Double DisplayObjectBase::GetZScale() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).ZScale);
}

Double DisplayObjectBase::GetRotation() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).Rotation);
}

Double DisplayObjectBase::GetXRotation() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).XRotation);
}

Double DisplayObjectBase::GetYRotation() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).YRotation);
}

Double DisplayObjectBase::GetWidth() const
{
    //!AB: width and height of nested movieclips returned in the coordinate space of its parent!
    RectF  boundRect;
    if (Is3D(false))
        boundRect = GetBoundsIn3D();
    else
        boundRect = GetBounds(GetMatrix());
    return TwipsToPixels(floor((Double)boundRect.Width() + 0.5));
}

Double DisplayObjectBase::GetHeight() const
{
    //!AB: width and height of nested movieclips returned in the coordinate space of its parent!
    RectF  boundRect;
    if (Is3D(false))
        boundRect = GetBoundsIn3D();
    else
        boundRect = GetBounds(GetMatrix());
    return TwipsToPixels(floor((Double)boundRect.Height() + 0.5));
}

RectF DisplayObjectBase::GetBoundsIn3D() const   
{ 
    // !AB: I am not sure this is the exactly correct way to calculate
    // bounds when a dispobj has 3D, but seems that what Flash does.
    // Refer to test files at Test/AS3/DisplayObj/:
    // test_width_height_in_3D_1.swf 
    // test_width_height_in_3D_2.swf 
    // test_width_height_in_3D_3.swf 
    // test_width_height_in_3D_4.swf 
    // test_width_height_in_3D_5.swf 
    RectF r = GetBounds(Matrix::Identity);
    PointF p1 = LocalToGlobal(r.TopLeft());
    PointF p2 = LocalToGlobal(r.BottomRight());

    if (GetParent())
    {
        p1 = GetParent()->GlobalToLocal(p1);
        p2 = GetParent()->GlobalToLocal(p2);
    }

    return RectF(p1, SizeF(p2.x - p1.x, p2.y - p1.y));//t.EncloseTransform(GetBounds(Matrix::Identity));
}

Double DisplayObjectBase::GetAlpha() const
{
    return GetCxform().M[0][3] * 100.F;
}

MovieImpl* DisplayObjectBase::FindMovieImpl() const
{
    const DisplayObjectBase* dobj = this;
    while (dobj && !dobj->IsInteractiveObject())
    {
        dobj = dobj->GetParent();
    }
    if (dobj)
        return dobj->CharToInteractiveObject_Unsafe()->GetMovieImpl();
    return NULL;
}

InteractiveObject*  DisplayObjectBase::GetTopMostMouseEntityDef
(CharacterDef* pdef, const Render::PointF &pt, bool testAll, const InteractiveObject* ignoreMC)
{   
    if (!GetVisible())
        return 0;

    Render::PointF p;          
    TransformPointToLocal(&p, pt);

    if ((ClipDepth == 0) && pdef->DefPointTestLocal(p, 1, this))
    {
        // The mouse is inside the shape.
        InteractiveObject* pCurrentParent = GetParent();
        while (pCurrentParent && pCurrentParent->IsSprite())
        {
            Sprite* spr = pCurrentParent->CharToSprite_Unsafe();
            // Parent sprite would have to be in button mode to grab attention.
            if (testAll || pCurrentParent->ActsAsButton()
                || (spr->GetHitAreaHolder() && spr->GetHitAreaHolder()->ActsAsButton()))
            {
                // Check if sprite should be ignored
                if (!ignoreMC || (ignoreMC != pCurrentParent))
                    return pCurrentParent;
            }
            pCurrentParent = pCurrentParent->GetParent();
        }
    }
    return 0;
}

bool DisplayObjectBase::IsAncestor(DisplayObjectBase* ch) const
{
    DisplayObjectBase* p;
    for (p = ch->GetParent(); p; p = p->GetParent())
    {
        if (p == this)
            return true;
    }
    return false;
}

void DisplayObjectBase::SetDirtyFlag()
{
    //GetMovieImpl()->SetDirtyFlag();
}


#if (defined SF_CC_SNC && defined SF_OS_PS3)
    #pragma control %push postopt=5
#endif

void DisplayObjectBase::UpdateTransform3D()
{
    Matrix3F rotXMat(Matrix3F::NoInit), transMat(Matrix3F::NoInit), scaleMat(Matrix3F::NoInit), rotYMat(Matrix3F::NoInit), rotZMat(Matrix3F::NoInit);

    // build new matrix 
    transMat = Matrix3F::Translation((float)pGeomData->X, (float)pGeomData->Y,(float)pGeomData->Z);
    scaleMat = Matrix3F::Scaling((float)pGeomData->XScale/100.f, (float)pGeomData->YScale/100.f, (float)pGeomData->ZScale/100.f);

    // MOOSE TODO - collapse this into one matrix?
    rotXMat = pGeomData->XRotation != 0.0 ? Matrix3F::RotationX((float)SF_DEGTORAD(pGeomData->XRotation)) : Matrix3F::Identity; 
    rotYMat = pGeomData->YRotation != 0.0 ? Matrix3F::RotationY((float)SF_DEGTORAD(pGeomData->YRotation)) : Matrix3F::Identity; 
    rotZMat =  pGeomData->Rotation != 0.0 ? Matrix3F::RotationZ((float)SF_DEGTORAD(pGeomData->Rotation))  : Matrix3F::Identity; 

    // Apply transforms in SRT order
    Matrix3F m(transMat, Matrix3F(Matrix3F(rotZMat, rotYMat), Matrix3F(rotXMat, scaleMat)));   // operations happen right to left 

    if (m.IsValid())
        SetMatrix3D(m);
}
#if (defined SF_CC_SNC && defined SF_OS_PS3)
    #pragma control %pop postopt
#endif

// Sets indirect transform for 'this' obj that will use its current parent as transform parent.
// The TreeNode for 'this' obj is removed from its current location in render tree, 
// and it is not re-inserted into it; it is caller's responsibility to re-insert it into the render tree
// at appropriate position. Returns the detached render node.
// 'newParent' node is used only for safety check, to avoid circular references. Can be NULL.
Ptr<Render::TreeNode> DisplayObjectBase::SetIndirectTransform(Render::TreeNode* newParent)
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    SF_ASSERT(!renderNode->IsMaskNode());

    Render::TreeContainer* oldParentNode = static_cast<Render::TreeContainer*>(renderNode->GetParent());
    
    // save original parent for correct scale9 operations
    renderNode->SetOrigScale9Parent(oldParentNode);

    // safety check - the node with indirect transform can't be set on its children.
    for (Render::TreeNode* c = newParent; c; c = c->GetParent())
    {
        if (c == renderNode)
            return NULL;
    }

    // find index of renderNode in its current hierarchy.
    DisplayObjContainer* parent = NULL;
    if (GetParent())
        parent = GetParent()->CharToDisplayObjContainer();
    if (parent)
    {
        DisplayList& dl = parent->GetDisplayList();

        UPInt idx = dl.FindDisplayIndex(this);
        dl.RemoveFromRenderTree(parent, idx);
    }

    GetMovieImpl()->AddIndirectTransformPair(parent, oldParentNode, this);

    if (!pIndXFormData)
        pIndXFormData = SF_HEAP_AUTO_NEW(this) IndirectTransformDataType;
    pIndXFormData->OrigTransformMatrix = renderNode->M3D();
    pIndXFormData->IsOrig3D            = renderNode->Is3D();
    Flags |= Mask_IndirectTransform;
    return renderNode;
}

// Note: the render node should be already detached from its parent
void DisplayObjectBase::RemoveIndirectTransform(bool readdToDisplayList)
{
    if (HasIndirectTransform())
    {
        Ptr<Render::TreeNode> renderNode = GetRenderNode();
        SF_ASSERT(renderNode->GetParent() == NULL);

        // Restore matrix
        if (!pIndXFormData->IsOrig3D)
        {
            renderNode->Clear3D();
            renderNode->SetMatrix(*(Matrix2F*)(void*)&pIndXFormData->OrigTransformMatrix);
        }
        else
        {
            renderNode->SetMatrix3D(pIndXFormData->OrigTransformMatrix);
        }
        delete pIndXFormData;
        pIndXFormData = NULL;
        Flags &= ~Mask_IndirectTransform;

        // Find a pair
        MovieImpl::IndirectTransPair p = GetMovieImpl()->RemoveIndirectTransformPair(this);

        if (readdToDisplayList && p.OriginalParent)
        {
            DisplayList& dl = p.OriginalParent->GetDisplayList();
            UPInt idx = dl.FindDisplayIndex(this);
            if (idx != SF_MAX_UPINT)
                dl.InsertIntoRenderTree(p.OriginalParent, idx);
        }
        
        renderNode->SetOrigScale9Parent(NULL);
    }
}

void DisplayObjectBase::SetScale9Grid(const RectF& gr)
{
    GetRenderNode()->SetScale9Grid(gr);
    SetDirtyFlag();
}

RectF DisplayObjectBase::GetScale9Grid() const
{
    return GetRenderNode()->GetScale9Grid();
}

Render::Scale9GridInfo* DisplayObjectBase::CreateScale9Grid() const
{
    InteractiveObject* parent   = GetParent();
    Matrix             shapeMtx = GetMatrix();
    while (parent)
    {
        if (parent->HasScale9Grid())
        {
            RectF bounds = parent->GetRectBounds(Matrix2F());
            return SF_HEAP_AUTO_NEW(this) Render::Scale9GridInfo(parent->GetScale9Grid(), 
                parent->GetMatrix(), shapeMtx, bounds);
        }
        shapeMtx.Append(parent->GetMatrix());
        parent = parent->GetParent();
    }
    return 0;
}


void DisplayObjectBase::SetRendererString(const char* str)
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    renderNode->SetRendererString(str);
}

void DisplayObjectBase::SetRendererFloat(float fl)
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    renderNode->SetRendererFloat(fl);
}

const char* DisplayObjectBase::GetRendererString()
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    return renderNode->GetRendererString();
}

float DisplayObjectBase::GetRendererFloat()
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    return renderNode->GetRendererFloat();
}

void DisplayObjectBase::DisableBatching(bool b)
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    renderNode->DisableBatching(b);
}

bool DisplayObjectBase::IsBatchingDisabled()
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    return renderNode->IsBatchingDisabled();
}

// 4.5.32 Function
// Tutorial in https://help.autodesk.com/view/SCLFRM/ENU/?guid=__scaleform_help_as3_reference_displayobjectex_extensions_html

void DisplayObjectBase::SetInvertedMask(bool b)
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    return renderNode->SetInvertedMask(b);
}

bool DisplayObjectBase::GetInvertedMask()
{
    Ptr<Render::TreeNode> renderNode = GetRenderNode();
    return renderNode->GetInvertedMask();
}
//////////////////////////////////////////////////////////////////////////
DisplayObject::DisplayObject(ASMovieRootBase* pasRoot, InteractiveObject* pparent, ResourceId id)
:   
    DisplayObjectBase(pasRoot, pparent, id),
    pScrollRect(0),
    Flags(0)
{
    pMaskCharacter = NULL;
    SetScriptableObjectFlag();

    SetInstanceBasedNameFlag();
    SetAcceptAnimMovesFlag();
}

DisplayObject::~DisplayObject()
{
    delete pScrollRect;

    // Free/reset mask. Note that, the mask is only readded to the display list
	// if it is a timeline mask, not if it was set programmatically.
    if (GetMask())
        SetMask(NULL, GetClipDepth() != 0);

    DisplayObject* pmaskOwner = GetMaskOwner();
    if (pmaskOwner)
        pmaskOwner->SetMask(NULL);

    if (pNameHandle)
        pNameHandle->ReleaseCharacter();
}

void DisplayObject::OnEventLoad()
{
    DisplayObjectBase::OnEventLoad();
    if (HasAvmObject())
        GetAvmObjImpl()->OnEventLoad();
}

void DisplayObject::OnEventUnload()
{
    DisplayObjectBase::OnEventUnload();
    // need to release the character to avoid accidental reusing unloaded character.
    if (pNameHandle)
        pNameHandle->ReleaseCharacter();
    if (HasAvmObject())
        GetAvmObjImpl()->OnEventUnload();
}

ASString        DisplayObject::CreateNewInstanceName() const
{
    MovieImpl* proot = GetMovieImpl();
    return ASString(proot->CreateNewInstanceName());
}

CharacterHandle* DisplayObject::CreateCharacterHandle() const
{
    if (!pNameHandle)
    {   
        MovieImpl* proot = GetMovieImpl();

        // Hacky, but this can happen.
        // Will clearing child handles recursively on parent release work better?
        if (IsUnloaded())
        {
            SF_DEBUG_WARNING(1, "GetCharacterHandle called on unloaded sprite");
            // Returns temp handle which is essentially useless.
            pNameHandle = *SF_HEAP_NEW(proot->GetMovieHeap())
                CharacterHandle(proot->GetStringManager()->CreateEmptyString(), 0, 0);
            return pNameHandle;
        }        

        // Create new instance names as necessary.
        ASString name(CreateNewInstanceName());
        // SetName logic duplicated to take advantage of 'mutable' pNameHandle.
        pNameHandle = *SF_HEAP_NEW(proot->GetMovieHeap())
            CharacterHandle(name, pParent, const_cast<DisplayObject*>(this));
    }

    return pNameHandle;
}

Double DisplayObject::GetMouseX() const
{
    Render::PointF a = GetMovieImpl()->GetMouseState(0)->GetLastPosition();
    Render::PointF b;
    GetMovieImpl()->SetNormalizedScreenCoords(a);
    TransformPointToLocal(&b, a, false);
    return TwipsToPixels(floor(b.x+0.5));
}

Double DisplayObject::GetMouseY() const
{
    Render::PointF a = GetMovieImpl()->GetMouseState(0)->GetLastPosition();
    Render::PointF b;
    GetMovieImpl()->SetNormalizedScreenCoords(a);
    TransformPointToLocal(&b, a, false);
    return TwipsToPixels(floor(b.y+0.5));
}

void    DisplayObject::SetName(const ASString& name)
{
    if (!name.IsEmpty())
        ClearInstanceBasedNameFlag();

    if (pNameHandle)
    {
        pNameHandle->ChangeName(name, pParent);

        // TBD: Propagate update to all children ??
    }
    else
    {
        pNameHandle = *SF_HEAP_AUTO_NEW(this) CharacterHandle(name, pParent, this);
    }
}

void    DisplayObject::SetOriginalName(const ASString& name)
{
    DisplayObject::SetName(name);
    CharacterHandle* pnameHandle = GetCharacterHandle();
    if (pnameHandle)
        return pnameHandle->SetOriginalName(name);
}

ASString DisplayObject::GetOriginalName() const
{
    CharacterHandle* pnameHandle = GetCharacterHandle();
    if (pnameHandle)
        return pnameHandle->GetOriginalName();
    return GetName();
}

ASString DisplayObject::GetName() const
{
    CharacterHandle* pnameHandle = GetCharacterHandle();
    if (pnameHandle)
        return pnameHandle->GetName();
    return GetMovieImpl()->GetStringManager()->CreateEmptyString();
}

// Determines the absolute path of the character.
const char*    DisplayObject::GetAbsolutePath(String *ppath) const
{
    if (HasAvmObject())
        return GetAvmObjImpl()->GetAbsolutePath(ppath);
    return "";
}

void    DisplayObject::SetAcceptAnimMoves(bool accept)
{ 
    if (!pGeomData)
    {
        GeomDataType geomData;
        SetGeomData(GetGeomData(geomData));
    }
    SetAcceptAnimMovesFlag(accept); 

    // cache the flag locally
    SetContinueAnimationFlag(GetMovieImpl()->IsContinueAnimationFlagSet()); 

    if (GetMovieImpl()->IsContinueAnimationFlagSet() && accept)
    {
        // if continueAnimation flag is set and we restore timeline acceptance - 
        // remove the pGeomData to provide correct coordinates/scales by accessing
        // via ActionScript.
        delete pGeomData;
        pGeomData = NULL;
    }
    SetDirtyFlag();
}

// If the object has ClipDepth this means it is used as timeline mask. 
// Remove its render node from the render tree, set clip depth to 0 and
// reinsert back as a regular node (as non-mask).
void DisplayObject::ResetClipDepth() 
{
    if (ClipDepth != 0)
    {
        DisplayObjContainer* parent = NULL;
        if (GetParent())
            parent = GetParent()->CharToDisplayObjContainer();
        if (parent)
        {
            DisplayList& dl = parent->GetDisplayList();

            UPInt idx = dl.FindDisplayIndex(this);
            dl.RemoveFromRenderTree(parent, idx);

            SetClipDepth(0); 

            dl.InsertIntoRenderTree(parent, idx);
        }
        else
            SetClipDepth(0); 
    }
}

void DisplayObject::SetMask(DisplayObject* pmaskSprite, bool readdMaskToDisplayList) 
{ 
    Ptr<Render::TreeNode> renNode = GetRenderNode();

    // setMask method override layers' masking (by zeroing ClipDepth).
    ResetClipDepth();
    if (pmaskSprite)
    {
        if (HasScrollRect())
        {
            SF_DEBUG_WARNING(1, "set mask is incompatible with scrollRect, removing scrollRect");
            SetScrollRect(NULL);
        }
        pmaskSprite->ResetClipDepth();
    }

    DisplayObject* poldMask = GetMask();
    if (poldMask)
    {
        // remove this movie clip from the mask holder for the old mask
        poldMask->SetMaskOwner(NULL);

        renNode->SetMaskNode(NULL);

        // mask sprite is NULL: need to restore the maskNode at its 
        // original position in render tree.
        poldMask->RemoveIndirectTransform(readdMaskToDisplayList);
    }

    // the sprite being masked cannot be a mask for another sprite
    DisplayObject* poldOwner = GetMaskOwner();
    if (poldOwner)
    {
        poldOwner->SetMask(NULL);
    }
    if (pmaskSprite && pmaskSprite->GetMaskOwner())
        pmaskSprite->GetMaskOwner()->SetMask(NULL);

    if (pMaskCharacter && !IsUsedAsMask())
        pMaskCharacter->Release();

    Ptr<Render::TreeNode> maskNode;
    if (pmaskSprite)
        maskNode = pmaskSprite->SetIndirectTransform(renNode);

    renNode->SetMaskNode(maskNode);
    SetUsedAsMask(false);

    if (maskNode)
    {
        pMaskCharacter = pmaskSprite;
        if (pmaskSprite)
            pmaskSprite->AddRef();

        if (pmaskSprite) 
            pmaskSprite->SetMaskOwner(this);
    }
    else
        pMaskCharacter = NULL;
   
    SetDirtyFlag();
}

DisplayObject* DisplayObject::GetMask() const
{
    if (pMaskCharacter && !IsUsedAsMask())
    {
        SF_ASSERT(pMaskCharacter->IsUsedAsMask() && pMaskCharacter->pMaskOwner == this);
        return pMaskCharacter;
    }
    return NULL;
}

void DisplayObject::SetMaskOwner(DisplayObject* pmaskOwner)
{
    if (GetMask()) 
        SetMask(NULL);

    SetUsedAsMask((pmaskOwner != NULL)?true:false);
    pMaskOwner = pmaskOwner;
}

DisplayObject* DisplayObject::GetMaskOwner() const
{
    if (pMaskOwner && IsUsedAsMask())
    {
        SF_ASSERT(!pMaskOwner->IsUsedAsMask() && pMaskOwner->pMaskCharacter == this);
        return pMaskOwner;
    }
    return NULL;
}

const Matrix&   DisplayObject::GetMatrix() const
{
    if (!pScrollRect)
        return DisplayObjectBase::GetMatrix();
    else
    {
        return *(Matrix2F *)(void*)&pScrollRect->OrigTransformMatrix;
    }
}

void DisplayObject::SetMatrix(const Matrix& mt)
{
    if (!pScrollRect)
        DisplayObjectBase::SetMatrix(mt);
    else
    {
        pScrollRect->OrigTransformMatrix = mt;
        Matrix2F m = *(Matrix2F *)(void*)&pScrollRect->OrigTransformMatrix;
        m.PrependTranslation((float)-pScrollRect->Rectangle.x1, (float)-pScrollRect->Rectangle.y1);
        GetRenderNode()->SetMatrix(m);

        Matrix2F mm;
        mm.PrependTranslation((float)pScrollRect->Rectangle.x1, (float)pScrollRect->Rectangle.y1);
        pScrollRect->Mask->GetTreeNode()->SetMatrix(mm);
    }
}

const Matrix3F& DisplayObject::GetMatrix3D() const
{
    if (!pScrollRect)
        return DisplayObjectBase::GetMatrix3D();
    else
    {
        return pScrollRect->OrigTransformMatrix;
    }
}

void DisplayObject::SetMatrix3D(const Matrix3F& mt)
{
    if (!pScrollRect)
        DisplayObjectBase::SetMatrix3D(mt);
    else
    {
        pScrollRect->OrigTransformMatrix = mt;
        pScrollRect->IsOrig3D = true;

        Matrix3F m = pScrollRect->OrigTransformMatrix;
        Matrix3F tr = Matrix3F::Translation((float)-pScrollRect->Rectangle.x1, 
                                            (float)-pScrollRect->Rectangle.y1, 0);
        m.Prepend(tr);
        GetRenderNode()->SetMatrix3D(m);

        Matrix3F mm;
        Matrix3F trm = Matrix3F::Translation((float)pScrollRect->Rectangle.x1, 
                                             (float)pScrollRect->Rectangle.y1, 0);
        mm.Prepend(trm);
        pScrollRect->Mask->GetTreeNode()->SetMatrix3D(mm);
    }
}

void DisplayObject::SetScrollRect(const Render::RectD* r)
{
    if (HasIndirectTransform())
    {
        // scrollRect is incompatible with indirect transform!
        SF_DEBUG_WARNING(1, "scrollRect is incompatible with indirect transform (topmostLevel, set mask)");
    }
    if (GetMask())
        SetMask(NULL);
    if (HasIndirectTransform())
        return;
    TreeNode* renNode = GetRenderNode();
    if (pScrollRect)
    {
        // removes the mask set by previous call to scrollRect
        renNode->SetMaskNode(NULL); 
        // Restore matrix
        if (!pScrollRect->IsOrig3D)
        {
            Matrix2F m = *(Matrix2F *)(void*)&pScrollRect->OrigTransformMatrix;
            renNode->SetMatrix(m);
        }
        else
        {
            renNode->SetMatrix3D(pScrollRect->OrigTransformMatrix);
        }
    }
    if (!r) 
    {
        delete pScrollRect;
        pScrollRect = NULL;
    }
    else
    {
        if (renNode->HasMask())
        {
            // all masks should be removed now.... Timeline mask? TODO?
            renNode->SetMaskNode(NULL); 
        }
        if (!pScrollRect)
            pScrollRect = SF_HEAP_AUTO_NEW(this) ScrollRectInfo;
        pScrollRect->Rectangle = *r;
        pScrollRect->OrigTransformMatrix = renNode->M3D();
        pScrollRect->IsOrig3D            = renNode->Is3D();

        // create a mask rectangle via DrawingContext and use
        // its TreeContainer as a mask node.
        pScrollRect->Mask = *GetMovieImpl()->CreateDrawingContext();
        pScrollRect->Mask->BeginSolidFill(0xFFFFFFFF);
        float w = (float)r->Width(), h = (float)r->Height();
        pScrollRect->Mask->MoveTo(0, 0);
        pScrollRect->Mask->LineTo(w, 0);
        pScrollRect->Mask->LineTo(w, h);
        pScrollRect->Mask->LineTo(0, h);
        pScrollRect->Mask->EndFill();
        pScrollRect->Mask->UpdateRenderNode();

        if (!pScrollRect->IsOrig3D)
            SetMatrix(renNode->M2D());
        else
            SetMatrix3D(renNode->M3D());

        renNode->SetMaskNode(pScrollRect->Mask->GetTreeNode());
    }
}

bool  DisplayObject::HasEventHandler(const EventId& id) const
{
    if (HasAvmObject())
        return GetAvmObjImpl()->HasEventHandler(id);
    return false;
}

bool  DisplayObject::OnEvent(const EventId& id) 
{ 
    if (HasAvmObject())
        return GetAvmObjImpl()->OnEvent(id);
    return false; 
}

bool DisplayObject::TransformPointToLocalAndCheckBounds
(Render::PointF *p, const Render::PointF & pt, bool bPtInParentSpace, const Matrix2F *mat) const
{
    if (pScrollRect)
    {
        Render::PointF localPt2;
        TransformPointToLocal(&localPt2, pt, bPtInParentSpace, mat);

        *p = localPt2;
        *p += Render::PointF((float)pScrollRect->Rectangle.x1, (float)pScrollRect->Rectangle.y1);

        if (localPt2.x < 0 || localPt2.y < 0 ||
            localPt2.x > pScrollRect->Rectangle.Width() ||
            localPt2.y > pScrollRect->Rectangle.Height())
            return false;
    }
    else
        TransformPointToLocal(p, pt, bPtInParentSpace, mat);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// ***** CharacterHandle
CharacterHandle::CharacterHandle(const ASString& name, DisplayObject *pparent, DisplayObject* pcharacter)
: Name(name), NamePath(name.GetManager()->CreateEmptyString()), OriginalName(name)
{
    RefCount    = 1;
    pCharacter  = pcharacter;    

    if (pcharacter && pcharacter->IsAVM1())
    {
        // Compute path based on parent
        String namePathBuff;
        if (pparent)
        {
            pparent->GetAbsolutePath(&namePathBuff);
            namePathBuff += ".";
        }
        namePathBuff += Name.ToCStr();
        NamePath = name.GetManager()->CreateString(namePathBuff);
    }
}

CharacterHandle::~CharacterHandle()
{
}

// Release a character reference, used when character dies
void    CharacterHandle::ReleaseCharacter()
{
    pCharacter = 0;
}


// Changes the name.
void    CharacterHandle::ChangeName(const ASString& name, DisplayObject *pparent)
{
    Name = name;
    if (pparent && pparent->IsAVM1())
    {
        // Compute path based on parent
        String namePathBuff;
        if (pparent)
        {
            pparent->GetAbsolutePath(&namePathBuff);
            namePathBuff += ".";
        }
        namePathBuff += Name.ToCStr();
        NamePath = name.GetManager()->CreateString(namePathBuff);

        // Do we need to update paths in all parents ??
    }
}

// Resolve the character, considering path if necessary.
DisplayObject* CharacterHandle::ResolveCharacter(MovieImpl *proot) const
{
    if (pCharacter)
        return pCharacter;
    // Resolve a global path based on Root.
    return proot->FindTarget(NamePath);
}

DisplayObject* CharacterHandle::ForceResolveCharacter(MovieImpl *proot) const
{
    // Resolve a global path based on Root.
    return proot->FindTarget(NamePath);
}

void CharacterHandle::ResetName(ASStringManager* psm)
{
    Name     = psm->CreateEmptyString();
    NamePath = Name; 
}


}} // namespace Scaleform::GFx

