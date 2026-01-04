/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ImageSupport.h
Content     :   
Created     :   Sep, 2010
Authors     :   Artem Bolgar

Notes       :   Redesigned to use inherited state objects for GFx 2.0.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef SF_GFx_ImageSupport_H
#define SF_GFx_ImageSupport_H

#include "GFx/GFx_Loader.h"
#include "Render/ImageFiles/JPEG_ImageFile.h"

namespace Scaleform { namespace GFx {

#ifdef SF_ENABLE_ZLIB

class ZlibSupportBase;
using Render::ImageData;
using Render::ImageSize;
using Render::ImageRect;
using Render::ImageFormat;
using Render::ImageUpdateSync;

#ifdef SF_ENABLE_LIBJPEG

// A memory image of JPEG + Zlibed alpha channel.
class MemoryBufferJpegImageWithZlibAlphas : public Render::ImageDelegate
{
    Ptr<ZlibSupportBase>              ZLib;
    Render::JPEG::AbstractReader*     JpegReader;
    unsigned                          ZlibAlphaOffset;
    Render::ImageFormat               Format;
public:
    MemoryBufferJpegImageWithZlibAlphas(ZlibSupportBase* zlib, Render::JPEG::AbstractReader* reader,
                                        unsigned alphaPos,
                                        ImageFormat format, const ImageSize& size, unsigned use,
                                        ImageUpdateSync* sync, 
                                        File* file, UPInt length = 0);

    virtual ImageFormat     GetFormat() const     { return Format; }

    virtual Render::Texture* GetTexture(Render::TextureManager* pmanager);

    // Decodes image data; see Image::Decode.
    virtual bool    Decode(ImageData* pdest, CopyScanlineFunc copyScanline = CopyScanlineDefault,
                           void* arg = 0) const;

    virtual Render::Image* GetAsImage() { return this; }
    virtual Render::MemoryBufferImage* GetAsMemoryImage() 
    { return static_cast<Render::MemoryBufferImage*>(pImage.GetPtr()); }
};

#endif

// Image source for Zlib-compressed swf image
class ZlibImageSource : public Render::FileImageSource
{
public:
    enum SourceBitmapDataFormat
    {
        ColorMappedRGB,
        RGB16,
        RGB24,
        ColorMappedRGBA,
        RGBA
    };
protected:
    Ptr<ZlibSupportBase>    Zlib;
    UInt16                  ColorTableSize;
    SourceBitmapDataFormat  BitmapFormatId;
public:
    ZlibImageSource(ZlibSupportBase* zlib, File* file, const ImageSize& size, 
                    SourceBitmapDataFormat bmpFormatId, ImageFormat format = Render::Image_None, 
                    UInt16 colorTableSize = 0, UInt64 uncompressedLen = 0);

    ~ZlibImageSource() {}

    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    // Creates an image out of source that is compatible with argument Use,
    // updateSync and other settings.
    virtual Render::Image* CreateCompatibleImage(const Render::ImageCreateArgs& args);
    virtual unsigned    GetMipmapCount() const { return 1;}
};

// A memory image of lossless zlib-compressed image
class MemoryBufferZlibImage : public Render::MemoryBufferImage
{
public:
    typedef ZlibImageSource::SourceBitmapDataFormat SourceBitmapDataFormat;
protected:
    Ptr<ZlibSupportBase>    Zlib;
    UInt16                  ColorTableSize;
    SourceBitmapDataFormat  BitmapFormatId;
public:
    MemoryBufferZlibImage(ZlibSupportBase* zlib, ImageFormat format, const ImageSize& size, SourceBitmapDataFormat bmpFormatId, 
        UInt16 colorTableSize, unsigned use, ImageUpdateSync* sync, 
        File* file, SInt64 filePos, UPInt length = 0)
        : Render::MemoryBufferImage(format,size, use, sync, file, filePos, length),
        Zlib(zlib), ColorTableSize(colorTableSize), BitmapFormatId(bmpFormatId)
    { }

    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
};

#endif

}} // Scaleform::GFx

#endif
