/**************************************************************************

Filename    :   Render_ResizeImage.cpp
Content     :   Bitmap Image resizing with filtering
Created     :   2007
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

----------------------------------------------------------------------
The code of these classes was taken from the Anti-Grain Geometry
Project and modified for the use by Scaleform/Autodesk. 
Permission to use without restrictions is hereby granted to 
Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
See http://antigrain.com for details.
**************************************************************************/

#include "Render_ResizeImage.h"

namespace Scaleform { namespace Render {

//--------------------------------------------------------------------
void ImageFilterLut::reallocLut(float radius)
{
    Radius = radius;
    Diameter = unsigned(ceilf(radius)) * 2;
    Start = -int(Diameter / 2 - 1);
    unsigned size = Diameter << ImgSubpixelShift;
    if (size > WeightArray.GetSize())
        WeightArray.Resize(size);
}



//--------------------------------------------------------------------
// This function normalizes integer values and corrects the rounding 
// errors. It corrects the filter values according to the rule 
// of 1.0 which means that any sum of pixel weights must be equal to 1.0.
// So, the filter function must produce a graph of the proper shape.
//--------------------------------------------------------------------
void ImageFilterLut::Normalize()
{
    unsigned i;
    int flip = 1;

    for(i = 0; i < ImgSubpixelScale; i++)
    {
        for(;;)
        {
            int sum = 0;
            unsigned j;
            for(j = 0; j < Diameter; j++)
            {
                sum += WeightArray[j * ImgSubpixelScale + i];
            }

            if(sum == ImgFilterScale) break;

            float k = float(ImgFilterScale) / float(sum);
            sum = 0;
            for(j = 0; j < Diameter; j++)
            {
                sum +=                 WeightArray[j * ImgSubpixelScale + i] = 
                    SInt16(Alg::IRound(WeightArray[j * ImgSubpixelScale + i] * k));
            }

            sum -= ImgFilterScale;
            int inc = (sum > 0) ? -1 : 1;

            for(j = 0; j < Diameter && sum; j++)
            {
                flip ^= 1;
                unsigned idx = flip ? Diameter/2 + j/2 : Diameter/2 - j/2;
                int v = WeightArray[idx * ImgSubpixelScale + i];
                if(v < ImgFilterScale)
                {
                    WeightArray[idx * ImgSubpixelScale + i] = 
                        SInt16(WeightArray[idx * ImgSubpixelScale + i] + inc);
                    sum += inc;
                }
            }
        }
    }

    unsigned pivot = Diameter << (ImgSubpixelShift - 1);

    for(i = 0; i < pivot; i++)
    {
        WeightArray[pivot + i] = WeightArray[pivot - i];
    }
    unsigned end = (Diameter << ImgSubpixelShift) - 1;
    WeightArray[0] = WeightArray[end];
}


//------------------------------------------------------------------------
static SF_INLINE void PixelFilterBoxGray8(UByte* pDst, const UByte* pSrc)
{
    *pDst = *pSrc;
}

//------------------------------------------------------------------------
static SF_INLINE void PixelFilterBoxRGB24(UByte* pDst, const UByte* pSrc)
{
    pDst[0] = pSrc[0];
    pDst[1] = pSrc[1];
    pDst[2] = pSrc[2];
}

//------------------------------------------------------------------------
static SF_INLINE void PixelFilterBoxRGBA32(UByte* pDst, const UByte* pSrc)
{
    pDst[0] = pSrc[0];
    pDst[1] = pSrc[1];
    pDst[2] = pSrc[2];
    pDst[3] = pSrc[3];
}


//------------------------------------------------------------------------
static SF_INLINE void PixelFilterBoxRGBtoRGBA32(UByte* pDst, const UByte* pSrc)
{
    pDst[0] = pSrc[0];
    pDst[1] = pSrc[1];
    pDst[2] = pSrc[2];
    pDst[3] = 255;
}


//------------------------------------------------------------------------
template<class FilterFunction>
static void 
ImageResizeBox(      UByte* pDst, int dstWidth, int dstHeight, int dstPitch, int dstBpp,
               const UByte* pSrc, int srcWidth, int srcHeight, int srcPitch, int srcBpp,
               FilterFunction filter)
{
    Scaleform::ArrayUnsafe<int> srcCoordX;
    srcCoordX.Resize(dstWidth);
    int x, y;
    int offset = ImgSubpixelOffset * srcWidth / dstWidth;

    // Prepare the X-interpolator and fill the interpolation array for faster access
    LinearInterpolator ix(offset, 
                           offset + (srcWidth << ImgSubpixelShift),
                           dstWidth);

    for(x = 0; x < dstWidth; x++)
    {
        srcCoordX[x] = ix.y();
        ++ix;
    }

    // Prepare the Y-interpolator
    offset = ImgSubpixelOffset * srcHeight / dstHeight;
    LinearInterpolator iy(offset, 
                           offset + (srcHeight << ImgSubpixelShift),
                           dstHeight);

    for(y = 0; y < dstHeight; y++)
    {
        int xInt;
        int yInt = iy.y() >> ImgSubpixelShift;
        if(yInt < 0)          yInt = 0;
        if(yInt >= srcHeight) yInt = srcHeight - 1;
        const UByte* pSrcRow = pSrc + srcPitch * yInt;
              UByte* pDstRow = pDst + dstPitch * y;

        // Filter left pixels, while the filtering window 
        // is out of the left bound of the source image.
        for(x = 0; x < dstWidth; x++)
        {
            xInt = srcCoordX[x] >> ImgSubpixelShift;
            if(xInt >= 0) break;
            filter(pDstRow, pSrcRow);
            pDstRow += dstBpp;
        }

        // Filter central pixels, while the filtering window 
        // is fully visible in the source image.
        int srcWidth1 = srcWidth - 1;
        for(; x < dstWidth; x++)
        {
            xInt = srcCoordX[x] >> ImgSubpixelShift;
            if(xInt >= srcWidth1) break;
            filter(pDstRow, pSrcRow + xInt * srcBpp);
            pDstRow += dstBpp;
        }

        // Filter right pixels, when the filtering window 
        // is out of the right bound of the source image.
        pSrcRow += (srcWidth - 1) * srcBpp;
        for(; x < dstWidth; x++)
        {
            filter(pDstRow, pSrcRow);
            pDstRow += dstBpp;
        }
        ++iy;
    }
}


//------------------------------------------------------------------------
void SF_STDCALL ResizeImageBox(UByte* pDst, 
                              int dstWidth, int dstHeight, int dstPitch,
                              const UByte* pSrc, 
                              int srcWidth, int srcHeight, int srcPitch,
                              ResizeImageType type)
{
    if (dstWidth <= 0 || dstHeight <= 0 ||
        srcWidth <= 0 || srcHeight <= 0) return;

    switch(type)
    {
    case ResizeRgbToRgb:
        ImageResizeBox(pDst, dstWidth, dstHeight, dstPitch, 3,
                       pSrc, srcWidth, srcHeight, srcPitch, 3,
                       PixelFilterBoxRGB24);
        break;

    case ResizeRgbaToRgba: 
        ImageResizeBox(pDst, dstWidth, dstHeight, dstPitch, 4,
                       pSrc, srcWidth, srcHeight, srcPitch, 4,
                       PixelFilterBoxRGBA32);
        break;

    case ResizeRgbToRgba:
        ImageResizeBox(pDst, dstWidth, dstHeight, dstPitch, 4,
                       pSrc, srcWidth, srcHeight, srcPitch, 3,
                       PixelFilterBoxRGBtoRGBA32);
        break;

    case ResizeGray:
        ImageResizeBox(pDst, dstWidth, dstHeight, dstPitch, 1,
                       pSrc, srcWidth, srcHeight, srcPitch, 1,
                       PixelFilterBoxGray8);
        break;

    case ResizeNone:
        break;
    }
}



//------------------------------------------------------------------------
static SF_INLINE void PixelFilterBilinearRGBA32(UByte* pDst, 
                                                const UByte* pSrc1,
                                                const UByte* pSrc2,
                                                const UByte* pSrc3,
                                                const UByte* pSrc4,
                                                int xFract, int yFract)
{
    int dst[4] = { ImgSubpixelInitial, ImgSubpixelInitial, 
                   ImgSubpixelInitial, ImgSubpixelInitial };
    int weight;

    weight = (ImgSubpixelScale - xFract) * (ImgSubpixelScale - yFract);
    dst[0] += weight * pSrc1[0];
    dst[1] += weight * pSrc1[1];
    dst[2] += weight * pSrc1[2];
    dst[3] += weight * pSrc1[3];

    weight = xFract * (ImgSubpixelScale - yFract);
    dst[0] += weight * pSrc2[0];
    dst[1] += weight * pSrc2[1];
    dst[2] += weight * pSrc2[2];
    dst[3] += weight * pSrc2[3];

    weight = (ImgSubpixelScale - xFract) * yFract;
    dst[0] += weight * pSrc3[0];
    dst[1] += weight * pSrc3[1];
    dst[2] += weight * pSrc3[2];
    dst[3] += weight * pSrc3[3];

    weight = xFract * yFract;
    dst[0] += weight * pSrc4[0];
    dst[1] += weight * pSrc4[1];
    dst[2] += weight * pSrc4[2];
    dst[3] += weight * pSrc4[3];

    pDst[0] = (UByte)(dst[0] >> ImgSubpixelShift2);
    pDst[1] = (UByte)(dst[1] >> ImgSubpixelShift2);
    pDst[2] = (UByte)(dst[2] >> ImgSubpixelShift2);
    pDst[3] = (UByte)(dst[3] >> ImgSubpixelShift2);
}


//------------------------------------------------------------------------
static SF_INLINE void PixelFilterBilinearRGB24(UByte* pDst, 
                                               const UByte* pSrc1,
                                               const UByte* pSrc2,
                                               const UByte* pSrc3,
                                               const UByte* pSrc4,
                                               int xFract, int yFract)
{
    int dst[3] = { ImgSubpixelInitial, ImgSubpixelInitial, ImgSubpixelInitial };
    int weight;

    weight = (ImgSubpixelScale - xFract) * (ImgSubpixelScale - yFract);
    dst[0] += weight * pSrc1[0];
    dst[1] += weight * pSrc1[1];
    dst[2] += weight * pSrc1[2];

    weight = xFract * (ImgSubpixelScale - yFract);
    dst[0] += weight * pSrc2[0];
    dst[1] += weight * pSrc2[1];
    dst[2] += weight * pSrc2[2];

    weight = (ImgSubpixelScale - xFract) * yFract;
    dst[0] += weight * pSrc3[0];
    dst[1] += weight * pSrc3[1];
    dst[2] += weight * pSrc3[2];

    weight = xFract * yFract;
    dst[0] += weight * pSrc4[0];
    dst[1] += weight * pSrc4[1];
    dst[2] += weight * pSrc4[2];

    pDst[0] = (UByte)(dst[0] >> ImgSubpixelShift2);
    pDst[1] = (UByte)(dst[1] >> ImgSubpixelShift2);
    pDst[2] = (UByte)(dst[2] >> ImgSubpixelShift2);
}


//------------------------------------------------------------------------
static SF_INLINE void PixelFilterBilinearRGBtoRGBA32(UByte* pDst, 
                                                     const UByte* pSrc1,
                                                     const UByte* pSrc2,
                                                     const UByte* pSrc3,
                                                     const UByte* pSrc4,
                                                     int xFract, int yFract)
{
    PixelFilterBilinearRGB24(pDst, pSrc1, pSrc2, pSrc3, pSrc4, xFract, yFract);
    pDst[3] = 255;
}


//------------------------------------------------------------------------
static SF_INLINE void PixelFilterBilinearGray8(UByte* pDst, 
                                               const UByte* pSrc1,
                                               const UByte* pSrc2,
                                               const UByte* pSrc3,
                                               const UByte* pSrc4,
                                               int xFract, int yFract)
{
    *pDst = (UByte)((ImgSubpixelInitial +
        *pSrc1 * (ImgSubpixelScale - xFract) * (ImgSubpixelScale - yFract) +
        *pSrc2 *                     xFract  * (ImgSubpixelScale - yFract) +
        *pSrc3 * (ImgSubpixelScale - xFract) *                     yFract  +
        *pSrc4 *                     xFract  *                     yFract) >> ImgSubpixelShift2);
}




//------------------------------------------------------------------------
template<class FilterFunction>
static void 
ImageResizeFilter2x2(      UByte* pDst, int dstWidth, int dstHeight, int dstPitch, int dstBpp,
                     const UByte* pSrc, int srcWidth, int srcHeight, int srcPitch, int srcBpp,
                     FilterFunction filter)
{
    Scaleform::ArrayUnsafe<int> srcCoordX;
    srcCoordX.Resize(dstWidth);
    int x, y;
    int offset = ImgSubpixelOffset * srcWidth / dstWidth;

    // Prepare the X-interpolator and fill the interpolation array for faster acceess
    LinearInterpolator ix(offset, 
                           offset + (srcWidth << ImgSubpixelShift),
                           dstWidth);

    for(x = 0; x < dstWidth; x++)
    {
        srcCoordX[x] = ix.y() - ImgSubpixelOffset;
        ++ix;
    }

    // Prepare the Y-interpolator
    offset = ImgSubpixelOffset * srcHeight / dstHeight;
    LinearInterpolator iy(offset, 
                           offset + (srcHeight << ImgSubpixelShift),
                           dstHeight);

    UByte* pDst2 = pDst;

    for(y = 0; y < dstHeight; y++)
    {
        int yFract = iy.y() - ImgSubpixelOffset;
        int yInt   = yFract >> ImgSubpixelShift;
        yFract    &= ImgSubpixelMask;

        const UByte* pSrcRow1 = (yInt < 0) ? pSrc : pSrc + srcPitch * yInt;
        ++yInt;
        if(yInt >= srcHeight) yInt = srcHeight - 1;
        const UByte* pSrcRow2 = pSrc + srcPitch * yInt;

        UByte* pDst3 = pDst2;
        int xInt, xFract, x1, x2;

        // Filter left pixels, while the filtering window 
        // is out of the left bound of the source image.
        for(x = 0; x < dstWidth; x++)
        {
            xFract  = srcCoordX[x];
            xInt    = xFract >> ImgSubpixelShift;
            xFract &= ImgSubpixelMask;
            if(xInt >= 0) break;
            filter(pDst3, 
                   pSrcRow1, pSrcRow1, 
                   pSrcRow2, pSrcRow2, 
                   xFract, yFract);
            pDst3 += dstBpp;
        }

        // Filter central pixels, while the filtering window 
        // is fully visible in the source image.
        int srcWidth1 = srcWidth - 1;
        for(; x < dstWidth; x++)
        {
            xFract  = srcCoordX[x];
            xInt    = xFract >> ImgSubpixelShift;
            xFract &= ImgSubpixelMask;
            if(xInt >= srcWidth1) break;
            x1 =  xInt      * srcBpp;
            x2 = (xInt + 1) * srcBpp;
            filter(pDst3, 
                   pSrcRow1 + x1, pSrcRow1 + x2, 
                   pSrcRow2 + x1, pSrcRow2 + x2, 
                   xFract, yFract);
            pDst3 += dstBpp;
        }

        // Filter right pixels, when the filtering window 
        // is out of the right bound of the source image.
        pSrcRow1 += (srcWidth - 1) * srcBpp;
        pSrcRow2 += (srcWidth - 1) * srcBpp;
        for(; x < dstWidth; x++)
        {
            xFract  = srcCoordX[x] & ImgSubpixelMask;
            filter(pDst3, 
                   pSrcRow1, pSrcRow1, 
                   pSrcRow2, pSrcRow2, 
                   xFract, yFract);
            pDst3 += dstBpp;
        }

        ++iy;
        pDst2 += dstPitch;
    }
}


//--------------------------------------------------------------------
void SF_STDCALL ResizeImageBilinear(UByte* pDst, 
                                    int dstWidth, int dstHeight, int dstPitch,
                                    const UByte* pSrc, 
                                    int srcWidth, int srcHeight, int srcPitch,
                                    ResizeImageType type)
{
    if (dstWidth <= 0 || dstHeight <= 0 ||
        srcWidth <= 0 || srcHeight <= 0) return;

    switch(type)
    {
    case ResizeRgbToRgb:
        ImageResizeFilter2x2(pDst, dstWidth, dstHeight, dstPitch, 3,
                             pSrc, srcWidth, srcHeight, srcPitch, 3,
                             PixelFilterBilinearRGB24);
        break;

    case ResizeRgbaToRgba: 
        ImageResizeFilter2x2(pDst, dstWidth, dstHeight, dstPitch, 4,
                             pSrc, srcWidth, srcHeight, srcPitch, 4,
                             PixelFilterBilinearRGBA32);
        break;

    case ResizeRgbToRgba:
        ImageResizeFilter2x2(pDst, dstWidth, dstHeight, dstPitch, 4,
                             pSrc, srcWidth, srcHeight, srcPitch, 3,
                             PixelFilterBilinearRGBtoRGBA32);
        break;

    case ResizeGray:
        ImageResizeFilter2x2(pDst, dstWidth, dstHeight, dstPitch, 1,
                             pSrc, srcWidth, srcHeight, srcPitch, 1,
                             PixelFilterBilinearGray8);
        break;
    case ResizeNone:
        break;
    }
}





//--------------------------------------------------------------------
struct PixelFilterGray8
{
    enum { SrcBpp = 1, DstBpp = 1 };

    static SF_INLINE void CopySrcPixel(UByte* pDst, const UByte* pSrc)
    {
        *pDst = *pSrc;
    }

    static SF_INLINE void Filter(UByte* pDst, 
                                 const UByte* pSrc,
                                 const SInt16* weightArray,
                                 int xFilter, int xCounter)
    {
        int dst = ImgFilterScale/2;
        for(;;)
        {
            dst += weightArray[xFilter] * *pSrc++;
            if(--xCounter == 0) break;
            xFilter += ImgSubpixelScale;
        }
        dst >>= ImgFilterShift;
        if(dst < 0)   dst = 0;
        if(dst > 255) dst = 255;
        *pDst = (UByte)dst;
    }
};


//--------------------------------------------------------------------
struct PixelFilterRGB24
{
    enum { SrcBpp = 3, DstBpp = 3 };

    static SF_INLINE void CopySrcPixel(UByte* pDst, const UByte* pSrc)
    {
        pDst[0] = pSrc[0];
        pDst[1] = pSrc[1];
        pDst[2] = pSrc[2];
    }

    static SF_INLINE void Filter(UByte* pDst, 
                                 const UByte* pSrc,
                                 const SInt16* weightArray,
                                 int xFilter, int xCounter)
    {
        int dst[] = { ImgFilterScale/2, ImgFilterScale/2, ImgFilterScale/2 }; 
        for(;;)
        {
            int weight = weightArray[xFilter];
            dst[0] += weight * pSrc[0];
            dst[1] += weight * pSrc[1];
            dst[2] += weight * pSrc[2];
            if(--xCounter == 0) break;
            xFilter += ImgSubpixelScale;
            pSrc    += SrcBpp;
        }

        dst[0] >>= ImgFilterShift;
        dst[1] >>= ImgFilterShift;
        dst[2] >>= ImgFilterShift;

        if(dst[0] < 0) dst[0] = 0;
        if(dst[1] < 0) dst[1] = 0;
        if(dst[2] < 0) dst[2] = 0;

        if(dst[0] > 255) dst[0] = 255;
        if(dst[1] > 255) dst[1] = 255;
        if(dst[2] > 255) dst[2] = 255;

        pDst[0] = (UByte)dst[0];
        pDst[1] = (UByte)dst[1];
        pDst[2] = (UByte)dst[2];
    }
};


//--------------------------------------------------------------------
struct PixelFilterRGBA32
{
    enum { SrcBpp = 4, DstBpp = 4 };

    static SF_INLINE void CopySrcPixel(UByte* pDst, const UByte* pSrc)
    {
        pDst[0] = pSrc[0];
        pDst[1] = pSrc[1];
        pDst[2] = pSrc[2];
        pDst[3] = pSrc[3];
    }

    static SF_INLINE void Filter(UByte* pDst, 
                                 const UByte* pSrc,
                                 const SInt16* weightArray,
                                 int xFilter, int xCounter)
    {
        int dst[4] = { ImgFilterScale/2, ImgFilterScale/2, 
                       ImgFilterScale/2, ImgFilterScale/2 }; 

        for(;;)
        {
            int weight = weightArray[xFilter];
            dst[0] += weight * pSrc[0];
            dst[1] += weight * pSrc[1];
            dst[2] += weight * pSrc[2];
            dst[3] += weight * pSrc[3];
            if(--xCounter == 0) break;
            xFilter += ImgSubpixelScale;
            pSrc    += SrcBpp;
        }

        dst[0] >>= ImgFilterShift;
        dst[1] >>= ImgFilterShift;
        dst[2] >>= ImgFilterShift;
        dst[3] >>= ImgFilterShift;

        if(dst[0] < 0) dst[0] = 0;
        if(dst[1] < 0) dst[1] = 0;
        if(dst[2] < 0) dst[2] = 0;
        if(dst[3] < 0) dst[3] = 0;

        if(dst[0] > 255) dst[0] = 255;
        if(dst[1] > 255) dst[1] = 255;
        if(dst[2] > 255) dst[2] = 255;
        if(dst[3] > 255) dst[3] = 255;

        pDst[0] = (UByte)dst[0];
        pDst[1] = (UByte)dst[1];
        pDst[2] = (UByte)dst[2];
        pDst[3] = (UByte)dst[3];
    }
};


//--------------------------------------------------------------------
struct PixelFilterRGB24toRGBA32
{
    enum { SrcBpp = 3, DstBpp = 4 };

    static SF_INLINE void CopySrcPixel(UByte* pDst, const UByte* pSrc)
    {
        pDst[0] = pSrc[0];
        pDst[1] = pSrc[1];
        pDst[2] = pSrc[2];
    }

    static SF_INLINE void Filter(UByte* pDst, 
                                 const UByte* pSrc,
                                 const SInt16* weightArray,
                                 int xFilter, int xCounter)
    {
        PixelFilterRGB24::Filter(pDst, pSrc, weightArray, xFilter, xCounter);
        pDst[3] = 255;
    }
};



//--------------------------------------------------------------------
template<class FilterType>
void ResizeImageRow(      UByte* pDst, unsigned dstWidth, int dstInc,
                           const UByte* pSrc, unsigned srcWidth, 
                           const int* srcCoordX,
                           const FilterType& filter,
                           const ImageFilterLut& lut)
{
    SF_UNUSED(filter); // Suppress stupid warning
    unsigned      diameter = lut.GetDiameter();
    int           start    = lut.GetStart();
    const SInt16* weights  = lut.GetWeightArray();

    unsigned i;
    unsigned x;
    int xFract;
    int xInt;
    int j;
    UByte tmpSrc[(ImgMaxFilterRadius*2+2) * FilterType::SrcBpp];

    // Filter left pixels, while the filtering window 
    // is out of the left bound of the source image.
    for(x = 0; x < dstWidth; ++x)
    {
        xFract = srcCoordX[x];
        xInt   = (xFract >> ImgSubpixelShift) + start;
        if (xInt >= 0) break;

        for (i = 0; i < diameter; ++i)
        {
            j = xInt + int(i);
            if (j < 0) j = 0;
            filter.CopySrcPixel(tmpSrc + i * FilterType::SrcBpp, 
                                pSrc   + j * FilterType::SrcBpp);
        }
        filter.Filter(pDst, tmpSrc, weights, 
                      ImgSubpixelMask - (xFract & ImgSubpixelMask), 
                      diameter);
        pDst += dstInc;
    }

    // Filter central pixels, while the filtering window 
    // is fully visible in the source image.
    for(; x < dstWidth; ++x)
    {
        xFract = srcCoordX[x];
        xInt   = (xFract >> ImgSubpixelShift) + start;
        if (xInt + diameter > srcWidth) break;
        filter.Filter(pDst, 
                      pSrc + FilterType::SrcBpp * xInt, 
                      weights, 
                      ImgSubpixelMask - (xFract & ImgSubpixelMask), 
                      diameter);
        pDst += dstInc;
    }

    // Filter right pixels, when the filtering window 
    // is out of the right bound of the source image.
    for(; x < dstWidth; ++x)
    {
        xFract  = srcCoordX[x];
        xInt    = (xFract >> ImgSubpixelShift) + start;

        for (i = 0; i < diameter; ++i)
        {
            j = xInt + int(i);
            if (j >= int(srcWidth)) j = int(srcWidth) - 1;
            filter.CopySrcPixel(tmpSrc + i * FilterType::SrcBpp, 
                                pSrc   + j * FilterType::SrcBpp);
        }
        filter.Filter(pDst, tmpSrc, weights, 
                      ImgSubpixelMask - (xFract & ImgSubpixelMask), 
                      diameter);
        pDst += dstInc;
    }
}


//--------------------------------------------------------------------
static void CreateResizeInterpolationArray(Scaleform::ArrayUnsafe<int>& srcCoord, 
                                           int dstWidth, int srcWidth)
{
    int x;
    int offset = ImgSubpixelOffset * srcWidth / dstWidth;
    LinearInterpolator ix(offset, 
                          offset + (srcWidth << ImgSubpixelShift),
                          dstWidth);

    srcCoord.Resize(dstWidth);
    for(x = 0; x < dstWidth; ++x)
    {
        srcCoord[x] = ix.y() - ImgSubpixelOffset;
        ++ix;
    }
}


//--------------------------------------------------------------------
template<class FilterType1, class FilterType2>
static void ResizeImageTwoPass(UByte* pDst, 
                               int dstWidth, int dstHeight, int dstPitch,
                               const UByte* pSrc, 
                               int srcWidth, int srcHeight, int srcPitch,
                               const FilterType1& filter1,
                               const FilterType2& filter2,
                               const ImageFilterLut& lut)
{
    Scaleform::ArrayUnsafe<int>   srcCoordX;
    Scaleform::ArrayUnsafe<UByte> tmpImg;
    int y;

    // This function resizes the image with filtering in two passes,
    // by X and Y with transposition for faster access to the memory. 
    // It requires a temporary buffer of dstWidth * srcHeight, but
    // reduces the computational complexity from R*R to R+R, where 
    // R is the radius of the filter.
    //-----------------------------
    tmpImg.Resize(dstWidth * srcHeight * FilterType1::DstBpp);

    // Filter the image in the X direction and transpose it.
    UByte* pTmp = &tmpImg[0];
    CreateResizeInterpolationArray(srcCoordX, dstWidth, srcWidth);
    for(y = 0; y < srcHeight; ++y)
    {
        ResizeImageRow<FilterType1>(pTmp, dstWidth, srcHeight * FilterType1::DstBpp,
                       pSrc, srcWidth, 
                       &srcCoordX[0], filter1, lut);
        pSrc += srcPitch;
        pTmp += FilterType1::DstBpp;
    }

    // Filter the image in the Y direction and transpose it back.
    pTmp = &tmpImg[0];
    CreateResizeInterpolationArray(srcCoordX, dstHeight, srcHeight);
    for(y = 0; y < dstWidth; ++y)
    {
        ResizeImageRow<FilterType2>(pDst, dstHeight, dstPitch,
                        pTmp, srcHeight, 
                        &srcCoordX[0], filter2, lut);
        pTmp += srcHeight * FilterType1::DstBpp;
        pDst += FilterType2::DstBpp;
    }
}


//--------------------------------------------------------------------
void SF_STDCALL ResizeImage(UByte* pDst, 
                            int dstWidth, int dstHeight, int dstPitch,
                            const UByte* pSrc, 
                            int srcWidth, int srcHeight, int srcPitch,
                            ResizeImageType type,
                            const ImageFilterLut& lut)
{
    if (dstWidth <= 0 || dstHeight <= 0 ||
        srcWidth <= 0 || srcHeight <= 0) return;

    switch(type)
    {
        case ResizeRgbToRgb:
        {
            PixelFilterRGB24 filter;
            ResizeImageTwoPass(pDst, dstWidth, dstHeight, dstPitch,
                               pSrc, srcWidth, srcHeight, srcPitch,
                               filter, filter, lut);
        }
        break;

        case ResizeRgbaToRgba: 
        {
            PixelFilterRGBA32 filter;
            ResizeImageTwoPass(pDst, dstWidth, dstHeight, dstPitch,
                               pSrc, srcWidth, srcHeight, srcPitch,
                               filter, filter, lut);
        }
        break;

        case ResizeRgbToRgba:
        {
            PixelFilterRGB24         filter1;
            PixelFilterRGB24toRGBA32 filter2;
            ResizeImageTwoPass(pDst, dstWidth, dstHeight, dstPitch,
                               pSrc, srcWidth, srcHeight, srcPitch,
                               filter1, filter2, lut);
        }
        break;

    case ResizeGray:
        {
            PixelFilterGray8 filter;
            ResizeImageTwoPass(pDst, dstWidth, dstHeight, dstPitch,
                               pSrc, srcWidth, srcHeight, srcPitch,
                               filter, filter, lut);
        }
        break;

    case ResizeNone:
        break;
    }
}

}}



