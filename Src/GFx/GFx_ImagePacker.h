/**************************************************************************

Filename    :   GFx_ImagePacker.h
Content     :   
Created     :   
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_IMAGEPACKER_H
#define INC_SF_GFX_IMAGEPACKER_H

#include "GFx/GFx_MovieDef.h"
#include "GFx/GFx_ImageResource.h"
#include "Render/Render_RectPacker.h"

namespace Scaleform { namespace GFx {

// Track state of packer while binding a movie.

class ImagePacker : public RefCountBaseNTS<ImagePacker,Stat_Default_Mem>
{
public:
    virtual void SetBindData(MovieDefImpl::BindTaskData* pbinddata) = 0;
    //virtual void Add(ResourceDataNode* presNode, Render::ImageBase* pimage) = 0;
	virtual void AddImageFromResource(ImageResource* presource, const char* pexportname) = 0;
	virtual void AddResource(ResourceDataNode* presNode, ImageResource* presource) = 0;
    virtual ResourceDataNode* GetResourceDataNode(Resource* presource) = 0;
    virtual void Finish() = 0;
};


class ImagePackerImpl : public ImagePacker
{
    struct InputImage
    {
        ResourceDataNode    *pResNode;                  // needed to update binding with subreference to packed texture
        Render::ImageBase   *pImage;
        //Render::ImageData    ImgData;
		Ptr<Render::RawImage> pRawImage;
    };
	HashLH<Resource*, ResourceDataNode*>         InputResources;
    ArrayLH<InputImage>             InputImages;
    const ImagePackParams*          pImpl;
    ResourceId*                     pIdGen;             // shared with FontTextures
    Render::RectPacker              Packer;
    ImageCreator*                   pImageCreator;
    ImageCreateInfo                 mImageCreateInfo;
    MovieDefImpl::BindTaskData*     pBindData;

    void CopyImage(Render::ImageFormat format, Render::ImagePlane* pdest, const Render::ImagePlane* psrc, Render::RectPacker::RectType rect);

public:
    ImagePackerImpl(const ImagePackParams* pimpl, ResourceId* pidgen, ImageCreator* pic, ImageCreateInfo* pici)
        : pImpl(pimpl), pIdGen(pidgen), pImageCreator(pic), mImageCreateInfo(*pici) { }

    virtual void SetBindData(MovieDefImpl::BindTaskData* pbinddata) { pBindData = pbinddata; }
    //virtual void Add(ResourceDataNode* presNode, Render::ImageBase* pimage);
	virtual void AddImageFromResource(ImageResource* presource, const char* pexportname);
	virtual void AddResource(ResourceDataNode* presNode, ImageResource* presource);
    virtual ResourceDataNode* GetResourceDataNode(Resource* presource);
    virtual void Finish();
};

}} // namespace Scaleform { namespace GFx {

#endif
