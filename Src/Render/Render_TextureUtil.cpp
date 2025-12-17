/**************************************************************************

Filename    :   Render_TextureUtil.cpp
Content     :   Mipmap/Image conversion and generation functions
                used for texture implementation.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_TextureUtil.h"
#include "Render_ResizeImage.h"

namespace Scaleform { namespace Render {


// ***** Rescale Support

ImageRescaleType SF_STDCALL GetImageFormatRescaleType(ImageFormat format)
{
    switch(format)
    {
    case Image_R8G8B8A8:
    case Image_B8G8R8A8:
        return ResizeRgbaToRgba;
    case Image_R8G8B8:
    case Image_B8G8R8:
        return ResizeRgbToRgb;
    case Image_A8:
        return ResizeGray;
    default:
        // fail
        break;
    }
    return ResizeNone;
}

void SF_STDCALL RescaleImageData(ImageData& dest, ImageData& src, ImageRescaleType resizeType)
{
    // For all data planes, do resize
    unsigned planeCount = src.GetPlaneCount();
    for (unsigned iplane = 0; iplane < planeCount; iplane++)
    {
        ImagePlane splane, dplane;
        src.GetPlane(iplane, &splane);
        dest.GetPlane(iplane, &dplane);
        ResizeImageBilinear(dplane.pData, dplane.Width, dplane.Height, (int)dplane.Pitch,
                            splane.pData, splane.Width, splane.Height, (int)splane.Pitch,
                            resizeType);
    }
}


// ***** Scan-line Conversion


void SF_STDCALL ConvertImagePlane(const ImagePlane &dplane, const ImagePlane &splane,
                                  ImageFormat format, unsigned formatPlaneIndex,
                                  Image::CopyScanlineFunc copyScanline,
                                  Palette* pcolorMap, void* scanlineArg)
{
    SF_ASSERT(splane.Width == dplane.Width);

    // Get number of scan-lines. Typically this is the same
    // as Height but is different for compressed formats.    
    UPInt   bitsPerPixel  = ImageData::GetFormatBitsPerPixel(format, formatPlaneIndex);
    UPInt   scanlineCount = ImageData::GetFormatScanlineCount(format, splane.Height,
                                                              formatPlaneIndex);

    UByte* psource = splane.pData;
    UByte* pdest   = dplane.pData;

    for (UPInt j = 0; j < scanlineCount; j++, psource += splane.Pitch, pdest += dplane.Pitch)
        copyScanline(pdest, psource, (bitsPerPixel * splane.Width) / 8, pcolorMap, scanlineArg);
}

void SF_STDCALL ConvertImageData(ImageData& dest, ImageData& src,
                                 Image::CopyScanlineFunc copyScanline,
                                 void* scanlineArg)
{
    ImageFormat format           = src.GetFormat();
    unsigned    formatPlaneCount = ImageData::GetFormatPlaneCount(format);
    unsigned    planeCount       = src.GetPlaneCount();

    for (unsigned iplane = 0; iplane < planeCount; iplane++)
    {
        ImagePlane splane, dplane;
        src.GetPlane(iplane, &splane);
        dest.GetPlane(iplane, &dplane);
        ConvertImagePlane(dplane, splane, format,
                          planeCount % formatPlaneCount, copyScanline,
                          dest.GetColorMap(), scanlineArg);
    }
}



// Makes a next MipmapLevel based on image format.
// Source and destination ImagePlane(s) are allowed to be the same.

void SF_STDCALL GenerateMipLevel(ImagePlane& dplane, ImagePlane& splane,
                                 ImageFormat format, unsigned formatPlaneIndex)
{
    //SF_ASSERT_ON_RENDERER_MIPMAP_GEN;
    SF_UNUSED(formatPlaneIndex);
    
    enum {
        FixedPointShift = 10,
        FixedPointMul   = 1 << FixedPointShift,
        FixedPointMask  = FixedPointMul - 1
    };
    UPInt    spitch = splane.Pitch,
             dpitch = dplane.Pitch;
    unsigned dx     = (splane.Width * FixedPointMul) / dplane.Width;
    unsigned dy     = (splane.Height * FixedPointMul) / dplane.Height;
    unsigned rx, ry;
    unsigned i, j;

    // Fraction is initialized as follows: (dy-1)/2
    //  dy/2 - Ensures that we begin at middle pixel for 1/2 size.
    //  -1   - One is subtracted to ensure that pointer value never reaches precisely
    //         the last pixel (else second sample would be out of bounds).

    if ((splane.Width != 1) && (splane.Height != 1))
    {
        for (j = 0, ry = (dy-1)/4; j < dplane.Height; j++, ry += dy)
        {
            UByte*   out     = dplane.pData + j * dpitch;
            UByte*   in0     = splane.pData + (ry >> FixedPointShift) * spitch;
            unsigned yrem    = (ry & FixedPointMask);
            unsigned yreminv = FixedPointMask - yrem;

            SF_ASSERT(((ry >> FixedPointShift) + 1) < splane.Height);

            switch(format & ~ImageFormat_Convertible)
            {
            case Image_R8G8B8A8:
            case Image_B8G8R8A8:
                for (i = 0, rx = (dx-1)/4; i < dplane.Width; i++, rx += dx)
                {
                    unsigned xrem    = (rx & FixedPointMask);
                    unsigned xreminv = FixedPointMask - xrem;
                    UByte*   in      = in0 + (rx >> FixedPointShift) * 4;

                    unsigned r = (in[0]          * xrem + in[4]          * xreminv) * yrem +
                                 (in[0 + spitch] * xrem + in[4 + spitch] * xreminv) * yreminv;
                    unsigned g = (in[1]          * xrem + in[5]          * xreminv) * yrem +
                                 (in[1 + spitch] * xrem + in[5 + spitch] * xreminv) * yreminv;
                    unsigned b = (in[2]          * xrem + in[6]          * xreminv) * yrem +
                                 (in[2 + spitch] * xrem + in[6 + spitch] * xreminv) * yreminv;
                    unsigned a = (in[3]          * xrem + in[7]          * xreminv) * yrem +
                                 (in[3 + spitch] * xrem + in[7 + spitch] * xreminv) * yreminv;
                    
                    out[0] = UByte(r >> (FixedPointShift * 2));
                    out[1] = UByte(g >> (FixedPointShift * 2));
                    out[2] = UByte(b >> (FixedPointShift * 2));
                    out[3] = UByte(a >> (FixedPointShift * 2));
                    out+=4;
                }
                break;

            case Image_A8:
            case Image_Y8_U2_V2:
            case Image_Y8_U2_V2_A8:
                 for (i = 0, rx = (dx-1)/4; i < dplane.Width; i++, rx += dx)
                {
                    unsigned xrem    = (rx & FixedPointMask);
                    unsigned xreminv = FixedPointMask - xrem;
                    UByte*   in      = in0 + (rx >> FixedPointShift);

                    unsigned a = (in[0]          * xrem + in[1]          * xreminv) * yrem +
                                 (in[0 + spitch] * xrem + in[1 + spitch] * xreminv) * yreminv; 
                    out[0] = UByte(a >> (FixedPointShift * 2));
                    out++;
                }
                break;
            }        
        }    
    }

    else if (splane.Width != 1)
    {
        SF_ASSERT(splane.Height == 1);

        UByte*  out = dplane.pData;
        UByte*  in0 = splane.pData;

        switch(format & ~ImageFormat_Convertible)
        {
        case Image_R8G8B8A8:
        case Image_B8G8R8A8:
            for (i = 0, rx = (dx-1)/4; i < dplane.Width; i++, rx += dx)
            {
                unsigned xrem    = (rx & FixedPointMask);
                unsigned xreminv = FixedPointMask - xrem;
                UByte*   in      = in0 + (rx >> FixedPointShift) * 4;

                unsigned r = in[0] * xrem + in[4] * xreminv;
                unsigned g = in[1] * xrem + in[5] * xreminv;
                unsigned b = in[2] * xrem + in[6] * xreminv;
                unsigned a = in[3] * xrem + in[7] * xreminv;
                
                out[0] = UByte(r >> FixedPointShift);
                out[1] = UByte(g >> FixedPointShift);
                out[2] = UByte(b >> FixedPointShift);
                out[3] = UByte(a >> FixedPointShift);
                out+=4;
            }
            break;

        case Image_A8:
        case Image_Y8_U2_V2:
        case Image_Y8_U2_V2_A8:
            for (i = 0, rx = (dx-1)/4; i < dplane.Width; i++, rx += dx)
            {
                unsigned xrem    = (rx & FixedPointMask);
                unsigned xreminv = FixedPointMask - xrem;
                UByte*   in = in0 + (rx >> FixedPointShift);
                unsigned a  = in[0] * xrem + in[1] * xreminv;
                out[0] = UByte(a >> FixedPointShift);
                out++;
            }
            break;
        }
    }

    else if (splane.Height != 1)
    {
        SF_ASSERT(splane.Width == 1);
        
        UByte*  out = dplane.pData;
        UByte*  in0 = splane.pData;

        switch(format & ~ImageFormat_Convertible)
        {
        case Image_R8G8B8A8:
        case Image_B8G8R8A8:
            for (j = 0, ry = (dy-1)/4; j < dplane.Height; j++, ry += dy)
            {                
                UByte*   in      = in0 + (ry >> FixedPointShift) * spitch;
                unsigned yrem    = (ry & FixedPointMask);
                unsigned yreminv = FixedPointMask - yrem;

                unsigned r = in[0] * yrem + in[0 + spitch] * yreminv;
                unsigned g = in[1] * yrem + in[1 + spitch] * yreminv;
                unsigned b = in[2] * yrem + in[2 + spitch] * yreminv;
                unsigned a = in[3] * yrem + in[3 + spitch] * yreminv;

                out[0] = UByte(r >> FixedPointShift);
                out[1] = UByte(g >> FixedPointShift);
                out[2] = UByte(b >> FixedPointShift);
                out[3] = UByte(a >> FixedPointShift);
                out += dpitch;
            }
            break;

        case Image_A8:
        case Image_Y8_U2_V2:
        case Image_Y8_U2_V2_A8:
            for (j = 0, ry = (dy-1)/4; j < dplane.Height; j++, ry += dy)
            {                
                UByte*   in      = in0 + (ry >> FixedPointShift) * spitch;
                unsigned yrem    = (ry & FixedPointMask);
                unsigned yreminv = FixedPointMask - yrem;

                unsigned a = in[0] * yrem + in[0 + spitch] * yreminv;           
                out[0] = UByte(a >> FixedPointShift);
                out += dpitch;
            }
            break;
        }
    }
}

// ***** General scan-line conversion functions


void SF_STDCALL  Image_CopyScanline32_SwapBR(UByte* pd, const UByte* ps, UPInt size,
                                             Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=4, pd+=4, ps+=4)
    {
        pd[0] = ps[2];
        pd[1] = ps[1];
        pd[2] = ps[0];
        pd[3] = ps[3];
    }
}

void SF_STDCALL Image_CopyScanline32_RGBA_ARGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=4, pd+=4, ps+=4)
    {
        pd[0] = ps[3];
        pd[1] = ps[0];
        pd[2] = ps[1];        
        pd[3] = ps[2];
    }
}

void SF_STDCALL  Image_CopyScanline24_SwapBR(UByte* pd, const UByte* ps, UPInt size,
                                             Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd+=3, ps+=3)
    {
        pd[0] = ps[2];
        pd[1] = ps[1];
        pd[2] = ps[0];        
    }
}


void SF_STDCALL   Image_CopyScanline24_Extend_RGB_RGBA(UByte* pd, const UByte* ps, UPInt size,
                                                       Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd+=4, ps+=3)
    {
        pd[0] = ps[0];
        pd[1] = ps[1];
        pd[2] = ps[2];
        pd[3] = 255;
    }
}

void SF_STDCALL   Image_CopyScanline24_Extend_RGB_BGRA(UByte* pd, const UByte* ps, UPInt size,
                                                       Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd+=4, ps+=3)
    {
        pd[0] = ps[2];
        pd[1] = ps[1];
        pd[2] = ps[0];
        pd[3] = 255;
    }
}

void SF_STDCALL   Image_CopyScanline24_Extend_RGB_ABGR(UByte* pd, const UByte* ps, UPInt size,
                                                       Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd+=4, ps+=3)
    {
        pd[0] = 255;
        pd[1] = ps[2];
        pd[2] = ps[1];
        pd[3] = ps[0];
    }
}

void SF_STDCALL   Image_CopyScanline24_Extend_RGB_ARGB(UByte* pd, const UByte* ps, UPInt size,
                                                       Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd+=4, ps+=3)
    {
        pd[0] = 255;
        pd[1] = ps[0];
        pd[2] = ps[1];
        pd[3] = ps[2];
    }
}

void    SF_STDCALL  Image_CopyScanline32_Retract_BGRA_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd+=3, ps+=4)
    {
        pd[0] = ps[2];
        pd[1] = ps[1];
        pd[2] = ps[0];
    }
}

void    SF_STDCALL  Image_CopyScanline32_Retract_RGBA_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd+=3, ps+=4)
    {
        pd[0] = ps[0];
        pd[1] = ps[1];
        pd[2] = ps[2];
    }
}

void    SF_STDCALL  Image_CopyScanline32_Retract_ARGB_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd+=3, ps+=4)
    {
        pd[0] = ps[1];
        pd[1] = ps[2];
        pd[2] = ps[3];
    }
}

void SF_STDCALL   Image_CopyScanline_BGRA_BGR(UByte* pd, const UByte* ps, UPInt size,
                                              Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=4, pd+=3, ps+=4)
    {
        pd[0] = ps[0];
        pd[1] = ps[1];
        pd[2] = ps[2];
    }
}
void SF_STDCALL   Image_CopyScanline_BGRA_RGB(UByte* pd, const UByte* ps, UPInt size,
                                              Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=4, pd+=3, ps+=4)
    {
        pd[0] = ps[2];
        pd[1] = ps[1];
        pd[2] = ps[0];
    }
}


void SF_STDCALL   Image_CopyScanline_BGRA_A(UByte* pd, const UByte* ps, UPInt size,
                                            Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=4, pd++, ps+=4)
        pd[0] = ps[3];
}

// Averages BGR channels. Should this be explicit-only?
void SF_STDCALL   Image_CopyScanline_BGR_A_Avg(UByte* pd, const UByte* ps, UPInt size,
                                               Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=3, pd++, ps+=3)
    {
        UByte alpha = UByte((unsigned(ps[0]) + ps[1] + ps[2])/3);
        pd[0] = alpha;
    }
}

void SF_STDCALL   Image_CopyScanline_A_BGRA(UByte* pd, const UByte* ps, UPInt size,
                                           Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=4, ps++)
    {
        pd[0] = 0xFF;
        pd[1] = 0xFF;
        pd[2] = 0xFF;
        pd[3] = ps[0];
    }
}

void SF_STDCALL   Image_CopyScanline_A_BGR(UByte* pd, const UByte* ps, UPInt size,
                                           Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=3, ps++)
    {
        pd[0] = ps[0];
        pd[1] = ps[0];
        pd[2] = ps[0];
    }
}


// Palette-indexed. These conversions are used by TGA loader
void SF_STDCALL   Image_CopyScanline_P_BGRA(UByte* pd, const UByte* ps, UPInt size,
                                           Palette* colorMap, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=4, ps++)
    {
        Color c = (*colorMap)[ps[0]];
        pd[0] = c.GetBlue();
        pd[1] = c.GetGreen();
        pd[2] = c.GetRed();
        pd[3] = c.GetAlpha();
    }
}
void SF_STDCALL   Image_CopyScanline_P_RGBA(UByte* pd, const UByte* ps, UPInt size,
                                            Palette* colorMap, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=4, ps++)
    {
        Color c = (*colorMap)[ps[0]];
        pd[0] = c.GetRed();
        pd[1] = c.GetGreen();
        pd[2] = c.GetBlue();
        pd[3] = c.GetAlpha();
    }
}

void SF_STDCALL   Image_CopyScanline_P_BGR(UByte* pd, const UByte* ps, UPInt size,
                                           Palette* colorMap, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=3, ps++)
    {
        Color c = (*colorMap)[ps[0]];
        pd[0] = c.GetBlue();
        pd[1] = c.GetGreen();
        pd[2] = c.GetRed();
    }
}

void SF_STDCALL   Image_CopyScanline_P_RGB(UByte* pd, const UByte* ps, UPInt size,
                                           Palette* colorMap, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=3, ps++)
    {
        Color c = (*colorMap)[ps[0]];
        pd[0] = c.GetRed();
        pd[1] = c.GetGreen();
        pd[2] = c.GetBlue();
    }
}

void SF_STDCALL   Image_CopyScanline_P_A(UByte* pd, const UByte* ps, UPInt size,
                                         Palette* colorMap, void*)
{
    for (UPInt i = 0; i< size; i++, pd++, ps++)
    {
        Color c = (*colorMap)[ps[0]];
        if (colorMap->HasAlpha())
            pd[0] = c.GetAlpha();
        else
            pd[0] = (UByte)((unsigned(c.GetRed()) + unsigned(c.GetGreen()) + unsigned(c.GetBlue()))/3);
    }
}



struct ScanlineConvert
{
    ImageFormat              Source;
    ImageFormat              Dest;    
    Image::CopyScanlineFunc  CopyFunc;
};

static ScanlineConvert ImageScanlineConvertTable[] = 
{
    { Image_R8G8B8A8, Image_B8G8R8A8, Image_CopyScanline32_SwapBR },
    { Image_R8G8B8A8, Image_R8G8B8,   Image_CopyScanline_BGRA_BGR },
    { Image_R8G8B8A8, Image_R8G8B8,   Image_CopyScanline_BGRA_RGB },
    { Image_R8G8B8A8, Image_A8,       Image_CopyScanline_BGRA_A },

    { Image_B8G8R8A8, Image_R8G8B8A8, Image_CopyScanline32_SwapBR },
    { Image_B8G8R8A8, Image_B8G8R8,   Image_CopyScanline_BGRA_BGR },
    { Image_B8G8R8A8, Image_R8G8B8,   Image_CopyScanline_BGRA_RGB },
    { Image_B8G8R8A8, Image_A8,       Image_CopyScanline_BGRA_A },

    { Image_R8G8B8,   Image_B8G8R8A8, Image_CopyScanline24_Extend_RGB_BGRA },
    { Image_R8G8B8,   Image_R8G8B8A8, Image_CopyScanline24_Extend_RGB_RGBA },
    { Image_R8G8B8,   Image_A8,       Image_CopyScanline_BGR_A_Avg },
    { Image_R8G8B8,   Image_B8G8R8,   Image_CopyScanline24_SwapBR },

    { Image_B8G8R8,   Image_B8G8R8A8, Image_CopyScanline24_Extend_RGB_RGBA },
    { Image_B8G8R8,   Image_R8G8B8A8, Image_CopyScanline24_Extend_RGB_BGRA },
    { Image_B8G8R8,   Image_A8,       Image_CopyScanline_BGR_A_Avg },
    { Image_B8G8R8,   Image_R8G8B8,   Image_CopyScanline24_SwapBR },

    { Image_A8,       Image_B8G8R8A8, Image_CopyScanline_A_BGRA},
    { Image_A8,       Image_R8G8B8A8, Image_CopyScanline_A_BGRA},
    { Image_A8,       Image_B8G8R8,   Image_CopyScanline_A_BGR},
    { Image_A8,       Image_R8G8B8,   Image_CopyScanline_A_BGR},

    // Palette conversions, needed for TGA loading
    { Image_P8,       Image_B8G8R8A8, Image_CopyScanline_P_BGRA},
    { Image_P8,       Image_R8G8B8A8, Image_CopyScanline_P_RGBA},
    { Image_P8,       Image_B8G8R8,   Image_CopyScanline_P_BGR},
    { Image_P8,       Image_R8G8B8,   Image_CopyScanline_P_RGB},
    { Image_P8,       Image_A8,       Image_CopyScanline_P_A},
   
    { Image_None, Image_None, 0 }    
};

// Obtains an image scanline conversion function which converts
// between the specified default formats.
Image::CopyScanlineFunc SF_STDCALL GetImageConvertFunc(ImageFormat destFormat,
                                                       ImageFormat sourceFormat)
{
    if (destFormat == sourceFormat)
        return &Image::CopyScanlineDefault;

    ScanlineConvert* convert = ImageScanlineConvertTable;
    while(convert->Source != Image_None)
    {
        if ((convert->Source == sourceFormat) && (convert->Dest == destFormat))
            return convert->CopyFunc;
        convert++;
    }   
    return 0;
}




}};  // namespace Scaleform::Render
