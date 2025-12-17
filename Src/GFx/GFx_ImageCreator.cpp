/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ImageCreator.cpp
Content     :   
Created     :   June 21, 2005
Authors     :   Michael Antonov

Notes       :   Redesigned to use inherited state objects for GFx 2.0.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_HeapNew.h"

#include "Render/Render_Image.h"
#include "Render/Render_ImageFiles.h"
#include "GFx_ImageCreator.h"

#include "Render/ImageFiles/SIF_ImageFile.h"
#include "Render/ImageFiles/TGA_ImageFile.h"
#ifdef SF_ENABLE_LIBJPEG
#include "Render/ImageFiles/PNG_ImageFile.h"
#endif
#ifdef SF_ENABLE_LIBPNG
#include "Render/ImageFiles/JPEG_ImageFile.h"
#endif
#if !defined(SF_OS_IPHONE)
#include "Render/ImageFiles/DDS_ImageFile.h"
#endif
#if defined(SF_OS_3DS) || defined(SF_OS_ANDROID) || defined(SF_OS_PSVITA) || defined(SF_OS_IPHONE)
#include "Render/ImageFiles/PVR_ImageFile.h"
#endif
#ifdef SF_OS_ANDROID
#include "Render/ImageFiles/KTX_ImageFile.h"
#endif
#ifdef SF_OS_PSVITA
#include "Render/ImageFiles/GXT_ImageFile.h"
#endif
#ifdef SF_OS_WIIU
#include "Render/ImageFiles/GTX_ImageFile.h"
#endif
#ifdef SF_OS_ORBIS
#include "Render/ImageFiles/GNF_ImageFile.h"
#endif

namespace Scaleform { namespace GFx {

using Render::ImageFileReader;
using Render::ImageCreateArgs;
    
ImageCreator::ImageCreator(TextureManager* textureManager) 
: State(State_ImageCreator), pTextureManager(textureManager) 
{ 
}

// Looks up image for "img://" protocol.
Image* ImageCreator::LoadProtocolImage(const ImageCreateInfo& info, const String& url)
{
    SF_UNUSED2(info, url);
    return 0;
}

// Loads image from file.
Image* ImageCreator::LoadImageFile(const ImageCreateInfo& info, const String& url)
{
    ImageFileHandlerRegistry* registry = info.GetImageFileHandlerRegistry();
    if (!registry || !info.GetFileOpener())
        return 0;
    String path = url;
    if (!url.HasExtension())
    {
        if (pTextureManager)
        {
            FileStat tmp;
            unsigned fs = pTextureManager->GetTextureFormatSupport();
            if (fs & Render::ImageFormats_DXT)
            {
                path = url + ".dds";
                if (SysFile::GetFileStat(&tmp, path))
                    goto file_detected;
            }
            else if ((fs & Render::ImageFormats_PVR) || (fs & Render::ImageFormats_ETC))
            {
                path = url + ".pvr";
                if (SysFile::GetFileStat(&tmp, path))
                    goto file_detected;
            }
            else if (fs & Render::ImageFormats_ATITC)
            {
                path = url + ".sif";
                if (SysFile::GetFileStat(&tmp, path))
                    goto file_detected;
            }
        }
        path = url + ".tga";
    }
file_detected:
    Ptr<File> file;
    Array<UByte> bytes;
    if (URLBuilder::IsProtocol(path))
    {
#ifdef SF_ENABLE_HTTP_LOADING
        if (URLBuilder::SendURLRequest(&bytes, path) && !bytes.IsEmpty())
        {
            file = *SF_NEW MemoryFile(path, bytes.GetDataPtr(), (int)bytes.GetSize());
        }
#endif
    }
    else
    {
        file = *info.GetFileOpener()->OpenFile(path);
    }

    ImageFileReader* reader;
    ImageCreateArgs args;
    if (registry->DetectFormat(&reader, file) != Render::ImageFile_Unknown)
    {
        // Get image source.
        args.pHeap      = info.pHeap;
        args.Use        = info.Use;
        args.pManager   = pTextureManager;
        if (info.RUse == Resource::Use_FontTexture)
            args.Format = Render::Image_A8;

        Ptr<ImageSource> source = *reader->ReadImageSource(file, args);
        if (source)
        {
            return CreateImage(info, source);
        }
    }

    return registry->ReadImage(file, args);
}

// Loads exported image file.
Image* ImageCreator::LoadExportedImage(const ImageCreateExportInfo& info, const String& url)
{
    Image* result = LoadImageFile(info, url);

    // Fall-back trying ".dds", ".tga"
    if (!result && url.HasExtension())
    {

        if (url.GetExtension().ToLower() != ".dds")
        {
            String url2 = url;
            url2.StripExtension();
            url2 += ".dds";
            result = LoadImageFile(info, url2);
        }
    }

    return result;
}

// Creates image
//  - Returns image that matches expected protocol.
Image* ImageCreator::CreateImage(const ImageCreateInfo& info, ImageSource* source)
{
    ImageCreateArgs args;
    args.pHeap = info.pHeap;
    args.Use = info.Use;
    args.pManager = pTextureManager;
    if (info.RUse == Resource::Use_FontTexture)
        args.Format = Render::Image_A8;

    return source->CreateCompatibleImage(args);

}

ImageFileHandlerRegistry::ImageFileHandlerRegistry(InitType init)
    : State(State_ImageFileHandlerRegistry), Render::ImageFileHandlerRegistry(0)
{

    switch(init)
    {
        default:
        case NoInit:
            break;

        case AddDefaultHandlers:
        {
            AddHandler(&Render::SIF::FileReader::Instance);
            AddHandler(&Render::TGA::FileReader::Instance);
#ifdef SF_ENABLE_LIBJPEG
            AddHandler(&Render::JPEG::FileReader::Instance);
#endif
#ifdef SF_ENABLE_LIBPNG
            AddHandler(&Render::PNG::FileReader::Instance);
#endif

            // Platform specific image readers.
#if !defined(SF_OS_IPHONE)
            AddHandler(&Render::DDS::FileReader::Instance);
#endif
#if defined(SF_OS_3DS) || defined(SF_OS_ANDROID) || defined(SF_OS_PSVITA) || defined(SF_OS_IPHONE)
            AddHandler(&Render::PVR::FileReader::Instance);
#endif
#ifdef SF_OS_ANDROID
            AddHandler(&Render::KTX::FileReader::Instance);
#endif
#ifdef SF_OS_PSVITA
            AddHandler(&Render::GXT::FileReader::Instance);
#endif
#ifdef SF_OS_WIIU
            AddHandler(&Render::GTX::FileReader::Instance);
#endif
#ifdef SF_OS_ORBIS
            AddHandler(&Render::GNF::FileReader::Instance);
#endif
            break;
        }
    }
}

void ImageFileHandlerRegistry::AddHandler(ImageFileHandler* handler)
{
    Render::ImageFileHandlerRegistry::AddHandler(handler);
}

}} // Scaleform::GFx
