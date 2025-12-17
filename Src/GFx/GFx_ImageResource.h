/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ImageResource.h
Content     :   Image resource representation for GFxPlayer
Created     :   January 30, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_IMAGERESOURCE_H
#define INC_SF_GFX_IMAGERESOURCE_H

#include "GFx/GFx_Resource.h"
#include "GFx/GFx_Loader.h"

#include "Kernel/SF_HeapNew.h"
#include "Render/Render_Image.h"

namespace Scaleform { namespace GFx {

// ***** ImageResource - image resource representation

struct ImageFileInfo : public ResourceFileInfo
{
    UInt16                  TargetWidth;
    UInt16                  TargetHeight;
    // Store Use, so that that we can pass it to ImageCreator 
    Resource::ResourceUse   Use;
    String                  ExportName;

    ImageFileInfo()
    {
        TargetWidth = TargetHeight = 0;
        Use = Resource::Use_Bitmap;
    }
    ImageFileInfo(const ImageFileInfo& other) : ResourceFileInfo(other)
    {
        TargetWidth = other.TargetWidth;
        TargetHeight = other.TargetHeight;
        Use = other.Use;
    }
};

class ImageResource : public Resource
{
protected:
    Render::ImageBase*  pImage; // addref/release to pImage are manual

    class ImageDelegate : public Render::ImageDelegate
    {
        ImageResource* GetImageResource() 
        {
            size_t delta  = (size_t)&reinterpret_cast<ImageResource*>(this)->Delegate - (size_t)this;
            return reinterpret_cast<ImageResource*>(((size_t)this) - delta);
        }
    public:

        virtual void AddRef()
        {
            GetImageResource()->AddRef();
        }
        virtual void Release()
        {
            GetImageResource()->Release();
        }
    }                       Delegate;

    // Key used to look up / resolve this object (contains file data).
    ResourceKey             Key;
    // What the resource is used for
    ResourceUse             UseType;

public:
    ImageResource(Render::ImageBase* pimageBase, ResourceUse use = Use_Bitmap) 
        : pImage(NULL)
    {
        UseType    = use;
        if (pimageBase)
        {
            if (pimageBase->GetImageType() != Render::ImageBase::Type_ImageBase)
                SetImageSource(static_cast<Render::ImageSource*>(pimageBase));
            else
                SetImage(static_cast<Render::Image*>(pimageBase));
        }
        SF_AMP_CODE(AmpServer::GetInstance().AddImage(this));
    }
    ImageResource(Render::ImageSource* pimageSrc, ResourceUse use = Use_Bitmap) 
        : pImage(NULL)
    {
        SetImageSource(pimageSrc);
        UseType    = use;
        SF_AMP_CODE(AmpServer::GetInstance().AddImage(this));
    }
    ImageResource(Render::Image* pimage, ResourceUse use = Use_Bitmap) 
        : pImage(NULL)
    {
        SetImage(pimage);
        UseType      = use;
        SF_AMP_CODE(AmpServer::GetInstance().AddImage(this));
    }
    ImageResource(Render::Image* pimage, const ResourceKey& key, ResourceUse use = Use_Bitmap) 
        : pImage(NULL)
    {
        SetImage(pimage);
        Key          = key;
        UseType      = use;
        SF_AMP_CODE(AmpServer::GetInstance().AddImage(this));
    }
    ~ImageResource()
    {
        SF_AMP_CODE(AmpServer::GetInstance().RemoveImage(this));
        if (pImage && pImage != &Delegate)
            pImage->Release();
    }

    // Returns the referenced image. All Render::ImageInfoBase derived classes are created
    // by ImageCreator::CreateImage or Render::ImageInfoBase::CreateSubImage. If the user
    // overrides both of those functions to return a custom class, they can use type-casts
    // to access they custom data structures.
    inline const Render::ImageBase*  GetImage() const
    {
        return pImage;
    }
    inline Render::ImageBase*  GetImage()
    {
        return pImage;
    }
    inline void         SetImageSource(Render::ImageSource* pimageSrc)
    {
        if (pImage && pImage != &Delegate) pImage->Release();
        pImage          = pimageSrc;
        Delegate.pImage = NULL;
        if (pImage) pImage->AddRef();
    }
    inline void         SetImage(Render::Image* pimage)
    {
        if (pImage && pImage != &Delegate) pImage->Release();
        Delegate.pImage = pimage;
        pImage          = &Delegate;
//         Delegate.pImage = NULL;
//         pImage          = pimage;
//         pImage->AddRef();
    }

    inline unsigned    GetWidth() const     { return pImage ? pImage->GetSize().Width : 0;  }
    inline unsigned    GetHeight() const    { return pImage ? pImage->GetSize().Height : 0;  }

#ifdef SF_AMP_SERVER
    inline UPInt       GetBytes(int* memRegion) const         { return pImage ? pImage->GetBytes(memRegion) : 0;  }
    inline UInt32      GetImageId() const       { return pImage ? pImage->GetImageId() : 0; }
    inline Render::ImageFormat GetImageFormat() const   { return pImage ? pImage->GetFormat() : Render::Image_None; }
#endif

    virtual ResourceId GetBaseImageId() { return ResourceId(0); }

    // Ref file info, if any.
    /*
        
    // MA: We may need to bring this back, however,
    // we can to do this by obtaining the info from key.
    // Instead, we would need to store it locally.

    const GFxImageFileInfo* GetFileInfo() const
    {
        // Image files
        return (const GFxImageFileInfo*)Key.GetFileInfo();
    }
    */

    const char* GetFileURL() const
    {
        // Image files
        return Key.GetFileURL();
    }

    // GImplement Resource interface.
    virtual ResourceKey GetKey()                        { return Key; }
    virtual ResourceUse GetImageUse()                   { return UseType; }
    virtual unsigned    GetResourceTypeCode() const     { return MakeTypeCode(RT_Image, UseType); }


    // *** Methods for creating image keys

    // Create a key for an image file.    
    static  ResourceKey  CreateImageFileKey(ImageFileInfo* pfileInfo,
                                               FileOpener* pfileOpener,
                                               ImageCreator* pimageCreator,
                                               MemoryHeap* pimageHeap);
};


struct SubImageResourceInfo : public RefCountBaseNTS<SubImageResourceInfo, Stat_Default_Mem>
{
    ResourceId          ImageId;
    Ptr<ImageResource>  Image;
    Render::ImageRect   Rect;

    UPInt  GetHashCode() const;
};

class SubImageResource : public ImageResource
{
    Render::ImageRect   Rect;
    ResourceId          BaseImageId;

public:
    SubImageResource(ImageResource* pbase, ResourceId baseid,
                        const Render::ImageRect& rect, MemoryHeap* pheap)
    :   ImageResource(
            Ptr<Render::Image>(*SF_HEAP_AUTO_NEW(pbase->GetImage()) Render::SubImage(static_cast<Render::Image*>(pbase->GetImage()), rect))),
        Rect(rect), BaseImageId(baseid)
    {
        SF_UNUSED2(pheap,pbase);
    }

    virtual ResourceId GetBaseImageId() { return BaseImageId; }
};

}} // namespace Scaleform { namespace GFx {

#endif

