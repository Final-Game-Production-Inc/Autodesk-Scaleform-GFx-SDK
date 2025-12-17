/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ImageCreator.h
Content     :   
Created     :   June 21, 2005
Authors     :   Michael Antonov

Notes       :   Redesigned to use inherited state objects for GFx 2.0.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef SF_GFx_ImageCreator_H
#define SF_GFx_ImageCreator_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_File.h"

#include "Render/Render_Image.h"
#include "Render/Render_ImageFiles.h"

#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Resource.h"

namespace Scaleform { namespace GFx {

using Render::ImageFileFormat;
using Render::ImageSize;
using Render::TextureManager;
using Render::Image;
using Render::ImageSource;
using Render::ImageFileHandler;

class FileOpener;
class ImageFileHandlerRegistry;
class Movie;

// Image creation information passed ImageCreator::CreateImage and GFxStateBag::CreateImageInfo. 
// This data can be used to decide on the type of Render::ImageInfoBase class to create and provides 
// the data which is stored there.

class ImageCreateInfo
{ 
public:
    // Type of creation expected for this object.
    enum CreateType
    {
        Create_Protocol,
        Create_FileImage,
        Create_ExportImage,
        Create_SourceImage
    };
    CreateType      Type;

    // Memory heap to use for GFxImageInfo (use global heap if null).
    MemoryHeap*     pHeap;
    unsigned        Use;
    Resource::ResourceUse RUse;
    // Useful states.
    Log*            pLog;
    FileOpener*     pFileOpener;
    ImageFileHandlerRegistry* pIFHRegistry;
    Movie*          pMovie; // a pointer to a Movie; set only for Create_Protocol type!

    // Assume image use is wrapped by default. On GLES, this makes a difference, because
    // wrapped textures cannot be used as NPOT, and thus will be resized unless explicitly
    // marked as clamped.
    ImageCreateInfo(CreateType type, MemoryHeap* heap = 0, unsigned imageUse = Render::ImageUse_Wrap,
                    Resource::ResourceUse resourceUse = Resource::Use_Bitmap)
    : Type(type), pHeap(heap), Use(imageUse), RUse(resourceUse),
      pLog(0), pFileOpener(0), pIFHRegistry(0), pMovie(NULL)
    {
    }

    void    SetStates(Log* log, FileOpener* opener, 
                      ImageFileHandlerRegistry* registry)
    {
        pFileOpener     = opener;
        pLog            = log;
        pIFHRegistry    = registry;
    }

    MemoryHeap* GetHeap() const { return pHeap ? pHeap : Memory::GetGlobalHeap(); }
    Log*        GetLog() const  { return pLog; }
    FileOpener* GetFileOpener() const  { return pFileOpener; }
    ImageFileHandlerRegistry* GetImageFileHandlerRegistry() const { return pIFHRegistry; }
};


struct ImageCreateExportInfo : public ImageCreateInfo
{
    ImageFileFormat     Format;
    const ExporterInfo* pExporterInfo;
    String              ExportName;
    ImageSize           TargetSize;

    ImageCreateExportInfo(MemoryHeap* heap = 0, unsigned imageUse = Render::ImageUse_Wrap,
                          Resource::ResourceUse resourceUse = Resource::Use_Bitmap) 
    : ImageCreateInfo(Create_ExportImage, heap, imageUse, resourceUse)
    { }
};


// Interface used to create Image objects in GFx player. By substituting this class
// you can control texture loading behavior.
//

class ImageCreator : public State
{
    // TextureManager is passed if it is available at image creation time. This can be
    // used to create textures directly out of image data; however, renderer must then be
    // configured before file loading.
    Ptr<TextureManager> pTextureManager;

public:

    ImageCreator(TextureManager* textureManager = 0);

    // Looks up image for "img://" protocol.
    virtual Image*  LoadProtocolImage(const ImageCreateInfo& info, const String& url);

    // Loads image from file.
    virtual Image*  LoadImageFile(const ImageCreateInfo& info, const String& url);

    // Loads exported image file.
    virtual Image*  LoadExportedImage(const ImageCreateExportInfo& info, const String& url);

    // Creates image
    //  - Returns image that matches expected protocol.
    virtual Image*  CreateImage(const ImageCreateInfo& info, ImageSource* source);


    // Identifies image creation loading stage. Load stage takes place before Bind stage.
    enum LoadStage {
        Stage_Load,
        Stage_Bind
    };
    // Bind can be returned if image creation result depends on internal state and
    // multiple image creators are used, such that specific result for all images
    // in the file is expected.
    //  - 
    virtual RefCountImpl* GetBindKey(LoadStage /*stage*/) { return 0; }

    // TextureManager access
    TextureManager* GetTextureManager() const { return pTextureManager; }

};



class ImageFileHandlerRegistry : public State, public Render::ImageFileHandlerRegistry
{
    friend class ImageCreator;
public:

    enum InitType 
    { 
        // No handlers are added when the ImageFileHandlerRegistry is created (default).
        NoInit, 

        // Adds all image handlers provided in GFx to the ImageFileHandlerRegistry. The actual handlers 
        // added are dependent on platform support, but may include any of SIF, DDS, TGA, PVR, PNG, KTX, 
        // JPEG, GTX, GXT.
        AddDefaultHandlers, 
    };

    ImageFileHandlerRegistry(InitType init = NoInit);

    void AddHandler(ImageFileHandler* handler);
};

inline void StateBag::SetImageCreator(ImageCreator *ptr) 
{ 
    SetState(State::State_ImageCreator, ptr); 
}
inline Ptr<ImageCreator> StateBag::GetImageCreator() const
{ 
    return *(ImageCreator*)GetStateAddRef(State::State_ImageCreator); 
}

inline void StateBag::SetImageFileHandlerRegistry(ImageFileHandlerRegistry* pmil) 
{ 
    SetState(State::State_ImageFileHandlerRegistry, pmil); 
}
inline Ptr<ImageFileHandlerRegistry> StateBag::GetImageFileHandlerRegistry() const 
{ 
    return *(ImageFileHandlerRegistry*)GetStateAddRef(State::State_ImageFileHandlerRegistry); 
}

}} // Scaleform::GFx

#endif
