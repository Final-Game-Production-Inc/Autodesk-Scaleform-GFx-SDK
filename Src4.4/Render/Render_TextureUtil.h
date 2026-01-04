/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TextureUtil.h
Content     :   Mipmap/Image conversion and generation functions
                used for texture implementation.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TextureUtil_H
#define INC_SF_Render_TextureUtil_H

#include "Render_Image.h"
#include "Render_ResizeImage.h"


namespace Scaleform { namespace Render {


// ***** Rescale Support

ImageRescaleType SF_STDCALL GetImageFormatRescaleType(ImageFormat format);
void    SF_STDCALL  RescaleImageData(ImageData& dest, ImageData& src,
                                     ImageRescaleType resizeType);


// ***** Scan-line Conversion

void    SF_STDCALL  ConvertImagePlane(const ImagePlane &dplane, const ImagePlane &splane,
                                      ImageFormat format, unsigned formatPlaneIndex,
                                      Image::CopyScanlineFunc copyScanline,
                                      Palette* pcolorMap, void* scanlineArg = 0);

void    SF_STDCALL  ConvertImageData(ImageData& dest, ImageData& src,
                                     Image::CopyScanlineFunc copyScanline,
                                     void* scanlineArg = 0);


// ***** Software Mip-map Generation

// Makes a next MipmapLevel based on image format.
// Source and destination ImagePlane(s) are allowed to be the same.

void    SF_STDCALL  GenerateMipLevel(ImagePlane& dplane, ImagePlane& splane,
                                     ImageFormat format, unsigned formatPlaneIndex);


// Obtains an image scaline conversion function which converts between the specified default formats.
Image::CopyScanlineFunc SF_STDCALL GetImageConvertFunc(ImageFormat destFormat, ImageFormat sourceFormat);

// ***** General scan-line conversion functions

// Generalized swizzle. The template parameters correspond to the destination channels of the source values in order.
// Eg. If the input is ARGB, and you do Image_CopyScanline32_Swizzle<1,2,3,0>(...), the output will be RGBA.
template<int d0, int d1, int d2, int d3>
void    SF_STDCALL  Image_CopyScanline32_Swizzle(UByte* pd, const UByte* ps, UPInt size, Palette*, void*)
{
    for (UPInt i = 0; i< size; i+=4, pd+=4, ps+=4)
    {
        pd[d0] = ps[0];
        pd[d1] = ps[1];
        pd[d2] = ps[2];
        pd[d3] = ps[3];
    }
}

void    SF_STDCALL  Image_CopyScanline24_SwapBR(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline32_SwapBR(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline32_RGBA_ARGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline24_Extend_RGB_RGBA(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline24_Extend_RGB_BGRA(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline24_Extend_RGB_ABGR(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline24_Extend_RGB_ARGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline32_Retract_BGRA_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline32_Retract_RGBA_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline32_Retract_ARGB_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);

}};  // namespace Scaleform::Render

#endif
