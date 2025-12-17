/**********************************************************************

PublicHeader:   Render
Filename    :   Render_DrawableImage_HW.cpp
Content     :   Hardware versions of commands for DrawableImage
Created     :   Feb 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/Render_DrawableImage_Queue.h"
#include "Render/Render_HAL.h"
#include "Render/Renderer2DImpl.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { namespace Render {

//---------------------------------------------------------------------------------------
void DICommand_CreateTexture::ExecuteHW(DICommandContext& context) const
{
    TextureManager* tmanager = context.pHAL->GetTextureManager();
    SF_ASSERT(tmanager);
    // TODO: there is no error checking that this actually succeeded
    pImage->createTextureFromManager(context.pHAL, tmanager);
}

//---------------------------------------------------------------------------------------
void DICommand_Clear::ExecuteHW(DICommandContext& context) const
{
    context.pHAL->applyBlendMode(Blend_OverwriteAll, true, true);
    context.pHAL->clearSolidRectangle(RectF(pImage->GetSize()), FillColor, false);
    context.pHAL->applyBlendMode(Blend_None);
}

//---------------------------------------------------------------------------------------
void DICommand_ApplyFilter::ExecuteHWGetImages( DrawableImage** images, Size<float>* readOffsets) const
{
    unsigned index = 0;
    // Unconditionally get the source image. We actually read from it, however, because filters
    // will be done within render targets already, we do not report that we are reading from it.
    images[index] = pImage;
    readOffsets[index] = Size<float>((float)DestPoint.x, (float)DestPoint.y);
    index++;
    images[index] = pSource;
    readOffsets[index] = Size<float>((float)SourceRect.TopLeft().x, (float)SourceRect.TopLeft().y);
}

void DICommand_ApplyFilter::ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen) const
{
    // Position the MVP for the filter primitive.
    Matrix2F mvp, mvpContent;

    RectF filterRect, sourceRect = SourceRect;
    DrawableImage::CalcFilterRect( &filterRect, PixelsToTwips(sourceRect), pFilter );
    filterRect = TwipsToPixels(filterRect);
    mvp.AppendScaling(filterRect.GetSize());
    mvp.AppendTranslation(filterRect.x1, filterRect.y1);

    FilterSet filters(pFilter);
    Ptr<FilterPrimitive> prim = *SF_NEW FilterPrimitive(&filters, false);
    MatrixPool& mpool = context.pR2D->GetImpl()->GetMatrixPool();
    HMatrix hm = mpool.CreateMatrix(mvp);
    hm.SetMatrix2D(mvp);
    prim->Insert(0, hm);

    // Push the filter primitive.
    context.pHAL->PushFilters(prim);

    // Draw the original content to the centre of the filter render target.
    mvpContent = Matrix2F::Scaling(2, context.pHAL->GetViewportScaling() * 2) * Matrix2F::Translation(-0.5f, -0.5f);
    mvpContent.AppendScaling( sourceRect.GetSize() / filterRect.GetSize() );
    context.pHAL->DrawableCopyback(tex[1], mvpContent, texgen[1]);

    // 
    context.pHAL->PopFilters();
}

//---------------------------------------------------------------------------------------
void DICommand_ColorTransform::ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const
{
    context.pHAL->applyBlendMode(pImage->IsTransparent() ? Blend_OverwriteAll : Blend_Overwrite, true, true);
    Cxform cx = Cx;

    // If the image is not transparent, then the Cxform multiplies all components by the alpha offset + alpha multiplier.
    if ( !pImage->IsTransparent() )
    {
        float falpha = cx.M[0][3] + cx.M[1][3];
        for ( int i = 0; i < 3; ++i)
        {
            cx.M[0][i] *= falpha;
            cx.M[1][i] *= falpha;
        }
        cx.M[0][3] = 1.0f;
        cx.M[1][3] = 0.0f;
    }

    // Skip the source image, it is both pImage and pSource.
    context.pHAL->DrawableCxform(tex+1, texgen+1, &cx);
}

//---------------------------------------------------------------------------------------
void DICommand_Draw::ExecuteHW(DICommandContext& context) const
{
    DrawableImageContext* pdicontext = pImage->GetContext();
    SF_DEBUG_ASSERT(pdicontext != 0, "Internal error - DrawableImageContext should survive until after all executed Draw commands.");

    // Ensure that NextCapture has been called. If it has already been called, then this 
    // will quickly return the displaying snapshot.
    pdicontext->ExecuteNextCapture(context.pR2D->GetContextNotify());

    // If we are currently in a display, we must end it and restart another one for Draw.
    bool wasInDisplay = false;
    Viewport oldVP = context.pHAL->VP;
    if (context.pHAL->GetHALState() & HAL::HS_InDisplay)
    {
        wasInDisplay = true;
        context.pHAL->EndDisplay();
    }

    // Must flush here, to ensure everything in the previous Display has executed.
    context.pHAL->Flush();

    context.pHAL->SetDisplayPass(Display_All);
    context.pHAL->applyBlendMode(Blend_Normal, true, true);
    context.pR2D->Display(pRoot);
    
    // Must flush here, to ensure nothing is queued from this Display.
    context.pHAL->Flush();

    // If we were in an display bracket, restart it.
    if (wasInDisplay)
        context.pHAL->BeginDisplay(Color::Alpha0, oldVP);

    pdicontext->AddTreeRootToKillList(pRoot);
}

void DICommand_Draw::ExecuteDiscard()
{
    DrawableImageContext* pdicontext = pImage->GetContext();
    SF_DEBUG_ASSERT(pdicontext != 0, "Internal error - DrawableImageContext should survive until after all commands are discarded.");
    pdicontext->AddTreeRootToKillList(pRoot);
}

//---------------------------------------------------------------------------------------
template<class D>
void DICommand_SourceRectImpl<D>::ExecuteHW(DICommandContext& context) const
{
    TextureManager* pmanager = context.pHAL->GetTextureManager();
    DrawableImage* images[MaximumDrawableSources];
    Texture* ptextures[MaximumDrawableSources];
    RenderTarget* prts[MaximumDrawableSources];
    Size<float> readOffsets[MaximumDrawableSources];

    // Get the images/RTs.
    memset(images, 0, sizeof(images));
    ExecuteHWGetImages(images, readOffsets);
    unsigned source = 0;
    for ( source = 0; source < MaximumDrawableSources; ++source )
    {
        prts[source] = images[source] ? images[source]->GetRenderTarget() : 0;
        ptextures[source] = images[source] ? images[source]->GetTexture(pmanager) : 0;
    }


    Rect<SInt32> destClippedRect;
    Point<SInt32> delta;
    CalculateDestClippedRect(pSource->GetSize(), pImage->GetSize(), SourceRect, destClippedRect, delta );

    bool forcePOT = !pmanager->IsNonPow2Supported(pmanager->GetDrawableImageFormat(), ImageUse_RenderTarget|ImageUse_MapRenderThread);
#if defined(SF_OS_IPHONE) || defined(SF_OS_ANDROID)
    forcePOT = true;
#endif
    
    // Create a temporary target, we will need to render using the source image's data, and can't bind an output to an input.
    // Note that it only must be the size of the sourceRect. We must copy the results back to the original texture.
    bool requiresSource = GetRequireSourceRead();
    Ptr<RenderTarget> prt = 0;
    if ( requiresSource )
    {
        ImageSize imageSize(destClippedRect.Width(), destClippedRect.Height());
		if(forcePOT)
		{
            imageSize.SetWidth(Alg::UpperPowerOfTwo(imageSize.Width));
			imageSize.SetHeight(Alg::UpperPowerOfTwo(imageSize.Height));
		}
        prt = *context.pHAL->CreateTempRenderTarget(imageSize, false);
        context.pHAL->PushRenderTarget(RectF(imageSize), prt, HAL::PRT_Resolve);
    }

    // Setup the texgen matrices
    Matrix2F texgen[MaximumDrawableSources];
    for ( source = 0; source < MaximumDrawableSources; ++source )
    {
        if ( !ptextures[source])
            continue;
        texgen[source].AppendScaling(((Size<float>)destClippedRect.GetSize()) / ptextures[source]->GetSize());
        Size<float> offset = readOffsets[source] / ptextures[source]->GetSize();
        texgen[source].AppendTranslation(offset.Width, offset.Height);
    }

    // Setup parameters and call the HAL's drawing function.
    ExecuteHWCopyAction(context, ptextures, texgen);
    if ( requiresSource )
    {
        context.pHAL->PopRenderTarget(HAL::PRT_Resolve);

        // Copy the results back to the original image.
        Matrix2F mvp, texgenCopy;
		Size<float> imageSize = destClippedRect.GetSize();
		if(forcePOT)
		{
            imageSize.SetWidth((float)Alg::UpperPowerOfTwo((UInt32)imageSize.Width));
            imageSize.SetHeight((float)Alg::UpperPowerOfTwo((UInt32)imageSize.Height));
		}
        
        mvp.AppendScaling(imageSize / prts[0]->GetSize() );
        Size<float> copyOffset = (readOffsets[0]-((Size<float>)prts[0]->GetSize())/2.0f) / prts[0]->GetSize();
        mvp.AppendTranslation(copyOffset.Width, copyOffset.Height);
        mvp.AppendScaling(2.0f, context.pHAL->GetViewportScaling() * 2);
        texgenCopy.AppendScaling(((Size<float>)prt->GetSize()) / prt->GetBufferSize());
        context.pHAL->DrawableCopyback(prt->GetTexture(), mvp, texgenCopy );
        prt->SetInUse(RTUse_Unused);
    }
}

// Explicit template instantiations to allow us to define template member in the .cpp.
template struct DICommand_SourceRectImpl<DICommand_ApplyFilter>;
template struct DICommand_SourceRectImpl<DICommand_ColorTransform>;
template struct DICommand_SourceRectImpl<DICommand_Compare>;
template struct DICommand_SourceRectImpl<DICommand_CopyChannel>;
template struct DICommand_SourceRectImpl<DICommand_CopyPixels>;
template struct DICommand_SourceRectImpl<DICommand_Merge>;
template struct DICommand_SourceRectImpl<DICommand_PaletteMap>;
template struct DICommand_SourceRectImpl<DICommand_Scroll>;
template struct DICommand_SourceRectImpl<DICommand_Threshold>;

//---------------------------------------------------------------------------------------
void DICommand_CopyChannel::ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const
{
    Matrix4F cxmul[2];

    // Figure out the channel indices
    int destChannelIndex = 0, srcChannelIndex = 0;
    bool valid = true;
    switch( DestChannel )
    {
    case DrawableImage::Channel_Red:   destChannelIndex = 0; break;
    case DrawableImage::Channel_Green: destChannelIndex = 1; break;
    case DrawableImage::Channel_Blue:  destChannelIndex = 2; break;
    case DrawableImage::Channel_Alpha: destChannelIndex = 3; break;
    default: 
        SF_DEBUG_WARNING1(0, "DrawableImage.CopyChannel - Unexpected DestChannel value (%d)", DestChannel);
        valid = false;
        break;
    }
    switch( SourceChannel )
    {
    case DrawableImage::Channel_Red:   srcChannelIndex = 0; break;
    case DrawableImage::Channel_Green: srcChannelIndex = 1; break;
    case DrawableImage::Channel_Blue:  srcChannelIndex = 2; break;
    case DrawableImage::Channel_Alpha: srcChannelIndex = 3; break;
    default: 
        SF_DEBUG_WARNING1(0, "DrawableImage.CopyChannel - Unexpected SourceChannel value (%d)", SourceChannel);
        valid = false;
        break;
    }

    // For the original image, use the identity, and mask out the destination channel.
    cxmul[0].M[destChannelIndex][destChannelIndex] = 0;

    // From the source, start with zero, put the source channel into the destination channel.
    memset(cxmul[1].M, 0, sizeof(cxmul[1].M));
    if (valid)
        cxmul[1].M[destChannelIndex][srcChannelIndex] = 1.0f;

    // Do the copy.
    context.pHAL->applyBlendMode(pImage->IsTransparent() ? Blend_OverwriteAll : Blend_Overwrite, true, true);
    context.pHAL->DrawableCopyChannel(tex, texgen, cxmul);
}

//---------------------------------------------------------------------------------------
void DICommand_CopyPixels::ExecuteHWGetImages( DrawableImage** images, Size<float>* readOffsets) const
{
    unsigned index = 0;
    images[index] = pImage;
    readOffsets[index++] = Size<float>((float)DestPoint.x, (float)DestPoint.y);
    images[index] = pSource;
    readOffsets[index++] = Size<float>((float)SourceRect.TopLeft().x, (float)SourceRect.TopLeft().y);
    images[index] = pAlphaSource;
    readOffsets[index++] = Size<float>((float)AlphaPoint.x, (float)AlphaPoint.y);
}

void DICommand_CopyPixels::ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const
{
    Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
	context.pHAL->applyBlendMode(pImage->IsTransparent() ? Blend_OverwriteAll : Blend_Overwrite, true, true);
    context.pHAL->DrawableCopyPixels(tex, texgen, mvp, MergeAlpha, pImage->IsTransparent());
}

//---------------------------------------------------------------------------------------
void DICommand_FillRect::ExecuteHW(DICommandContext& context) const
{
    context.pHAL->applyBlendMode(Blend_OverwriteAll, true, true);

    // If the image is not transparent, set the fill color to full alpha
    Color fill = FillColor;
    if ( !pImage->IsTransparent() )
        fill.SetAlpha(0xFF);

    context.pHAL->clearSolidRectangle(ApplyRect, fill, false);
}

//---------------------------------------------------------------------------------------
void DICommand_Compare::ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const
{
    Matrix2F mvp;
    mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
    tex[0] = pSource->GetRenderTarget()->GetTexture();
    tex[1] = pImageCompare1->GetRenderTarget()->GetTexture();
    context.pHAL->DrawableCompare( tex, texgen );
}

//---------------------------------------------------------------------------------------
void DICommand_Merge::ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const
{
    Matrix4F cxmul[2];
    float mult[4];
    mult[0] = Alg::Clamp<unsigned>(RedMultiplier, 0, 256)   / 256.0f;
    mult[1] = Alg::Clamp<unsigned>(GreenMultiplier, 0, 256) / 256.0f;
    mult[2] = Alg::Clamp<unsigned>(BlueMultiplier, 0, 256)  / 256.0f;
    mult[3] = Alg::Clamp<unsigned>(AlphaMultiplier, 0, 256) / 256.0f;
    for ( int i = 0; i < 4; ++i )
    {
        cxmul[0].M[i][i] = 1.0f - mult[i];
        cxmul[1].M[i][i] = mult[i];
    }

    // Do the copy.
    context.pHAL->applyBlendMode(pImage->IsTransparent() ? Blend_OverwriteAll : Blend_Overwrite, true, true);
    context.pHAL->DrawableMerge(tex, texgen, cxmul);
}

//---------------------------------------------------------------------------------------
void DICommand_PaletteMap::ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const
{
    Matrix2F mvp;

    // If we are rendering to a render target, then we simply render to the whole target. Otherwise, we
    // need to position the output in the destination image properly.
    if ( GetRequireSourceRead() )
    {
        mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
    }
    else
    {
        mvp.AppendScaling((Size<float>)SourceRect.GetSize() / pImage->GetSize());
        Size<float> copyOffset = (Size<float>((float)DestPoint.x, (float)DestPoint.y)-((Size<float>)pImage->GetSize())/2.0f) / pImage->GetSize();
        mvp.AppendTranslation(copyOffset.Width, copyOffset.Height);
        mvp.AppendScaling(2.0f, -2.0f);
    }

    context.pHAL->DrawablePaletteMap(tex, texgen, mvp, ChannelMask, (const UInt32*)Channels);
}

//---------------------------------------------------------------------------------------
void DICommand_Scroll::ExecuteHWCopyAction( DICommandContext& context, Render::Texture**tex, const Matrix2F* texgen) const
{
    Matrix2F mvp = Matrix2F::Scaling(2,context.pHAL->GetViewportScaling() * 2) * Matrix2F::Translation(-0.5f, -0.5f);
    context.pHAL->applyBlendMode(pImage->IsTransparent() ? Blend_OverwriteAll : Blend_Overwrite, true, true);
    context.pHAL->DrawableCopyback(tex[0], mvp, texgen[1] );
}

//---------------------------------------------------------------------------------------
void DICommand_Threshold::ExecuteHWCopyAction( DICommandContext&, Render::Texture**, const Matrix2F*) const
{
    SF_DEBUG_ASSERT(0, "Not Implemented.");
}

}} // Scaleform::Render
