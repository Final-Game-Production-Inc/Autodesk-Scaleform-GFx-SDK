/**************************************************************************

Filename    :   GFx_DrawingContext.cpp
Content     :   Drawing API implementation
Created     :   Aug 7, 2007
Authors     :   Maxim Shemanarev, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_DrawingContext.h"
#include "GFx/AMP/Amp_ViewStats.h"
#include "GFx/GFx_ImageResource.h"
#include "GFx/GFx_ImageCreator.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Render/Render_Gradients.h"
#include "Render/Render_HitTest.h"

namespace Scaleform { namespace GFx {

DrawingContext::DrawingContext(Render::Context& renCtxt, ImageCreator* imgCreator) : 
    RenContext(renCtxt),
    ImgCreator(imgCreator),
    mFillStyle(),
    mLineStyle(),
    States(0)
{
    States |= State_NewShapeFlag;
    pNext = pPrev = NULL;
    pTreeContainer = *RenContext.CreateEntry<TreeContainer>();
    Clear();
}

DrawingContext::~DrawingContext()
{
    if (pPrev && pNext)
        RemoveNode();
}

void DrawingContext::Clear()
{
    Shapes = *SF_HEAP_NEW(GetHeap()) PackedShape(GetHeap());
    States = 0;
    States |= State_NewShapeFlag;
    Ex = Ey = 0;
    StX = StY = SF_MIN_FLOAT;
    FillStyle0 = FillStyle1 = StrokeStyle = 0;
    pTreeContainer->Remove(0, pTreeContainer->GetSize());
    States |= State_Dirty;
}

void DrawingContext::SetNoLine()
{
    StrokeStyle = 0;
    States &= ~State_FreshLineStyle;
}

void DrawingContext::SetNoFill()
{
    FillStyle0 = 0;
    FillStyle1 = 0;
}

void DrawingContext::SetLineStyle(float lineWidth, 
                                  unsigned  rgba, 
                                  bool  hinting, 
                                  unsigned scaling, 
                                  unsigned caps,
                                  unsigned joins,
                                  float miterLimit)
{
    mLineStyle.Width = lineWidth;
    mLineStyle.Color = rgba;
    mLineStyle.Flags = (((hinting)?StrokeFlag_StrokeHinting:0) | scaling | caps | joins);
    mLineStyle.Miter = miterLimit;
    mLineStyle.Units = 0.05f;
    StrokeStyle = Shapes->AddStrokeStyle(mLineStyle);
    States |= State_FreshLineStyle;
}

void DrawingContext::ChangeLineStyle(float lineWidth, 
                                     unsigned  rgba, 
                                     bool  hinting, 
                                     unsigned scaling, 
                                     unsigned caps,
                                     unsigned joins,
                                     float miterLimit)
{
    if ((rgba & 0xFF000000) != 0)
    {
        // Check the line width and set it to the possible minimum (hairline) 
        // if negative or zero.
        if (lineWidth <= 0.0f)
            lineWidth  = TwipsToPixels(1.0f);

        if (!SameLineStyle(lineWidth, rgba, hinting, scaling, caps, joins, miterLimit))
        {
            AcquirePath(false);
            SetLineStyle(lineWidth, rgba, hinting, scaling, caps, joins, miterLimit);
        }
    }
    else
    {
        if (!NoLine())
        {
            AcquirePath(false);
            SetNoLine();
        }
    }
}

bool DrawingContext::SameLineStyle(float lineWidth, 
                                   unsigned  rgba, 
                                   bool  hinting, 
                                   unsigned scaling, 
                                   unsigned caps,
                                   unsigned joins,
                                   float miterLimit) const
{
    if (Shapes->GetStrokeStyleCount() == 0)
        return false;

    if (StrokeStyle == 0)
        return false;

    LineStyle ls;
    Shapes->GetStrokeStyle(StrokeStyle, &ls);

    if (ls.pFill)
        return false;

    return ls.Color == rgba &&
           int(ls.Width*20) == int(lineWidth*20) &&
           ls.Flags == (((hinting)?StrokeFlag_StrokeHinting:0) | scaling | caps | joins) &&
           int(ls.Miter*20) == int(miterLimit*20);
}

bool DrawingContext::NoLine() const
{
    return Shapes->GetStrokeStyleCount() == 0 || StrokeStyle == 0;
}

// The function begins new fill with an "empty" style. 
// It returns the pointer to just created fill style, so the caller
// can set any values. The function is used in Action Script, beginGradientFill
unsigned DrawingContext::SetNewFill()
{
    mFillStyle.Color = 0;
    mFillStyle.pFill = 0;
    FillStyle0 = Shapes->AddFillStyle(mFillStyle);
    FillStyle1 = 0;
    return FillStyle0;
}

ComplexFill* DrawingContext::CreateNewComplexFill()
{
    unsigned f = SetNewFill();
    Ptr<ComplexFill> cf = *SF_HEAP_NEW(GetHeap()) ComplexFill();
    FillStyle fs;
    Shapes->GetFillStyle(f, &fs);
    fs.pFill = cf;
    Shapes->UpdateFillStyle(f, fs);
    return cf;
}

ComplexFill* DrawingContext::CreateLineComplexFill()
{
    if (!(States & State_FreshLineStyle))
        StrokeStyle = Shapes->AddStrokeStyle(mLineStyle);
    Ptr<ComplexFill> cf = *SF_HEAP_NEW(GetHeap()) ComplexFill();
    LineStyle ls;
    Shapes->GetStrokeStyle(StrokeStyle, &ls);
    ls.pFill = cf;
    Shapes->UpdateStrokeStyle(StrokeStyle, ls);
    return cf;
}

void DrawingContext::SetNonZeroFill(bool fill)
{
    SF_UNUSED(fill);
    SF_ASSERT(0); //@TODO
    //? Shapes->SetNonZeroFill(fill);
}

// begins fill with current style
void DrawingContext::BeginFill()
{
    if (States & State_NeedClosePath)
        EndFill();
    States |= State_FreshFillStyle | State_NeedClosePath;
}

void DrawingContext::BeginSolidFill(unsigned rgba)
{
    mFillStyle.Color = rgba;;
    mFillStyle.pFill = NULL;
    FillStyle0 = Shapes->AddFillStyle(mFillStyle);
    FillStyle1 = 0;
    BeginFill();
}

bool DrawingContext::IsSolidFill(unsigned rgba) const
{
    return !mFillStyle.pFill && mFillStyle.Color == rgba;
}

void DrawingContext::BeginBitmapFill(FillType fillType,
                                    ImageResource* pimageRes,
                                    const Matrix& mtx)
{
    unsigned styleIdx = SetNewFill();
    if (styleIdx)
    {
        FillStyle fs;
        Shapes->GetFillStyle(styleIdx, &fs);
        fs.pFill = *SF_HEAP_NEW(GetHeap()) ComplexFill();
        Matrix m(mtx);
        m.AppendScaling(20);
        Matrix2F textureMatrix(m.GetInverse());
        fs.pFill->ImageMatrix   = textureMatrix;
        Ptr<Render::Image> img;
        if (pimageRes->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
            img = static_cast<Render::Image*>(pimageRes->GetImage());
        else
        {
            if (!ImgCreator)
                LogDebugMessage(Log_Warning, "ImageCreator is null in BeginBitmapFill");
            else
            {
                ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, Memory::GetHeapByAddress(this));
                img = *ImgCreator->CreateImage(cinfo, static_cast<Render::ImageSource*>(pimageRes->GetImage()));
            }
        }
        fs.pFill->pImage = img;
        switch(fillType)
        {
        case Fill_TiledImage:           fs.pFill->FillMode.Fill = ( Render::Wrap_Repeat | Render::Sample_Point ); break;
        case Fill_TiledSmoothImage:     fs.pFill->FillMode.Fill = ( Render::Wrap_Repeat | Render::Sample_Linear ); break;
        case Fill_ClippedImage:         fs.pFill->FillMode.Fill = ( Render::Wrap_Clamp | Render::Sample_Point ); break;
        case Fill_ClippedSmoothImage:   fs.pFill->FillMode.Fill = ( Render::Wrap_Clamp | Render::Sample_Linear ); break;
        default:                        break;
        }
        Shapes->UpdateFillStyle(styleIdx, fs);

        BeginFill();
    }
}

void DrawingContext::EndFill()
{
    FinishPath();
    FillStyle0 = FillStyle1 = 0;
    StX = StY = SF_MIN_FLOAT;
}

void DrawingContext::MoveTo(float x, float y)
{
    ShapePosInfo pos;
    PosInfo = pos;
    if (!(States & State_FreshFillStyle) && (States & State_NeedClosePath) != 0 && (Ex != StX || Ey != StY))
    {
        // need to close current path, if:
        // a) new beginFill wasn't called, but moveTo was;
        // b) last coordinates do not match start coords.
        LineTo(StX, StY);
    }
    NewPath(x, y);
    StX = x; StY = y;
    States &= ~State_FreshFillStyle;
}

void DrawingContext::LineTo(float x, float y)
{
    if (!(States & State_NeedEndPath))
    {
        NewPath(Ex, Ey);
    }
    Shapes->LineTo(&PosInfo, x, y);
    States &= ~(State_FreshLineStyle);
    Ex = x;
    Ey = y;
    States |= State_Dirty;
}

void DrawingContext::CurveTo(float cx, float cy, float ax, float ay)
{
    if (!(States & State_NeedEndPath))
    {
        NewPath(Ex, Ey);
    }
    Shapes->QuadTo(&PosInfo, cx, cy, ax, ay);
    States &= ~(State_FreshLineStyle);
    Ex = ax;
    Ey = ay;
    States |= State_Dirty;
}

void DrawingContext::NewPath(float x, float y)
{
    AcquirePath((States & State_NewShapeFlag) != 0);
    if (States & State_NewShapeFlag)
        Shapes->StartPath(&PosInfo, Shape_NewLayer, FillStyle0, FillStyle1, StrokeStyle, x, y);
    else
        Shapes->StartPath(&PosInfo, Shape_NewPath, FillStyle0, FillStyle1, StrokeStyle, x, y);
    States &= ~State_NewShapeFlag;
    States |= State_NeedEndPath;
    Ex = x;
    Ey = y;
    States |= State_Dirty;
}

void DrawingContext::FinishPath()
{
    if (States & State_NeedClosePath)
    {
        States &= ~State_NeedClosePath;
        if (Ex != StX || Ey != StY)
        {
            LineTo(StX, StY);
        }
    }
}

bool DrawingContext::AcquirePath(bool newShapeFlag)
{
    if (!IsEmpty())
    {
        States |= State_Dirty;

        if (newShapeFlag && (States & State_NeedClosePath))
            FinishPath();

        if (States & State_NeedEndPath)
        {
            Shapes->EndPath();
            States &= ~State_NeedEndPath;
        }

        (newShapeFlag) ? States |= State_NewShapeFlag : States &= ~State_NewShapeFlag;
        return true;
    }
    return false;
}

void DrawingContext::ComputeBound(RectF *pRect) const
{
     SF_ASSERT(pRect);
     const_cast<DrawingContext*>(this)->UpdateRenderNode();
     for (UPInt i = 0, n = pTreeContainer->GetSize(); i < n; ++i)
     {   
         TreeShape* tsh = static_cast<TreeShape*>(pTreeContainer->GetAt(i));
         ShapeMeshProvider* meshProv = tsh->GetShape();
         SF_ASSERT(meshProv);
         RectF b = meshProv->GetIdentityBounds();
         if (i == 0)
             *pRect = b;
         else
             pRect->Union(b);
     }
}

bool DrawingContext::DefPointTestLocal(const Render::PointF &pt, bool testShape, const DisplayObjectBase *pinst) const
{
    const_cast<DrawingContext*>(this)->UpdateRenderNode();
    SF_UNUSED3(pt, testShape, pinst);

    for (UPInt i = 0, n = pTreeContainer->GetSize(); i < n; ++i)
    {   
        TreeShape* tsh = static_cast<TreeShape*>(pTreeContainer->GetAt(i));
        ShapeMeshProvider* meshProv = tsh->GetShape();
        SF_ASSERT(meshProv);
        RectF b = meshProv->GetIdentityBounds();
        if (b.Contains(pt))
        {
            if (testShape)
            {
                //// DBG
                //bool ret = Render::HitTestFill<Matrix2F>(*this, Matrix2F::Identity, pt.x, pt.y);
                //printf("%d", ret);
                SF_ASSERT(meshProv->GetShapeData());
                return Render::HitTestFill<Matrix2F>(*meshProv->GetShapeData(), Matrix2F::Identity, pt.x, pt.y);
            }
            else
                return true;
        }
    }
    return false;
}

void DrawingContext::UpdateRenderNode()
{
    States &= ~State_Dirty;
    if (!IsEmpty())
    {
        unsigned s = StrokeStyle, f0 = FillStyle0;
        AcquirePath(true);
        Shapes->EndShape();

        Ptr<ShapeMeshProvider> meshProv = *SF_HEAP_NEW(GetHeap()) ShapeMeshProvider(Shapes);
        Ptr<TreeShape> tshp = *RenContext.CreateEntry<TreeShape>();
        tshp->SetShape(meshProv);
        pTreeContainer->Add(tshp);

        Shapes = *SF_HEAP_NEW(GetHeap()) PackedShape(GetHeap());

        // if endFill wasn't called we need to restore FillStyle
        if (f0)
        {
            FillStyle0 = Shapes->AddFillStyle(mFillStyle);
            BeginFill();
        }
        // if lineStyle was active we need to restore it too.
        if (s)
        {
            StrokeStyle = Shapes->AddStrokeStyle(mLineStyle);
            States |= State_FreshLineStyle;
        }
    }
}

}} // namespace Scaleform::GFx

