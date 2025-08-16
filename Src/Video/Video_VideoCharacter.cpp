/**************************************************************************

Filename    :   Video_VideoCharacter.cpp
Content     :   GFx video: VideoCharacterDef and VideoCharacter classes
Created     :   May, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoCharacter.h"

#ifdef GFX_ENABLE_VIDEO

#include "Render/Render_TreeShape.h"
#include "Render/Render_ShapeDataFloatMP.h"

#include "Video/Video_VideoImage.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

void VideoCharacterDef::Read(LoadProcess* p, TagType tagType)
{
    SF_ASSERT(tagType == Tag_DefineVideoStream);
    SF_UNUSED(tagType);

    Stream* pin = p->GetStream();

    NumFrames  = pin->ReadU16();
    Width      = pin->ReadU16();
    Height     = pin->ReadU16();

    unsigned reserve = pin->ReadUInt(5);
    SF_UNUSED(reserve);

    Deblocking = (UInt8)pin->ReadUInt(2);
    Smoothing  = pin->ReadUInt(1) ? true : false;
    CodecId    = pin->ReadU8();
}

//////////////////////////////////////////////////////////////////////////
//

VideoCharacter::VideoCharacter(VideoCharacterDef* def, MovieDefImpl *pbindingDefImpl,
                               ASMovieRootBase* pasroot, InteractiveObject* parent, ResourceId id) :
    InteractiveObject(pbindingDefImpl, pasroot, parent, id), pDef(def)
{
    SF_ASSERT(pDef);
    ViewRect = Render::RectF(0, 0, PixelsToTwips((float)pDef->Width),
                                   PixelsToTwips((float)pDef->Height));
}

Ptr<Render::TreeNode> VideoCharacter::CreateRenderNode(Render::Context& context) const
{
    const AvmVideoCharacterBase* pavm = GetAvmVideoCharacter();
    SF_ASSERT(pavm);

    Ptr<Render::TreeShape> shape;
    Ptr<Render::ShapeDataFloatMP> shapeData = *SF_HEAP_AUTO_NEW(this) Render::ShapeDataFloatMP();

    if (pTextureImage)
    {
        Render::FillStyleType fill;
        fill.pFill = *SF_HEAP_AUTO_NEW(this) Render::ComplexFill();
        fill.pFill->pImage = pTextureImage;
        fill.pFill->ImageMatrix.SetIdentity();
        fill.pFill->ImageMatrix.AppendScaling(0.05f);
        fill.pFill->ImageMatrix.AppendScaling(
            PixelsToTwips(pavm->PictureWidth)/ViewRect.Width(),
            PixelsToTwips(pavm->PictureHeight)/ViewRect.Height());
        if (pavm->Smoothing)
            fill.pFill->FillMode.Fill = (Render::Wrap_Clamp | Render::Sample_Linear);
        else
            fill.pFill->FillMode.Fill = (Render::Wrap_Clamp | Render::Sample_Point);

        shapeData->AddFillStyle(fill);
        shapeData->StartLayer();
        shapeData->StartPath(shapeData->GetFillStyleCount(), 0, 0);
        shapeData->RectanglePath(0.f, 0.0f, ViewRect.Width(), ViewRect.Height());
        shapeData->ClosePath();
        shapeData->CountLayers();
    }
    else 
    {
        shapeData->StartPath(0, 0, 0);
        shapeData->MoveTo(0, 0);
        shapeData->LineTo(1, 1);
        shapeData->ClosePath();
        shapeData->EndPath();
    }

    shape = *context.CreateEntry<Render::TreeShape>(shapeData);
    return shape;
}

Render::TreeNode* VideoCharacter::RecreateRenderNode() const
{
    UPInt index = ~0u;
    Render::TreeContainer* parent = NULL;
    bool nodeExisted = (pRenNode ? true : false);

    bool has3DMatrix = false;
    Matrix2F m2d;
    Matrix3F m3d;
    bool hasViewMatrix = false;
    Matrix3F vm;
    bool hasProjMatrix = false;
    Matrix4F pm;

    if (pRenNode && pRenNode->GetParent())
    {
        index = 0;
        parent = static_cast<Render::TreeContainer*>(pRenNode->GetParent());
        for (UPInt n = parent->GetSize(); index < n; index++)
        {
            if (parent->GetAt(index) == pRenNode)
                break;
        }

        has3DMatrix = pRenNode->Is3D();
        if (has3DMatrix)
        {
            m3d = pRenNode->M3D();
            hasViewMatrix = pRenNode->GetViewMatrix3D(&vm);
            hasProjMatrix = pRenNode->GetProjectionMatrix3D(&pm);
        }
        else 
            m2d = pRenNode->M2D();

        parent->Remove(index, 1);
    }
    pRenNode = NULL;

    if (nodeExisted)
    {
        if (!pRenNode)
        {
            pRenNode = CreateRenderNode(GetMovieImpl()->GetRenderContext());
            SF_ASSERT(pRenNode);
            pRenNode->SetVisible(IsVisibleFlagSet());

            if (has3DMatrix)
            {
                pRenNode->SetMatrix3D(m3d);
                if (hasViewMatrix) pRenNode->SetViewMatrix3D(vm);
                if (hasProjMatrix) pRenNode->SetProjectionMatrix3D(pm);
            }
            else
                pRenNode->SetMatrix(m2d);
        }
        if (parent)
        {
            SF_ASSERT(index != ~0u);
            parent->Insert(index, pRenNode);
        }
    }
    return pRenNode;
}

void VideoCharacter::ReleaseTexture()
{
    if (pTextureImage)
    {
        pTextureImage = NULL;
        RecreateRenderNode();
    }
}

void VideoCharacter::CreateTexture()
{
    AvmVideoCharacterBase* pavm = GetAvmVideoCharacter();
    SF_ASSERT(pavm);

    int width, height;
    Ptr<VideoImage> pimage;

    Ptr<VideoProvider> p = pVideoProvider;
    if (p && p->IsActive())
    {
        pimage = p->GetTexture(&width, &height);
        if (pimage)
        {
            pTextureImage = pimage;
            pavm->PictureWidth  = width;
            pavm->PictureHeight = height;
        }
    }
    RecreateRenderNode();
}

void VideoCharacter::AttachVideoProvider(VideoProvider* pprovider) 
{ 
    Ptr<VideoProvider> p = pVideoProvider;
    if (pprovider != p.GetPtr())
    {
        pVideoProvider = pprovider; 
        ReleaseTexture();
    }
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
