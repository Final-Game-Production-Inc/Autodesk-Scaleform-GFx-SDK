/**************************************************************************

Filename    :   AS3_AvmBitmap.cpp
Content     :   Implementation of timeline part of Bitmap.
Created     :   Jun, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS3/AS3_AvmBitmap.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Render/Render_TreeLog.h"
#include "Render/Render_ShapeDataFloat.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_Bitmap.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_BitmapData.h"

namespace Scaleform { namespace GFx { namespace AS3 {

AvmBitmap::AvmBitmap(ASMovieRootBase* pasRoot,
                     const CharacterCreateInfo& ccinfo,
                     InteractiveObject* pparent, 
                     ResourceId id)
                     :   DisplayObject(pasRoot, pparent, id),
                     AvmDisplayObj(GetThis()), pDefImpl(ccinfo.pBindDefImpl)
{
    Resource* pres = ccinfo.pResource;

    if (id != ResourceId::InvalidId)
    {
        ResourceHandle      rh;
        if (pDefImpl->GetDataDef()->GetResourceHandle(&rh, id))
        {        
            pres = rh.GetResource(&pDefImpl->GetResourceBinding());
        }
    }
    if (pres)
    {
        if (pres->GetResourceType() == Resource::RT_Image)
        {
            pImage = static_cast<ImageResource*>(pres);
        }
    }
}
AvmBitmap::~AvmBitmap()
{
    if (pDefImpl)
        GetMovieImpl()->AddMovieDefToKillList(pDefImpl);

    // We must nullify AvmObjOffset to avoid a second call of ~AvmBitmap from 
    // ~DisplayObjectBase @ 'GetAvmObjImpl()->~AvmDisplayObjBase()'. This can happen,
    // since the AvmBitmap is inherited from both DisplayObject and AvmDisplayObj and
    // ~AvmDisplayObj is called automatically.
    AvmObjOffset = 0;
}

// creates instance of AS class for the object
// returns >0 if needs ExecuteCode (number of stack frames), 0 - otherwise
unsigned AvmBitmap::CreateASInstance(bool execute)
{
    unsigned rv = 0;
    if (!GetAS3Obj()) // create AS3 instance of Bitmap...
    {
        rv = AvmDisplayObj::CreateASInstance(execute);
    }
    if (GetAS3Obj()) // ... and create AS3 instance of the BitmapData
    {
        // create BitmapData
        Instances::fl_display::Bitmap* as3bmp = static_cast<Instances::fl_display::Bitmap*>(GetAS3Obj());

        if (!as3bmp->pBitmapData && pDispObj->GetResourceMovieDef())
        {
            // Obtain the size parameters for the constructor from the resource, if possible.
            Value params[] = { Value(0.f), Value(0.f) };
            if ( pImage && pImage->GetImage() )
            {
                ImageSize size = pImage->GetImage()->GetSize();
                params[0] = Value(size.Width);
                params[1] = Value(size.Height);
            }
                
            SPtr<Instances::fl_display::BitmapData> bmpData;

            // resolve class name, check if id is associated with a custom class;
            // use "flash.display.BitmapData" otherwise.
            const String* pclassName = pDispObj->GetResourceMovieDef()->GetNameOfExportedResource(pDispObj->GetId());
            if (pclassName)
            {
                ASVM& vm = *GetAS3Root()->GetAVM();
                Value v;
                bool need2execute = vm.Construct(*pclassName, *AppDomain, v, 2, params);
                if (need2execute)
                {
                    if (execute)
                        vm.ExecuteCode();
                    else
                        ++rv; // inc cnt of frames to execute later
                }

                if (!GetAS3Root()->GetAVM()->IsException() && !v.IsNullOrUndefined())
                {
                    bmpData = static_cast<Instances::fl_display::BitmapData *>(v.GetObject());
                }
                else
                {
                    bmpData = 0;
                    if (GetAS3Root()->GetAVM()->IsException())
            {
                        GetAS3Root()->GetAVM()->OutputAndIgnoreException();
                    }
                }
            }
            else
            {
                GetAS3Root()->GetAVM()->ConstructBuiltinObject(bmpData, "flash.display.BitmapData", 2, params).DoNotCheck();
            }

            if (bmpData)
            {
                bmpData->CreateLibraryObject(pImage, pDispObj->GetResourceMovieDef());
                as3bmp->SetBitmapData(bmpData);
            }
        }
    }
    return rv;
}

DisplayObjectBase::TopMostResult AvmBitmap::GetTopMostMouseEntity(
    const Render::PointF &pt, TopMostDescr* pdescr)
{   
    if (!GetVisible())
        return TopMost_FoundNothing;

    if (pImage)
    {
        Render::PointF p;          
        TransformPointToLocal(&p, pt);

        Render::ImageRect rect = pImage->GetImage()->GetRect();
        RectF r(0, 0, 
            SizeF(PixelsToTwips((float)rect.Width()), PixelsToTwips((float)rect.Height())));
        Render::Image* img = pImage->GetImage()->GetAsImage();
        if (img)
        {
            Matrix2F m;
            img->GetMatrix(&m);
            m.EncloseTransform(&r, RectF(r));
        }
        if (r.Contains(p))
        {
            // need to return parent (since it is an InteractiveObject)
            pdescr->pResult = DisplayObject::GetParent();
            return TopMost_Found;
        }
    }

    return TopMost_FoundNothing;
}

bool AvmBitmap::PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask) const
{     
    SF_UNUSED(hitTestMask);
    if (pImage)
    {
        Render::ImageRect rect = pImage->GetImage()->GetRect();
        RectF r(0, 0, 
            SizeF(PixelsToTwips((float)rect.Width()), PixelsToTwips((float)rect.Height())));
        if (r.Contains(pt))
        {
            return true;
        }
    }
    return false;
}

// "transform" matrix describes the transform applied to parent and us,
// including the object's matrix itself. This means that if transform is
// identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
RectF  AvmBitmap::GetBounds(const Matrix &transform) const
{
    RectF r;
    r.Clear();
    if (pImage)
    {
        Render::ImageRect imgR = pImage->GetImage()->GetRect();
        Matrix m;
        Render::Image* img = pImage->GetImage()->GetAsImage();
        if (img) // can it be NULL? Meaning it is not the Image?
        {
            img->GetMatrix(&m);
            m.EncloseTransform(&r, RectF(float(PixelsToTwips(imgR.x1)), float(PixelsToTwips(imgR.y1)), 
                                         float(PixelsToTwips(imgR.x2)), float(PixelsToTwips(imgR.y2))));
        }
        else
        {
            r = RectF(float(PixelsToTwips(imgR.x1)), float(PixelsToTwips(imgR.y1)), 
                      float(PixelsToTwips(imgR.x2)), float(PixelsToTwips(imgR.y2)));
        }
    }
    return transform.EncloseTransform(RectF(0, 0, SizeF(r.GetSize())));
}

Instances::fl_display::Bitmap* AvmBitmap::GetAS3Bitmap() const
{
    SF_ASSERT(GetAS3Root()->GetAVM()->IsOfType(Value(GetAS3Obj()), "flash.display.Bitmap", GetAS3Root()->GetAVM()->GetCurrentAppDomain()));
    return static_cast<Instances::fl_display::Bitmap*>(GetAS3Obj());
}


Instances::fl_display::BitmapData* AvmBitmap::GetBitmapData() const
{
    Instances::fl_display::Bitmap* bmp = GetAS3Bitmap();
    if (bmp == NULL)
    {
        return NULL;
    }
    return bmp->GetBitmapData();
}

Render::TreeNode* AvmBitmap::RecreateRenderNode() const
{
    Instances::fl_display::Bitmap* as3bmp = GetAS3Bitmap();
    UPInt index = ~0u;
    Render::TreeContainer* parent = NULL;
    bool renNodeExisted = !!pRenNode;
    if (as3bmp)
    {
        if (pRenNode && pRenNode->GetParent())
        {
            if (!pRenNode->IsMaskNode())
                parent = static_cast<Render::TreeContainer*>(pRenNode->GetParent());
            else
                SF_ASSERT(0); // mask is not supported yet @TODO
            // remove old render tree node with saving position to re-insert a new one at.
            // currently, if bitmap is changed we need to re-create the TreeNode. 
            index = 0;
            for (UPInt n = parent->GetSize(); index < n; ++index)
            {
                if (parent->GetAt(index) == pRenNode)
                    break;
            }
            parent->Remove(index, 1);
        }

        // bitmap has been changed, re-create image shape
        ImageResource* pimg = as3bmp->GetImageResource();
        if (pimg)
            pImage = pimg;
    }
    if (renNodeExisted)
    {
        Ptr<TreeNode> node = pRenNode;

        pRenNode = CreateRenderNode(GetMovieImpl()->GetRenderContext());
        SF_ASSERT(pRenNode);
        if (pRenNode)
        {
            if (node)
            {
                pRenNode->CopyGeomData(*node.GetPtr());
            }
            else
                pRenNode->SetVisible(IsVisibleFlagSet());
            // re-insert the new render tree node at the appropriate position
            if (parent)
            {
                SF_ASSERT(index != ~0u);
                parent->Insert(index, pRenNode);
            }
        }
    }
    return pRenNode;
}

Ptr<Render::TreeNode> AvmBitmap::CreateRenderNode(Render::Context& context) const
{
    pRenNode = *context.CreateEntry<Render::TreeShape>();
    if (!CreateBitmapShape())
    {
        pRenNode = NULL;
        return NULL;
    }
    return pRenNode;
}

bool AvmBitmap::CreateBitmapShape() const
{
    SF_ASSERT(pRenNode);
    Render::TreeShape* tshp = static_cast<Render::TreeShape*>(pRenNode.GetPtr());
    MemoryHeap* renderHeap = Memory::GetHeapByAddress(tshp);
    Ptr<Render::ShapeDataFloat> shapeData = *SF_HEAP_NEW(renderHeap) Render::ShapeDataFloat();
    Render::FillStyleType fill;
    fill.Color = 0; // just to avoid warning on Mac
    //fill.Color = 0xFF0394AA; //@DBG; comment all accesses to fill.pFill to enable solid color fill
    fill.pFill = *SF_HEAP_NEW(renderHeap) Render::ComplexFill();
    if (pImage)
    {
        Ptr<Render::Image> img;
        if (pImage->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
            img = static_cast<Render::Image*>(pImage->GetImage());
        else
        {
            ImageCreator* imgCreator = GetMovieImpl()->GetImageCreator();
            if (imgCreator)
            {
                ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, GetMovieImpl()->GetMovieHeap());
                img = *imgCreator->CreateImage(cinfo, static_cast<Render::ImageSource*>(pImage->GetImage()));
            }
            else
            {
                LogDebugMessage(Log_Warning, "Image is not created: can't find ImageCreator.");
                return false;
            }
        }
        fill.pFill->pImage = img;
        if (!img)
        {
            LogDebugMessage(Log_Warning, "Image is not created.");
            return false;
        }
        fill.pFill->ImageMatrix.SetIdentity();
        fill.pFill->ImageMatrix.AppendScaling(0.05f);
        fill.pFill->FillMode.Fill = (Wrap_Clamp | Sample_Point);
        if (GetAS3Obj())
        {
            Instances::fl_display::Bitmap* as3bmp = GetAS3Bitmap();
            if (as3bmp->Smoothing)
                fill.pFill->FillMode.Fill = (Wrap_Clamp | Sample_Linear);
        }
        else if (GFX_AS3_SMOOTH_BMP_BY_DEFAULT)
            fill.pFill->FillMode.Fill = (Wrap_Clamp | Sample_Linear);

        Render::ImageRect imgR = img->GetRect();
        Matrix2F m;
        img->GetMatrix(&m);
        RectF r;
        m.EncloseTransform(&r, RectF(float(imgR.x1), float(imgR.y1), float(imgR.x2), float(imgR.y2)));

        shapeData->AddFillStyle(fill);
        shapeData->StartPath(1, 0, 0);
        shapeData->MoveTo(0, 0);
        shapeData->LineTo(float(PixelsToTwips(r.Width())), 0);
        shapeData->LineTo(float(PixelsToTwips(r.Width())), float(PixelsToTwips(r.Height())));
        shapeData->LineTo(0, float(PixelsToTwips(r.Height())));
        shapeData->ClosePath();
        shapeData->EndPath();
        shapeData->EndShape();
        Ptr<ShapeMeshProvider> pr = *SF_HEAP_NEW(renderHeap) Render::ShapeMeshProvider(shapeData);
        tshp->SetShape(pr);
    }
    else
    {
        // no image, but still need to create an empty treeshape...
        shapeData->StartPath(0, 0, 0);
        shapeData->MoveTo(0, 0);
        shapeData->LineTo(1, 1);
        shapeData->ClosePath();
        shapeData->EndPath();
        shapeData->EndShape();
        Ptr<ShapeMeshProvider> pr = *SF_HEAP_NEW(renderHeap) Render::ShapeMeshProvider(shapeData);
        tshp->SetShape(pr);
    }
    return true;
}

void AvmBitmap::ForceShutdown()
{
    if (HasAS3Obj())
    {
        GetAS3Obj()->SetLoaderInfo(NULL);
    }
}

void AvmBitmap::SetResourceMovieDef(MovieDefImpl* md)
{
    pDefImpl = md;
}

}}} // SF::GFx::AS3
