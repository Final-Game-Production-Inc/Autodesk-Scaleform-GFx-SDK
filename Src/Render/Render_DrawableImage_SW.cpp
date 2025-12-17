/**************************************************************************

PublicHeader:   Render
Filename    :   Render_DrawableImage_SW.cpp
Content     :   Commands and command queue implementation for DrawableImage
Created     :   January 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_DrawableImage_Queue.h"
#include "Render/Render_HAL.h"
#include "Kernel/SF_Random.h"

namespace Scaleform { namespace Render {


//--------------------------------------------------------------------

// Channel mapping tables for CopyChannel.

UByte ChannelIndexMap[9] = {
    0xFF,
    0,    // Red   = 1
    1,    // Green = 2
    0xFF,
    2,    // Blue  = 4
    0xFF,
    0xFF,
    0xFF,
    3     // Alpha = 8
};

UByte MapChannelIndex(DrawableImage::ChannelBits channel)
{
    if (channel <= DrawableImage::Channel_Alpha)
    {
		return ChannelIndexMap[channel];
    }
    return 0xFF;
}


void DICommand_Clear::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData**) const
{
	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

	UInt32 c = FillColor.ToColor32();
	
    for ( unsigned y = 0; y < dest.GetSize().Height; ++y )
    {
		dstSwiz.CacheScanline(y);
        for ( unsigned x = 0; x < dest.GetSize().Width; ++x )
        {
            dstSwiz.SetPixelInScanline(x, c);
        }
    }
}

bool DICommand_SourceRect::CalculateDestClippedRect( const ImageSize &srcSize, const ImageSize &destSize, const Rect<SInt32>& srcRect,
                                                            Rect<SInt32>& dstClippedRect, Point<SInt32>& delta ) const
{
    // General coordinate mapping:
    // dst(x,y)  = src(x,y) + destPt - SourceRect.TopLeft
    delta = DestPoint - srcRect.TopLeft();

    Rect<SInt32> srcImageRect(srcSize);
    Rect<SInt32> dstImageRect(destSize);
    Rect<SInt32> srcClippedRect;

    if (!srcImageRect.IntersectRect(&srcClippedRect, srcRect))
        return false;
    if (!(srcClippedRect + delta).IntersectRect(&dstClippedRect, dstImageRect))
        return false;
    return true;
}

void DICommand_ColorTransform::ExecuteSW(DICommandContext& context,
										 ImageData& dest,
										 ImageData** psrc) const
{
	const ImageData& src = *psrc[0];
	ImagePlane d, s;
	dest.GetPlane(0, &d);
	src.GetPlane(0, &s);

	Point<SInt32> delta;
	Rect<SInt32> dstClippedRect;

	if(!CalculateDestClippedRect(src, dest, SourceRect, dstClippedRect, delta))
	{
		return;
	}

	// If the image is not transparent, then the Cxform multiplies all components by the alpha offset + alpha multiplier.
	Cxform cx = Cx;
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

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
	ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &(*psrc[0]));

	SInt32 x, y;
	for (y = dstClippedRect.y1; y < dstClippedRect.y2; y++)
	{
		dstSwiz.CacheScanline(y);
		srcSwiz.CacheScanline(y - delta.y);

		for (x = dstClippedRect.x1; x < dstClippedRect.x2; x++)
		{
			Color sCol = srcSwiz.GetPixelInScanline(x - delta.x);

			if (!pSource->IsTransparent())
				sCol.SetAlpha(255);

			// Note: More or less a duplicate of the implementation in the Cxform class itself,
			// but requires rescaling by 256 rather than 255 in order to be pixel-exact. Using
			// Cxform.Transform causes the resulting colors to be off by 1 in
			// test_bitmapdata_colorTransform.swf.
			float rgbaM[4] = { ((float)sCol.GetRed()   / 255.0f) * cx.M[0][0],
							   ((float)sCol.GetGreen() / 255.0f) * cx.M[0][1],
							   ((float)sCol.GetBlue()  / 255.0f) * cx.M[0][2],
							   ((float)sCol.GetAlpha() / 255.0f) * cx.M[0][3] };
			Color  dCol(
				(UByte)Alg::Clamp<float>((rgbaM[0] + cx.M[1][0]) * 256.0f, 0, 255),
				(UByte)Alg::Clamp<float>((rgbaM[1] + cx.M[1][1]) * 256.0f, 0, 255),
				(UByte)Alg::Clamp<float>((rgbaM[2] + cx.M[1][2]) * 256.0f, 0, 255),
				(UByte)Alg::Clamp<float>((rgbaM[3] + cx.M[1][3]) * 256.0f, 0, 255) );

			if (!pImage->IsTransparent())
				dCol.SetAlpha(255);

			dstSwiz.SetPixelInScanline(x, dCol.ToColor32());
		}
	}
}

void DICommand_Compare::ExecuteSW(DICommandContext& context,
								  ImageData& dest,
								  ImageData** psrc) const
{
	const ImageData& src0 = *psrc[0];
	const ImageData& src1 = *psrc[1];
	ImagePlane d, s[2];
	dest.GetPlane(0, &d);
	src0.GetPlane(0, &s[0]);
	src1.GetPlane(0, &s[1]);

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
	ImageSwizzlerContext src0Swiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &(*psrc[0]));
	ImageSwizzlerContext src1Swiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &(*psrc[1]));

	for (unsigned y = 0; y < src0.GetSize().Height; ++y)
	{
		dstSwiz.CacheScanline(y);
		src0Swiz.CacheScanline(y);
		src1Swiz.CacheScanline(y);
		
		for (unsigned x = 0; x < src0.GetSize().Width; ++x)
		{
			Color s0Col = src0Swiz.GetPixelInScanline(x);
			Color s1Col = src1Swiz.GetPixelInScanline(x);

			UByte s0Alpha = pSource->IsTransparent() ? s0Col.GetAlpha() : 255;
			UByte s1Alpha = pImageCompare1->IsTransparent() ? s1Col.GetAlpha() : 255;

			UByte delta[4] = { static_cast<UByte>(s0Col.GetRed() - s1Col.GetRed()),
                               static_cast<UByte>(s0Col.GetGreen() - s1Col.GetGreen()),
                               static_cast<UByte>(s0Col.GetBlue()  - s1Col.GetBlue()),
                               static_cast<UByte>(s0Alpha - s1Alpha) };
			// If no color channels are different, return white color with alpha difference only
			// If any color channel is different, return the difference with opaque alpha
			if(delta[0] == 0 && delta[1] == 0 && delta[2] == 0 && delta[3] != 0)
			{
				delta[0] = 0xff;
				delta[1] = 0xff;
				delta[2] = 0xff;
			}
			else
			{
				delta[3] = 0xff;
			}

			Color dCol(delta[0], delta[1], delta[2], delta[3]);
			
			if (!pImage->IsTransparent())
				delta[3] = 0xff;

			dstSwiz.SetPixelInScanline(x, dCol.ToColor32());
		}
	}
}

void DICommand_CopyChannel::ExecuteSW(DICommandContext& context,
                                      ImageData& dest,
                                      ImageData** psrc) const
{
    const ImageData& src = *psrc[0];
    ImagePlane d, s;
    dest.GetPlane(0, &d);
    src.GetPlane(0, &s);

	Point<SInt32> delta;
	Rect<SInt32> dstClippedRect;

	if(!CalculateDestClippedRect(src, dest, SourceRect, dstClippedRect, delta))
	{
		return;
	}
    
	// Iterate & Copy
    SInt32 x, y;
    UByte sCI = MapChannelIndex(SourceChannel);
    UByte dCI = MapChannelIndex(DestChannel);

    if ((sCI == 0xFF) || (dCI == 0xFF))
        return;

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
	ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &(*psrc[0]));

    for (y = dstClippedRect.y1; y < dstClippedRect.y2; y++)
    {
        dstSwiz.CacheScanline(y);
        srcSwiz.CacheScanline(y - delta.y);

		for (x = dstClippedRect.x1; x < dstClippedRect.x2; x++)
		{
			Color dCol = dstSwiz.GetPixelInScanline(x);
			Color sCol = srcSwiz.GetPixelInScanline(x - delta.x);
			
			UByte dChannels[4] = { dCol.GetRed(), dCol.GetGreen(), dCol.GetBlue(), dCol.GetAlpha() };
			UByte sChannels[4] = { sCol.GetRed(), sCol.GetGreen(), sCol.GetBlue(), sCol.GetAlpha() };
			
			if (!pSource->IsTransparent())
				sChannels[3] = 255;

			dChannels[dCI] = sChannels[sCI];

			if (!pImage->IsTransparent())
				dChannels[3] = 255;

			dCol.SetRGBA(dChannels[0], dChannels[1], dChannels[2], dChannels[3]);
			
			dstSwiz.SetPixelInScanline(x, dCol.ToColor32());
		}
    }
}

void DICommand_CopyPixels::ExecuteSW(DICommandContext& context,
                                     ImageData& dest,
                                     ImageData** psrc) const
{
    ImageData& src = *psrc[0];

    Rect<SInt32>  dstClippedRect;
    Point<SInt32> delta, deltaAlpha;
    if (!CalculateDestClippedRect(src, dest, SourceRect, dstClippedRect, delta))
        return;

    bool alphaImage = pAlphaSource != 0;
    ImageData& alphaSrc = alphaImage ? *psrc[1] : *psrc[0];
    int x, y;
    if (alphaImage)
    {        
        if (!CalculateDestClippedRect(alphaSrc, dest, Rect<SInt32>(AlphaPoint, SourceRect.GetSize()), 
            dstClippedRect, deltaAlpha))
            return;
    }
    else
    {
        deltaAlpha = delta;
    }

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
	ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &(*psrc[0]));
	ImageSwizzlerContext alphaSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), alphaImage ? &alphaSrc : &(*psrc[0]));

	for (y = dstClippedRect.y1; y < dstClippedRect.y2; y++)
    {
        dstSwiz.CacheScanline(y);
        srcSwiz.CacheScanline(y - delta.y);
        alphaSwiz.CacheScanline(y - deltaAlpha.y);

        for (x = dstClippedRect.x1; x < dstClippedRect.x2; x++)
        {
            Color s = srcSwiz.GetPixelInScanline(x - delta.x);
			
			if (!pSource->IsTransparent())
				s.SetAlpha(255);

            Color alpha = alphaImage ? alphaSwiz.GetPixelInScanline(x - deltaAlpha.x) : s;
            Color d = dstSwiz.GetPixelInScanline(x);
            Color final = s;

			UByte alphaIn = s.GetAlpha();
			SInt32 secAlpha = (alphaImage ? (alpha.GetAlpha()+1) : 256);
			UByte srcAlpha = (UByte)((alphaIn * secAlpha) >> 8);
			UByte dstAlpha = d.GetAlpha();
			UByte outAlpha = srcAlpha;
			if(MergeAlpha)
			{
				outAlpha = (UByte)Alg::Lerp<UByte, float>(srcAlpha, 0xFF, dstAlpha/255.0f);
			}
			if(!pImage->IsTransparent())
			{
				outAlpha = 255;
			}
			
			final = Color::Blend(d, s, (float)srcAlpha / (float)outAlpha);
            final.SetAlpha(outAlpha);

            dstSwiz.SetPixelInScanline(x, final.ToColor32());
        }
    }
}


void DICommand_FillRect::ExecuteSW(DICommandContext& context,
                                   ImageData& dest,
                                   ImageData**) const
{
	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
    Rect<SInt32> dstImageRect(dest.GetSize());
    Rect<SInt32> clippedRect;
    if (!dstImageRect.IntersectRect(&clippedRect, ApplyRect))
        return;

	UInt32 fillColor = FillColor.ToColor32();

	if (!pImage->IsTransparent())
        fillColor |= (UInt32)255 << 24;

	// Iterate & Copy
    int x, y;
    for (y = clippedRect.y1; y < clippedRect.y2; y++)
    {
		dstSwiz.CacheScanline(y);

        for (x = clippedRect.x1; x < clippedRect.x2; x++)
        {
			dstSwiz.SetPixelInScanline(x, fillColor);
        }
    }
}

void DICommand_FloodFill::ExecuteSW(DICommandContext& context,
                                    ImageData& dest,
                                    ImageData**) const
{
    // Check whether the floodfill origin is even within the bounds.
    Rect<SInt32> imageRect = Rect<SInt32>(dest.GetSize());
    if ( !imageRect.Contains(Pt))
        return;

	UInt32 fillColor = FillColor.ToColor32();
	if (!pImage->IsTransparent())
        fillColor |= (UInt32)255 << 24;

    ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
    ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

    srcSwiz.CacheScanline(Pt.y);
    Color overwriteColor = srcSwiz.GetPixelInScanline(Pt.x);
    Array<Point<SInt32> > toProcess;
    toProcess.PushBack(Point<SInt32>(Pt.x,Pt.y));
    while (!toProcess.IsEmpty())
    {
        const Point<SInt32> pt = toProcess.Back();
        toProcess.PopBack();
        Rect<SInt32> intersection;
        imageRect.IntersectRect(&intersection, Rect<SInt32>(pt,Size<SInt32>(1,1)));
        if ( intersection.Area() > 0 )
        {
            srcSwiz.CacheScanline(pt.y);
            if( srcSwiz.GetPixelInScanline(pt.x) == overwriteColor )
            {
                dstSwiz.CacheScanline(pt.y);
                dstSwiz.SetPixelInScanline(pt.x, fillColor);

                // Generate 4 cardinal directions to continue fill.
                toProcess.PushBack(Point<SInt32>(pt.x-1, pt.y));
                toProcess.PushBack(Point<SInt32>(pt.x, pt.y-1));
                toProcess.PushBack(Point<SInt32>(pt.x, pt.y+1));
                toProcess.PushBack(Point<SInt32>(pt.x+1, pt.y));
            }
        }
    }
}

void DICommand_GetColorBoundsRect::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData**) const
{
    ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

    // This could be much improved by doing a more intelligent search.
    bool foundAny = false;
    Point<SInt32> min = Point<SInt32>(dest.GetSize().Width, dest.GetSize().Height), max = Point<SInt32>(0,0);
    for ( unsigned y = 0; y < dest.GetSize().Height; ++y )
    {
        dstSwiz.CacheScanline(y);
        for ( unsigned x = 0; x < dest.GetSize().Width; ++x )
        {
            Color c = dstSwiz.GetPixelInScanline(x);
            if ( (FindColor && ( (c.ToColor32() & Mask) == SearchColor)) ||
                 (!FindColor && ( (c.ToColor32() & Mask) != SearchColor )))
            {
                min.x = Alg::Min<SInt32>(min.x, x);
                min.y = Alg::Min<SInt32>(min.y, y);
                max.x = Alg::Max<SInt32>(max.x, x+1);
                max.y = Alg::Max<SInt32>(max.y, y+1);
                foundAny = true;
            }
        }
    }

    // If we didn't find any, make sure to return a zero rectangle.
    if (Result)
    {
        if ( foundAny )
            *Result = Rect<SInt32>(min, max);
        else
            *Result = Rect<SInt32>(Size<SInt32>(0,0));
    }
}

void DICommand_GetPixel32::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData**) const
{
    ImageSwizzlerContext mappedSwizzler = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
    mappedSwizzler.CacheScanline(Y);
    if (Result)
        *Result = mappedSwizzler.GetPixelInScanline(X);
}

void DICommand_GetPixels::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData**) const
{
    ImageSwizzlerContext mappedSwizzler = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

    // TODO: Bounds checking?
    int x, y;
    for (y = SourceRect.y1; y < SourceRect.y2; y++)
    {
        mappedSwizzler.CacheScanline(y);
        for (x = SourceRect.x1; x < SourceRect.x2; x++)
        {
            Provider.WriteNextPixel(mappedSwizzler.GetPixelInScanline(x).ToColor32());
        }
    }
}

void DICommand_Histogram::ExecuteSW(DICommandContext& context, ImageData& dst, ImageData**) const
{
    ImageSwizzlerContext mappedSwizzler = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dst);
    for ( SInt32 y = Alg::Max<SInt32>(0, SourceRect.y1); y < Alg::Min<SInt32>(dst.GetSize().Height, SourceRect.y2); ++y )
    {
        mappedSwizzler.CacheScanline(y);
        for ( SInt32 x = Alg::Max<SInt32>(0, SourceRect.x1); x < Alg::Min<SInt32>(dst.GetSize().Width, SourceRect.x2); ++x )
        {            
            Color c = mappedSwizzler.GetPixelInScanline(x);
            UInt32 uc = c.ToColor32();
            Result[512 + (uc & 0xFF)]++;
            uc >>= 8;
            Result[256 + (uc & 0xFF)]++;
            uc >>= 8;
            Result[0   + (uc & 0xFF)]++;
            uc >>= 8;
            Result[768 + (uc & 0xFF)]++;
        }
    }
}

void DICommand_HitTest::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** psrc) const
{
    if (!SecondImage)
    {
        ImageSwizzlerContext imgSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

        Size<SInt32> compareDimensions(
            Alg::Min<SInt32>(dest.GetSize().Width, SecondArea.Width()),
            Alg::Min<SInt32>(dest.GetSize().Height, SecondArea.Height()));

        for ( SInt32 y = Alg::Max<int>(0, SecondArea.y1 - FirstPoint.y); y < SecondArea.y1 - FirstPoint.y + compareDimensions.Height; ++y )
        {
            if ( y < 0 || y >= (SInt32)dest.GetHeight())
                break;
            imgSwiz.CacheScanline(y);
            for ( SInt32 x = Alg::Max<int>(0, + SecondArea.x1 - FirstPoint.x); x < + SecondArea.x1 - FirstPoint.x + compareDimensions.Width; ++x )
            {
                if (x < 0 || x >= (SInt32)dest.GetWidth())
                    break;
                Color c0 = imgSwiz.GetPixelInScanline(x);
                if ( c0.GetAlpha() < FirstThreshold )
                    continue;

                if (Result)
                    *Result = true;
                return;
            }
        }
    }
    else
    {
        ImageSwizzlerContext img0Swiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
        ImageSwizzlerContext img1Swiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), psrc[0]);

        Size<SInt32> compareDimensions(
            Alg::Min(dest.GetSize().Width, SecondImage->GetSize().Width),
            Alg::Min(dest.GetSize().Height, SecondImage->GetSize().Height));

        for ( SInt32 y0 = Alg::Max<int>(0, SecondPoint.y - FirstPoint.y); y0 < SecondPoint.y - FirstPoint.y + compareDimensions.Height; ++y0 )
        {
            SInt32 y1 = y0 - SecondPoint.y + FirstPoint.y;
            if ( y0 < 0 || y0 >= (SInt32)dest.GetHeight() ||
                y1 < 0 || y1 >= (SInt32)psrc[0]->GetHeight() )
                break;

            img0Swiz.CacheScanline(y0);
            img1Swiz.CacheScanline(y1);

            for ( SInt32 x0 = Alg::Max<int>(0, + SecondPoint.x - FirstPoint.x); x0 < + SecondPoint.x - FirstPoint.x + compareDimensions.Width; ++x0 )
            {
                SInt32 x1 = x0 - SecondPoint.x + FirstPoint.x;
                if ( x0 < 0 || x0 >= (SInt32)dest.GetWidth() ||
                    x1 < 0 || x1 >= (SInt32)psrc[0]->GetWidth() )
                    break;
                Color c0 = img0Swiz.GetPixelInScanline(x0);
                if ( c0.GetAlpha() < FirstThreshold )
                    continue;
                Color c1 = img1Swiz.GetPixelInScanline(x1);
                if ( c1.GetAlpha() < SecondThreshold )
                    continue;

                if (Result)
                    *Result = true;
                return;
            }
        }
    }

    if (Result)
        *Result = false;
}

void DICommand_Merge::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** psrc) const
{
	const ImageData& src = *psrc[0];
	ImagePlane d, s;
	dest.GetPlane(0, &d);
	src.GetPlane(0, &s);

	Point<SInt32> delta;
	Rect<SInt32> dstClippedRect;

	if(!CalculateDestClippedRect(src, dest, SourceRect, dstClippedRect, delta))
	{
		return;
	}

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
	ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &(*psrc[0]));

	SInt32 x, y, channel;
	for (y = dstClippedRect.y1; y < dstClippedRect.y2; y++)
	{
		dstSwiz.CacheScanline(y);
		srcSwiz.CacheScanline(y - delta.y);

		for (x = dstClippedRect.x1; x < dstClippedRect.x2; x++)
		{
			Color dCol = dstSwiz.GetPixelInScanline(x);
			Color sCol = srcSwiz.GetPixelInScanline(x - delta.x);
			UInt32 factors[4] = { RedMultiplier, GreenMultiplier, BlueMultiplier, AlphaMultiplier };
			UInt32 sChan[4] = { sCol.GetRed(), sCol.GetGreen(), sCol.GetBlue(), sCol.GetAlpha() };
			UInt32 dChan[4] = { dCol.GetRed(), dCol.GetGreen(), dCol.GetBlue(), dCol.GetAlpha() };
			UByte bChan[4];

			if (!pSource->IsTransparent())
				sChan[3] = 255;
			if (!pImage->IsTransparent())
				dChan[3] = 255;

			for(channel = 0; channel < 4; channel++)
			{
				bChan[channel] = (UByte)((sChan[channel] * factors[channel] + dChan[channel] * (256 - factors[channel])) >> 8);
			}

			if (!pImage->IsTransparent())
				bChan[3] = 255;

			Color blendedCol(bChan[0], bChan[1], bChan[2], bChan[3]);
			dstSwiz.SetPixelInScanline(x, blendedCol.ToColor32());
		}
	}
}

//---------------------------------------------------------------------------------------
void DICommand_Noise::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData**) const
{
    Alg::Random::Generator rng;
    rng.SeedRandom(RandomSeed);

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

    for (unsigned x = 0; x < dest.GetSize().Width; ++x)
    {
        for (unsigned y = 0; y < dest.GetSize().Height; ++y)
        {
            dstSwiz.CacheScanline(y);

            Color c(0);
            if ( GrayScale )
            {
                float val = rng.GetUnitFloat();
                c.SetRGBAFloat(val, val, val, ChannelMask & DrawableImage::Channel_Alpha ? rng.GetUnitFloat() : 1.0f);
            }
            else
            {
                if ( ChannelMask & DrawableImage::Channel_Red )
                    c.SetRed((UByte)(rng.GetUnitFloat()*255));
                if ( ChannelMask & DrawableImage::Channel_Green )
                    c.SetGreen((UByte)(rng.GetUnitFloat()*255));
                if ( ChannelMask & DrawableImage::Channel_Blue )
                    c.SetBlue((UByte)(rng.GetUnitFloat()*255));
                if ( ChannelMask & DrawableImage::Channel_Alpha )
                    c.SetAlpha((UByte)(rng.GetUnitFloat()*255));
                else
                    c.SetAlpha(255);
            }
			
			if (!pImage->IsTransparent())
				c.SetAlpha(255);

            dstSwiz.SetPixelInScanline(x, c.ToColor32());
        }
    }
}

void DICommand_PaletteMap::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** psrc) const
{
	const ImageData& src = *psrc[0];
	ImagePlane d, s;
	dest.GetPlane(0, &d);
	src.GetPlane(0, &s);

	Point<SInt32> delta;
	Rect<SInt32> dstClippedRect;

	if(!CalculateDestClippedRect(src, dest, SourceRect, dstClippedRect, delta))
	{
		return;
	}

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
	ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &(*psrc[0]));

	SInt32 x, y, channel;
	for (y = dstClippedRect.y1; y < dstClippedRect.y2; y++)
	{
		dstSwiz.CacheScanline(y);
		srcSwiz.CacheScanline(y - delta.y);

		for (x = dstClippedRect.x1; x < dstClippedRect.x2; x++)
		{
			Color sCol = srcSwiz.GetPixelInScanline(x - delta.x);
			
			if (!pSource->IsTransparent())
				sCol.SetAlpha(255);

			// Retrieve source channels
			UByte rgba[4];
			UInt32 outCols[4];
			UInt32 total = 0;
			sCol.GetRGBA(rgba + 0, rgba + 1, rgba + 2, rgba + 3);
			
			outCols[3] = rgba[3] << 24;
			outCols[0] = rgba[0] << 16;
			outCols[1] = rgba[1] << 8;
			outCols[2] = rgba[2];
			
			// Remap channels as necessary
			for(channel = 0; channel < 4; channel++)
			{
				if(ChannelMask & (1 << channel))
				{
					outCols[channel] = Channels[channel*(ChannelSize/sizeof(UInt32)) + rgba[channel]];
				}
				total += outCols[channel];
			}

			if (!pImage->IsTransparent())
                total |= (UInt32)255 << 24;

			Color  dCol(total);

			dstSwiz.SetPixelInScanline(x, dCol.ToColor32());
		}
	}
}

// PerlineGenerator is dervied from: http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
class PerlinGenerator
{
public:
    PerlinGenerator(unsigned seed, unsigned channel) 
    {
        PrimeSet = NoisePrimeFactors[(seed * 173 + channel * 157) %8];
    }

	float TurbulenceNoise(float x, float y)
	{
		return Alg::Abs(InterpolatedNoise(x, y)) - 0.5f;
	}

    float InterpolatedNoise(float x, float y)
    {
        int integer_X    = int(x);
        float fractional_X = x - integer_X;

        int integer_Y    = int(y);
        float fractional_Y = y - integer_Y;

        float v1 = SmoothNoise(integer_X,     integer_Y);
        float v2 = SmoothNoise(integer_X + 1, integer_Y);
        float v3 = SmoothNoise(integer_X,     integer_Y + 1);
        float v4 = SmoothNoise(integer_X + 1, integer_Y + 1);

        float i1 = CosInterpolate(v1 , v2 , fractional_X);
        float i2 = CosInterpolate(v3 , v4 , fractional_X);

        return CosInterpolate(i1 , i2 , fractional_Y);
    }

private:
    struct NoisePrimeSet
    {
        unsigned primes[4];
    };
    NoisePrimeSet PrimeSet;
    static const NoisePrimeSet NoisePrimeFactors[];

    float Noise(int x, int y)
    {
        int n = x + y * PrimeSet.primes[0];
        n = (n<<13) ^ n;
        return ( 1.0f - ( (n * (n * n * PrimeSet.primes[1] + PrimeSet.primes[2]) + 
            PrimeSet.primes[3]) & 0x7fffffff) / 1073741824.0f);
    }

    float SmoothNoise(int x, int y)
    {
        float corners = ( Noise(x-1, y-1)+Noise(x+1, y-1)+Noise(x-1, y+1)+Noise(x+1, y+1) ) / 16;
        float sides   = ( Noise(x-1, y)  +Noise(x+1, y)  +Noise(x, y-1)  +Noise(x, y+1) ) /  8;
        float center  =  Noise(x, y) / 4;
        return corners + sides + center;
    }

	float LinearInterpolate(float a, float b, float x)
	{
		return a*(1.0f-x) + b*x;
	}

    float CosInterpolate(float a, float b, float x)
    {
        float ft = x * 3.1415927f;
        float f = (float)((1 - cos(ft)) * .5f);
        return a*(1-f) + b*f;
    }
};

const PerlinGenerator::NoisePrimeSet PerlinGenerator::NoisePrimeFactors[] =
{
    {{ 67, 19937, 178757, 7978189}},
    {{ 97, 21701, 100511, 2200591}},
    {{ 73, 23209, 499321, 9218263}},
    {{ 19, 44497, 422861, 6200351}},
    {{ 59, 43261, 300589, 9299317}},
    {{ 17, 33461, 475333, 1200809}},
    {{ 23, 96557, 457117, 6267011}},
    {{ 43, 28657, 199669, 2299553}}
};

void DICommand_PerlinNoise::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData**) const
{
	unsigned channelMask = ChannelMask;

	// If GrayScale is set, only generate the red channel, then duplicate it to green and blue.
	// If alpha channel is requested, it is still generated separately.
	if ( GrayScale )
	{
		channelMask &= ~(DrawableImage::Channel_RGB);
		channelMask |=  DrawableImage::Channel_Red;
	}

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

	for ( unsigned y = 0; y < dest.GetSize().Height; ++y )
	{
		dstSwiz.CacheScanline(y);
		for ( unsigned x = 0; x < dest.GetSize().Width; ++x )
		{
			Color color(0xFF000000);
			unsigned channelCount = pImage->IsTransparent() ? 4 : 3;
			for ( unsigned channel = 0; channel < channelCount; ++ channel )
			{
				if ( (channelMask & 1<<channel ) == 0 )
					continue;
				PerlinGenerator generator(RandomSeed, channel);

				// Compute all the octaves/channels for the particular pixel.
                float freqX = 2.0f / FrequencyX;
                float freqY = 2.0f / FrequencyY;
                float amplitude = 1.0f;
                float noise = 0.0f;
                float fmax  = 0.0f;
                for ( unsigned octave = 0; octave < NumOctaves; ++octave )
                {
					float xPos = x*freqX;
					float yPos = y*freqY;
					if(octave < OffsetCount)
					{
						xPos += Offsets[octave*2+0];
						yPos += Offsets[octave*2+1];
					}
                    noise += (1.0f + generator.InterpolatedNoise(xPos, yPos)) * 0.5f * amplitude;
                    freqX *= 2.0f;
                    freqY *= 2.0f;
                    fmax += amplitude;
                    amplitude /= 2.0f;
                }
                noise /= fmax;

                if ( GrayScale && channel != 3)
                {
                    color.SetRGBFloat(noise,noise,noise);
                }
                else
                {
                    switch(channel)
                    {
                    default:
                    case 0: color.SetRed((UByte)(255*noise)); break;
                    case 1: color.SetGreen((UByte)(255*noise)); break;
                    case 2: color.SetBlue((UByte)(255*noise)); break;
                    case 3: color.SetAlpha((UByte)(255*noise)); break;
                    }
                }
			}
			
			if (!pImage->IsTransparent())
				color.SetAlpha(255);

			// Now actually set the pixel
			dstSwiz.SetPixelInScanline(x, color.ToColor32());
        }
    }
}

// PixelDisolve
//---------------------------------------------------------------------------------------
// http://en.wikipedia.org/wiki/Linear_feedback_shift_register
class LFSR
{
private:
    static const unsigned FeedbackPoly[];

    // The maximum value desired as an output.
    unsigned MaximumOutput;
    unsigned FeedbackIndex;
public:
    LFSR(unsigned maximum) : MaximumOutput(maximum)
    {
        FeedbackIndex = 0;
        while (MaximumOutput > (unsigned)(1<<FeedbackIndex))
            FeedbackIndex++;
    }

    unsigned Next(int lfsr)
    {
        // Simply throwaway results that are past the maximum.
        do
        {
            lfsr = (lfsr >> 1) ^ (-(lfsr & 1) & FeedbackPoly[FeedbackIndex]);
        } while ((unsigned)lfsr > MaximumOutput);
        return (unsigned)lfsr;
    }
};

const unsigned LFSR::FeedbackPoly[] =
{
    0x00000001u,    // 1-bit
    0x00000003u,    // 2-bit
    0x00000006u,    // 3-bit
    0x0000000Cu,    // 4-bit
    0x00000014u,    // 5-bit
    0x00000030u,    // 6-bit
    0x00000060u,    // 7-bit
    0x000000B8u,    // 8-bit
    0x00000110u,    // 9-bit
    0x00000240u,    // 10-bit
    0x00000500u,    // 11-bit
    0x00000E08u,    // 12-bit
    0x00001C80u,    // 13-bit
    0x00003802u,    // 14-bit
    0x00006000u,    // 15-bit
    0x0000B400u,    // 16-bit
    0x00009000u,    // 17-bit
    0x00020400u,    // 18-bit
    0x00072000u,    // 19-bit
    0x00090000u,    // 20-bit (1024x1024)
    0x00140000u,    // 21-bit
    0x00300000u,    // 22-bit (2048x2048)
    0x00420000u,    // 23-bit
    0x00E10000u     // 24-bit (16.7m pixels - maximum BitmapData size)
};


void DICommand_PixelDissolve::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** psrc) const
{
    ImageSwizzlerContext destSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

    // Behavior is different if the source and destination are the same image.
    if ( pImage == pSource )
    {
        Rect<SInt32> dstImageRect = Rect<SInt32>(dest.GetSize());
        Rect<SInt32> clippedRect;
        if (!dstImageRect.IntersectRect(&clippedRect, SourceRect))
        {
            Result = 0;
            return;
        }
        LFSR pixelGen(clippedRect.Area());

        unsigned pixelIndex = RandomSeed;
        for ( unsigned i = 0; i < NumPixels; ++i )
        {
            pixelIndex = pixelGen.Next(pixelIndex);
            destSwiz.CacheScanline((pixelIndex-1)/clippedRect.Width());
            destSwiz.SetPixelInScanline((pixelIndex-1)%clippedRect.Width(), Fill.ToColor32());
        }
        if (Result)
            *Result = pixelIndex;
    }
    else
    {
        ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), psrc[0]);

        Rect<SInt32> dstImageRect = Rect<SInt32>(DestPoint.x, DestPoint.y,
            dest.GetSize().Width-DestPoint.x, dest.GetSize().Height-DestPoint.y);
        if ( dstImageRect.Width() <= 0 || dstImageRect.Height() <= 0)
        {
            Result = 0;
            return;
        }
        Rect<SInt32> srcClippedRect;
        if (!SourceRect.IntersectRect(&srcClippedRect, Rect<SInt32>(psrc[0]->GetSize())))
        {
            Result = 0;
            return;
        }
        Size<SInt32> dimensions(
            Alg::Min(srcClippedRect.Width(), dstImageRect.Width()),
            Alg::Min(srcClippedRect.Height(), dstImageRect.Height()));

        LFSR pixelGen(dimensions.Area());
        unsigned pixelIndex = RandomSeed;
        for ( unsigned i = 0; i < NumPixels; ++i )
        {
            pixelIndex = pixelGen.Next(pixelIndex);
            srcSwiz.CacheScanline(SourceRect.y1 + (pixelIndex-1)/dimensions.Width);
            Color color = srcSwiz.GetPixelInScanline(SourceRect.x1 + (pixelIndex-1)%dimensions.Width);
            destSwiz.CacheScanline(DestPoint.y + (pixelIndex-1)/dimensions.Width);
            destSwiz.SetPixelInScanline(DestPoint.x + (pixelIndex-1)%dimensions.Width, color.ToColor32());
        }
        if (Result)
            *Result = pixelIndex;
    }
}
void DICommand_Scroll::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** psrc) const
{
	const ImageData& src = *psrc[0];
	ImagePlane d, s;
	dest.GetPlane(0, &d);
	src.GetPlane(0, &s);

	// General coordinate mapping:
	// dst(x,y)  = src(x,y) + destPt - SourceRect.TopLeft
	Point<SInt32> delta = DestPoint - SourceRect.TopLeft();
	//delta.x += X;
	//delta.y += Y;

	Rect<SInt32> srcImageRect(src.GetSize());
	Rect<SInt32> dstImageRect(dest.GetSize());
	Rect<SInt32> srcClippedRect, dstClippedRect;

	if (!srcImageRect.IntersectRect(&srcClippedRect, SourceRect))
		return;
	if (!(srcClippedRect + delta).IntersectRect(&dstClippedRect, dstImageRect))
		return;

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(),&dest);
	ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(),&(*psrc[0]));

	// Iterate & Copy
	SInt32 x, y;
	for (y = dstClippedRect.y2 - 1; y >= dstClippedRect.y1; y--)
	{
		dstSwiz.CacheScanline(y);
		srcSwiz.CacheScanline(y - delta.y);

		for (x = dstClippedRect.x2 - 1; x >= dstClippedRect.x1; x--)
		{
			Color sCol = srcSwiz.GetPixelInScanline(x - delta.x);

			if (!pSource->IsTransparent() || !pImage->IsTransparent())
				sCol.SetAlpha(255);

			dstSwiz.SetPixelInScanline(x, sCol.ToColor32());
		}
	}
}

void DICommand_SetPixel32::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData**) const
{
    ImageSwizzlerContext imgSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
    imgSwiz.CacheScanline(Y);
    Color c = Fill;
    if (!OverwriteAlpha)
        c.SetAlpha(imgSwiz.GetPixelInScanline(X).GetAlpha());
    imgSwiz.SetPixelInScanline(X, c.ToColor32());
}

void DICommand_SetPixels::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData**) const
{
    ImageSwizzlerContext imgSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);

    // Could be more efficient
    unsigned pixelCount = 0;
    for ( int y = DestRect.y1; y < DestRect.y2; ++y )
    {
        imgSwiz.CacheScanline(y);
        for ( int x = DestRect.x1; x < DestRect.x2; ++x, ++pixelCount )
        {
            if ( pixelCount >= Provider.GetLength() )
            {
                if(Result)
                    *Result = false;
                return;
            }
            imgSwiz.SetPixelInScanline(x,Provider.ReadNextPixel());
        }
    }
    if(Result)
        *Result = true;
}
void DICommand_Threshold::ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** psrc) const
{
	const ImageData& src = *psrc[0];
	ImagePlane d, s;
	dest.GetPlane(0, &d);
	src.GetPlane(0, &s);

	Point<SInt32> delta;
	Rect<SInt32> dstClippedRect;

	if(!CalculateDestClippedRect(src, dest, SourceRect, dstClippedRect, delta))
	{
		return;
	}

	ImageSwizzlerContext dstSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &dest);
	ImageSwizzlerContext srcSwiz = ImageSwizzlerContext(context.pHAL->GetTextureManager()->GetImageSwizzler(), &(*psrc[0]));

	// Iterate & Copy
	SInt32 x, y;
	for (y = dstClippedRect.y1; y < dstClippedRect.y2; y++)
	{
		dstSwiz.CacheScanline(y);
		srcSwiz.CacheScanline(y - delta.y);

		for (x = dstClippedRect.x1; x < dstClippedRect.x2; x++)
		{
			Color sCol = srcSwiz.GetPixelInScanline(x - delta.x);
			UInt32 rVal = Threshold & Mask;
			UInt32 lVal = sCol.ToColor32() & Mask;

			bool compareSuccess = false;
			
			switch(Operation)
			{
			case DrawableImage::Operator_EQ:
				compareSuccess = (lVal == rVal);
				break;
			case DrawableImage::Operator_GE:
				compareSuccess = (lVal >= rVal);
				break;
			case DrawableImage::Operator_GT:
				compareSuccess = (lVal > rVal);
				break;
			case DrawableImage::Operator_LE:
				compareSuccess = (lVal <= rVal);
				break;
			case DrawableImage::Operator_LT:
				compareSuccess = (lVal < rVal);
				break;
			case DrawableImage::Operator_NE:
				compareSuccess = (lVal != rVal);
				break;
			}

			if (!pSource->IsTransparent())
				sCol.SetAlpha(255);

			UInt32 outColor = compareSuccess ? ThresholdColor : sCol.ToColor32();
			
			if (!pImage->IsTransparent())
                outColor |= (UInt32)255 << 24;

			dstSwiz.SetPixelInScanline(x, outColor);
		}
	}
}

}}; // namespace Scaleform::Render

