/**************************************************************************

Filename    :   GFx_MorphCharacter.cpp
Content     :   Morphable shape character implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_MorphCharacter.h"

#ifdef SF_RENDER_ENABLE_MORPHING
#include "GFx/GFx_Stream.h"
#include "Render/Render_Math2D.h"

#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_ImageResource.h"

#include <stdio.h>

namespace Scaleform { namespace GFx {

MorphCharacterDef::MorphCharacterDef()
{
}


MorphCharacterDef::~MorphCharacterDef()
{
}

void    MorphCharacterDef::ReadMorphFillStyle(LoadProcess* p, TagType tagType,
                                              FillStyleType& fs1, FillStyleType& fs2,
                                              bool* needResolve)
{
    Stream* in = p->GetStream();

    UInt8 type = in->ReadU8();

    in->LogParse("morph fill style type = 0x%X\n", type);

    if (type == Fill_Solid)
    {
        Color c1, c2;
        in->ReadRgba(&c1);
        in->ReadRgba(&c2);
        fs1.Color = c1.ToColor32();
        fs2.Color = c2.ToColor32();

        in->LogParse("morph fill style begin color: "); 
        in->LogParseClass(c1);
        in->LogParse("morph fill style end color: "); 
        in->LogParseClass(c2);
    }

    else if (type & Fill_Gradient_TestBit)
    {
        Matrix2F imageMatrix1, imageMatrix2;
        in->ReadMatrix(&imageMatrix1);
        in->ReadMatrix(&imageMatrix2);

        // convert swf matrix to texture UV one
        Matrix2F textureMatrix1, textureMatrix2;
        if (type == Fill_LinearGradient)
        {
            textureMatrix1.AppendScaling(1.0f / 32768.F);
            textureMatrix1.AppendTranslation(0.5F, 0.F);
            textureMatrix2.AppendScaling(1.0f / 32768.F);
            textureMatrix2.AppendTranslation(0.5F, 0.F);
        }
        else
        {
            textureMatrix1.AppendScaling(1.0f / 32768.F);
            textureMatrix1.AppendTranslation(0.5F, 0.5F);
            textureMatrix2.AppendScaling(1.0f / 32768.F);
            textureMatrix2.AppendTranslation(0.5F, 0.5F);
            // gradient's square size is 32768x32768 (-16,384 - 16,384)
        }
        textureMatrix1.Prepend(imageMatrix1.GetInverse());
        textureMatrix2.Prepend(imageMatrix2.GetInverse());

        // GRADIENT
        UByte  numGradients = in->ReadU8();
        bool   linearRGB    = 0;

        // For DefineMorphShape2 gradient records are interpreted differently.
        if ((tagType == Tag_DefineShapeMorph2) ||
            (type == Fill_FocalPointGradient))
        {
            if (numGradients & 0x10)
                linearRGB = 1;
            // TBD: Other bits may be set here, such as 0x80 for focal point
            // Do they need custom processing?
            numGradients &= 0xF;
        }

        SF_ASSERT(numGradients >= 1 && numGradients <= 15);

        fs1.pFill = *SF_NEW ComplexFill(); // heap? stat id? @TODO
        fs2.pFill = *SF_NEW ComplexFill(); // heap? stat id? @TODO
        GradientType gradType;
        switch (type)
        {
        case Fill_LinearGradient: gradType = GradientLinear; break;
        case Fill_RadialGradient: gradType = GradientRadial; break;
        case Fill_FocalPointGradient: gradType = GradientFocalPoint; break;
        default: gradType = GradientLinear; SF_ASSERT(0); //?
        }
        fs1.pFill->pGradient = *SF_NEW GradientData(gradType, numGradients, linearRGB);
        fs2.pFill->pGradient = *SF_NEW GradientData(gradType, numGradients, linearRGB);

        for (unsigned i = 0; i < numGradients; i++)
        {
            UInt8 ratio = p->ReadU8();
            Color color;
            p->ReadRgbaTag(&color, tagType);

            fs1.pFill->pGradient->At(i).ColorV = color;
            fs1.pFill->pGradient->At(i).Ratio  = ratio;

            ratio = p->ReadU8();
            p->ReadRgbaTag(&color, tagType);

            fs2.pFill->pGradient->At(i).ColorV = color;
            fs2.pFill->pGradient->At(i).Ratio  = ratio;
        }

        in->LogParse("morph fsr: numGradients = %d\n", numGradients);

        // SWF 8: Focal point location can morph, so read in those locations.
        if (type == Fill_FocalPointGradient)           
        {
            fs1.pFill->pGradient->SetFocalRatio((float) (in->ReadS16()) / 256.0f);
            fs2.pFill->pGradient->SetFocalRatio((float) (in->ReadS16()) / 256.0f);
        }

        fs1.pFill->ImageMatrix = textureMatrix1;
        fs2.pFill->ImageMatrix = textureMatrix2;
    }

    else if (type & Fill_Image_TestBit)
    {
        int bitmapCharId = in->ReadU16();
        in->LogParse("morph fsr BitmapChar = %d\n", bitmapCharId);

        Matrix2F imageMatrix1, imageMatrix2;
        in->ReadMatrix(&imageMatrix1);
        in->ReadMatrix(&imageMatrix2);

        // Look up the bitmap character.
        ResourcePtr<ImageResource> imageRes;

        // convert swf matrix to texture one
        // the matrix in the swf transforms bitmap's pixels into shape coordinates.
        // Renderer needs textureMatrix that transforms shape coordinates into texture ones.
        // Actually, texture coordinates should be in UV, but ShapeMeshProvider::GetFillMatrix
        // takes care about this scaling; here, we just need to get inverse matrix (that
        // transforms shape coords into bitmap's pixels).
        Matrix2F textureMatrix1(imageMatrix1.GetInverse());
        Matrix2F textureMatrix2(imageMatrix2.GetInverse());

        // When using scale9grid with gradients or images, 
        // the image matrix has to be adjusted according to the 
        // "logical difference" between the world matrix and the 
        // central part of the scale9grid. The s9gImgAdjust (if not null)
        // represents this adjustment matrix.
        //             imgMtx = ImageMatrix;
        //             if (s9gImgAdjust)
        //                 imgMtx.Append(*s9gImgAdjust);

        fs1.pFill = *SF_NEW ComplexFill(); // heap? stat id? @TODO
        fs1.pFill->ImageMatrix = textureMatrix1;
        switch( type )
        {
        case Fill_TiledImage:           fs1.pFill->FillMode.Fill = ( Wrap_Repeat | Sample_Point ); break;
        case Fill_TiledSmoothImage:     fs1.pFill->FillMode.Fill = ( Wrap_Repeat | Sample_Linear ); break;
        case Fill_ClippedImage:         fs1.pFill->FillMode.Fill = ( Wrap_Clamp | Sample_Point ); break;
        case Fill_ClippedSmoothImage:   fs1.pFill->FillMode.Fill = ( Wrap_Clamp | Sample_Linear ); break;
        }

        fs2.pFill = *SF_NEW ComplexFill(); // heap? stat id? @TODO
        fs2.pFill->ImageMatrix = textureMatrix2;
        fs2.pFill->FillMode.Fill = fs1.pFill->FillMode.Fill;

        if (!p->GetResourceHandle(&imageRes, ResourceId(bitmapCharId)) || !imageRes.GetResourcePtr())
        {
            // Should not happen..
            // New image ??
            fs1.Color = fs2.Color = 0xFFA7DD39; //???

            if (imageRes.IsIndex())
            {
                fs1.pFill->BindIndex = imageRes.GetBindIndex();
                fs2.pFill->BindIndex = imageRes.GetBindIndex();
                *needResolve = true;
            }
            else
            {
                fs1.pFill = fs2.pFill = 0;
                if (bitmapCharId != 0xFFFF) // weird value that is something like "new bitmap" flag
                {
                    LogDebugMessage(Log_Warning, "An image with resource id %d is not found in resource table.",
                        bitmapCharId);
                }

                //@TODO. FillStyle holds a pointer to Image, but in the case of dynamic binding (when KeepBindingData
                // flag is set) we can't get a pointer to the Image at the loading stage; instead, it is 
                // necessary to resolve it during GetFillStyle call of ShapeInterface interface.
                // If the index is detected as a resource then new ShapeMeshProvider should be created for such
                // shapes (do not use one from ShapeDef/do not create one in ShapeDef). The ShapeMeshProvider 
                // should contain info enough to resolve the resource by index (resource binding, defimpl). This
                // data should be passed to GetFillStyle/GetStrokeStyle methods. Fill style stored inside the shape
                // should contain a resource index instead of pImage pointer, but the methods will return 
                // a fillstyle with resolved image.
                // To avoid creation of separate ShapeMeshProviders for each instance of such shape we would need
                // to hold a hash table of all shapemeshproviders inside the defimpl: (ShapeInterface,ShapeMeshProvider) pair.
            }
        }
        else
        {
            if (imageRes.GetResourcePtr() && imageRes.GetResourcePtr()->GetResourceType() == Resource::RT_Image)
            {
                Image* pimage = static_cast<Image*>(static_cast<ImageResource*>(imageRes.GetResourcePtr())->GetImage());
                fs1.pFill->pImage = pimage;
                fs2.pFill->pImage = pimage;
            }
        }
    }
}

void    MorphCharacterDef::Read(LoadProcess* p, const TagInfo& tagInfo, bool withStyle)
{
    SF_UNUSED(withStyle);

    Stream* in = p->GetStream();

    RectF  bound1, bound2;
    RectF  rbound1, rbound2;
    in->ReadRect(&bound1);
    in->ReadRect(&bound2);

    // SWF 8 includes extra rectangle bounds.
    if (tagInfo.TagType == 84)
    {
        in->ReadRect(&rbound1);
        in->ReadRect(&rbound2);

        // MA: What does this byte do?
        // It normally takes on a value of 0x01.
        // Seems similar to a similar byte in tag 83 (DefinedShape5).
        //UByte mysteryByte = 
        in->ReadU8();
    }
    else
    {
        rbound1 = bound1;
        rbound2 = bound2;
    }

    Array<FillStyleType> shape1FillStyles, shape2FillStyles;

//     pShape1->SetBound(bound1);
//     pShape2->SetBound(bound2);
//     pShape1->SetRectBoundsLocal(rbound1);
//     pShape2->SetRectBoundsLocal(rbound2);

    unsigned endShapeOffset = in->ReadU32();
    int endShapeBase = in->Tell();

    unsigned fillStyleCount = in->ReadVariableCount();
    
    bool needResolve = false;
    unsigned i;
    for (i = 0; i < fillStyleCount; i++)
    {
        FillStyleType fs1, fs2;

        ReadMorphFillStyle(p, tagInfo.TagType, fs1, fs2, &needResolve);

        shape1FillStyles.PushBack(fs1);
        shape2FillStyles.PushBack(fs2);
    }

    Array<StrokeStyleType> shape1LineStyles, shape2LineStyles;
    unsigned lineStyleCount = in->ReadVariableCount();
    shape1LineStyles.Resize(lineStyleCount);
    shape2LineStyles.Resize(lineStyleCount);
    for (i = 0; i < lineStyleCount; i++)
    {
        StrokeStyleType& ls1 = shape1LineStyles[i];
        StrokeStyleType& ls2 = shape2LineStyles[i];

        ls1.Width = in->ReadU16();
        ls2.Width = in->ReadU16();
        ls1.Flags = ls2.Flags = 0;
        ls1.Units = ls2.Units = 0.05f;
        ls1.Miter = ls2.Miter = Default_MiterSize;

        // SWF 8 - includes line style.
        if (tagInfo.TagType == Tag_DefineShapeMorph2)
        {   
            // There is only one line style, since it cannot change in a morph.
            unsigned styleFlags = in->ReadU16();

            ls1.Flags = ls2.Flags = ConvertSwfLineStyles(styleFlags);

            // Miter is followed by a special 8.8 pixed point miter value.
            if (ls1.Flags & StrokeJoin_Miter)
            {
                ls1.Miter = ls2.Miter = (float)(in->ReadU16()) / 256.0f;
            }
        }

        if (ls1.Flags & StrokeFlag_ComplexFill)
        {
            FillStyleType fs1, fs2;
            ReadMorphFillStyle(p, tagInfo.TagType, fs1, fs2, &needResolve);

            ls1.Color = fs1.Color;
            ls1.pFill = fs1.pFill;

            ls2.Color = fs2.Color;
            ls2.pFill = fs2.pFill;

            if (fs1.pFill->pGradient)
            {
                GradientData *pgdata = fs1.pFill->pGradient;
                if (pgdata && pgdata->GetRecordCount() > 0)
                    ls1.Color = pgdata->At(0).ColorV.ToColor32();
            }
            if (fs2.pFill->pGradient)
            {
                GradientData *pgdata = fs2.pFill->pGradient;
                if (pgdata && pgdata->GetRecordCount() > 0)
                    ls2.Color = pgdata->At(0).ColorV.ToColor32();
            }
        }
        else
        {
            Color c1, c2;
            p->ReadRgbaTag(&c1, tagInfo.TagType);
            p->ReadRgbaTag(&c2, tagInfo.TagType);
            ls1.Color = c1.ToColor32();
            ls2.Color = c2.ToColor32();
        }
    }
    unsigned beginShapeAbsOffset = (unsigned)in->Tell();
    unsigned endShapeAbsOffset   = (unsigned)(endShapeBase + endShapeOffset);

    if (endShapeAbsOffset >= beginShapeAbsOffset)
    {
        in->LogParseShape("MorphCharacterDef, first shape:\n");

        pShape1 = *SF_HEAP_AUTO_NEW(this) ConstShapeWithStyles();
        pShape1->Read(p, tagInfo.TagType, endShapeAbsOffset - beginShapeAbsOffset, false);
        pShape1->SetStyles(fillStyleCount, shape1FillStyles.GetDataPtr(), lineStyleCount, shape1LineStyles.GetDataPtr());
        if (needResolve)
            pShape1->SetNeedsResolving();

        in->LogParseShape("MorphCharacterDef, second shape:\n");

        //in->Align();
        in->SetPosition(endShapeAbsOffset);
        pShape2 = *SF_HEAP_AUTO_NEW(this) ConstShapeWithStyles();
        pShape2->Read(p, tagInfo.TagType, tagInfo.TagDataOffset + tagInfo.TagLength - endShapeAbsOffset, false);
        pShape2->SetStyles(fillStyleCount, shape2FillStyles.GetDataPtr(), lineStyleCount, shape2LineStyles.GetDataPtr());
        if (needResolve)
            pShape2->SetNeedsResolving();

        SF_ASSERT(shape1FillStyles.GetSize() == shape2FillStyles.GetSize());
        SF_ASSERT(shape1LineStyles.GetSize() == shape2LineStyles.GetSize());
    }
    else
    {
        pShape1 = *SF_HEAP_AUTO_NEW(this) ConstShapeWithStyles(ConstShapeWithStyles::Empty_Shape);
        pShape2 = *SF_HEAP_AUTO_NEW(this) ConstShapeWithStyles(ConstShapeWithStyles::Empty_Shape);
    }
    pShapeMeshProvider = *SF_HEAP_AUTO_NEW(this) ShapeMeshProvider(pShape1, pShape2);
}

RectF MorphCharacterDef::GetBoundsLocal(float morphRatio) const
{
    SF_ASSERT(pShapeMeshProvider);
    //!AB: Flash doesn't track bounds changes during the morph,
    // therefore passing 'ratio' = 0.
    return pShapeMeshProvider->GetCorrectBounds(Matrix2F(), morphRatio, 0, 0);
}

bool  MorphCharacterDef::DefPointTestLocal(const Render::PointF &pt, bool testShape, 
                                           const DisplayObjectBase *pinst) const 
{ 
    Ptr<Render::Scale9GridInfo> s9g;

    if (pinst && pinst->DoesScale9GridExist())
    {
        s9g = *pinst->CreateScale9Grid();
    }

    if (!testShape)
    {
        //!AB: Flash doesn't track bounds changes during the morph,
        // therefore passing 'ratio' = 0.
        RectF bnd = pShapeMeshProvider->GetCorrectBounds(Matrix2F(), 0, 0, 0);

        if (s9g.GetPtr())
            bnd = s9g->AdjustBounds(bnd);
        return bnd.Contains(pt);
    }
    else
    {
        return pShapeMeshProvider->HitTestShape(Matrix2F(), pt.x, pt.y, pinst->GetRatio(), 0, 0, s9g);
    }
}

bool  MorphCharacterDef::IsEqualGeometry(const ShapeBaseCharacterDef& cmpWith) const 
{
    SF_UNUSED(cmpWith);
    SF_ASSERT(0); //@TODO??
    return false; 
}

Ptr<Render::ShapeMeshProvider> MorphCharacterDef::BindResourcesInStyles(const GFx::ResourceBinding& resourceBinding) const
{
    Ptr<ShapeDataBase> newSh1 = *pShape1->Clone();
    Ptr<ShapeDataBase> newSh2 = *pShape2->Clone();
    newSh1->BindResourcesInStyles(resourceBinding);
    newSh2->BindResourcesInStyles(resourceBinding);
    Ptr<Render::ShapeMeshProvider> shapeMeshProvider = *SF_HEAP_AUTO_NEW(this) ShapeMeshProvider(newSh1, newSh2);
    return shapeMeshProvider;
}

}} // namespace Scaleform::GFx
#endif //#ifdef SF_RENDER_ENABLE_MORPHING
