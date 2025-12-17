/**************************************************************************

Filename    :   GFx_ImagePacker.cpp
Content     :   
Created     :   
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_ImagePacker.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Render/Render_TextureUtil.h"

namespace Scaleform { namespace GFx {

using namespace Render;


//// Used in the packer to substitute embedded images
//class ImageSubstProvider : public GRefCountBase<ImageSubstProvider, GStat_Default_Mem>
//{
//public:
//	virtual Image* CreateImage(const char* pname) = 0;
//};


ImagePacker* ImagePackParams::Begin(ResourceId* pIdGen, ImageCreator* pic, ImageCreateInfo* pici) const
{
    return new ImagePackerImpl(this, pIdGen, pic, pici);
}

void ImagePackerImpl::AddResource(ResourceDataNode* presNode, ImageResource* presource )
{
	InputResources.Set(presource, presNode);
}


ResourceDataNode* ImagePackerImpl::GetResourceDataNode( Resource* presource )
{
    ResourceDataNode** ppresNode = InputResources.Get(presource);
    return ppresNode ? *ppresNode : 0;    
}


void ImagePackerImpl::AddImageFromResource( ImageResource* presource, const char *pexportname )
{
	if (pexportname && (strstr(pexportname, "-nopack") || strstr(pexportname, ".nopack") || strstr(pexportname, "_nopack")))
		return;
	ResourceDataNode** ppresNode = InputResources.Get(presource);
	if (ppresNode)
	{
		ImageBase *pimage = presource->GetImage();
		//ImageSubstProvider* pisp = pImpl->GetImageSubstProvider(); 
		//GPtr<GImage> pimage = 0;
		//GImage *peimage = ((GImageInfo*) pimageinfo)->GetImage();
		//GASSERT(peimage);
		//if (pisp && pexportname)
		//{
		//	pimage = *pisp->CreateImage(pexportname);
		//	if (!pimage)
		//	{
		//		fprintf(stderr, "\nWarning: Can't find substitution image for '%s'.\n ", pexportname);
		//	}
		//	else if ((pimage->Width != peimage->Width) || (pimage->Height != peimage->Height) )
		//	{
		//		fprintf(stderr, "\nWarning: Substitution image for '%s' has different resolution.\n " 
		//			"Embedded image will be used.\n", pexportname);
		//		pimage = peimage;
		//	}
		//}
		//if (!pimage)
		//	pimage = peimage;
		if (pimage && (pimage->GetFormat() == Render::Image_R8G8B8A8 ||
			pimage->GetFormat() == Render::Image_R8G8B8))
		{
			InputImage in;
			in.pImage = pimage;
			in.pResNode = *ppresNode;
			InputImages.PushBack(in);
			//InputImages.Back().ImgData.Initialize(pimage->GetFormat(), pimage->GetSize().Width, pimage->GetSize().Height,
			//    pimage->GetSize().Width * 4);

			InputImages.Back().pRawImage = *RawImage::Create(pimage->GetFormat(), 1, pimage->GetSize(), 0);

			ImageData tmpd;
			InputImages.Back().pRawImage->GetImageData(&tmpd);		
			if (!pimage->Decode(&tmpd))
			{
				InputImages.PopBack();
				SF_DEBUG_WARNING(1, "ImagePacker: Decode failed, not packing");
			}
		}
		else
			SF_DEBUG_WARNING(1, "ImagePacker: ImageCreator did not provide uncompressed RGBA Render::Image, not packing");
	}
}

void ImagePackerImpl::CopyImage( Render::ImageFormat format, Render::ImagePlane* pdest, const Render::ImagePlane* psrc, Render::RectPacker::RectType rect )
{
    const UByte* psrcrow = psrc->GetScanline(0);
    unsigned destpitch = (unsigned)pdest->Pitch;
    unsigned srcpitch = (unsigned)psrc->Pitch;

	bool startx = (rect.x > 0);
	bool starty = (rect.y > 0);
	if (startx)
		rect.x--;
	if (starty)
		rect.y--;
	bool endx = (rect.x + psrc->Width + (rect.x ? 0 : -1) >= pdest->Width-1) ? 0 : 1;
	bool endy = (rect.y + psrc->Height + (rect.y ? 0 : -1) >= pdest->Height-1) ? 0 : 1;
	UByte* pdestrow = pdest->GetScanline(rect.y) + 4*rect.x;

    SF_ASSERT(rect.x + psrc->Width + endx <= pdest->Width);
    SF_ASSERT(rect.y + psrc->Height + endy <= pdest->Height);

    switch(format)
    {
    case Render::Image_R8G8B8A8:
		if (!startx)
			pdestrow -= 4;
		if (starty)
		{
			if (startx) *((UInt32*) pdestrow) = *((const UInt32*) psrcrow);
			memcpy(pdestrow+4, psrcrow, srcpitch);
			if (endx) *((UInt32*) (pdestrow+4+psrc->Width*4)) = *((const UInt32*) (psrcrow+psrc->Width*4-4));
			pdestrow += destpitch;
		}
		for (unsigned j = 0; j < psrc->Height; j++, pdestrow += destpitch, psrcrow += srcpitch)
		{
			if (startx) *((UInt32*) pdestrow) = *((const UInt32*) psrcrow);
			memcpy(pdestrow+4, psrcrow, psrc->Width*4);
			if (endx) *((UInt32*) (pdestrow+4+psrc->Width*4)) = *((const UInt32*) (psrcrow+psrc->Width*4-4));
		}
		if (endy)
		{
			psrcrow -= srcpitch;
			if (startx) *((UInt32*) pdestrow) = *((const UInt32*) psrcrow);
			memcpy(pdestrow+4, psrcrow, srcpitch);
			if (endx) *((UInt32*) (pdestrow+4+psrc->Width*4)) = *((const UInt32*) (psrcrow+psrc->Width*4-4));
			pdestrow += destpitch;
		}
		break;

	case Render::Image_R8G8B8:
		destpitch -= psrc->Width*4 + (startx ? 4 : 0);
		srcpitch -= psrc->Width*3;
		if (starty)
		{
			if (startx)
			{
				pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
				pdestrow += 4;
			}
			for (unsigned i = 0; i < psrc->Width; i++, psrcrow+=3, pdestrow+=4)
			{
				pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
			}
			if (endx)
			{
				pdestrow[0] = psrcrow[-3]; pdestrow[1] = psrcrow[-2]; pdestrow[2] = psrcrow[-1];  pdestrow[3] = 255;
			}
			pdestrow += destpitch;
			psrcrow -= psrc->Width*3;
		}
		for (unsigned j = 0; j < psrc->Height; j++, pdestrow += destpitch, psrcrow += srcpitch)
		{
			if (startx)
			{
				pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
				pdestrow += 4;
			}
			for (unsigned i = 0; i < psrc->Width; i++, psrcrow+=3, pdestrow+=4)
			{
				pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
			}
			if (endx)
			{
				pdestrow[0] = psrcrow[-3]; pdestrow[1] = psrcrow[-2]; pdestrow[2] = psrcrow[-1];  pdestrow[3] = 255;
			}
		}
		if (endy)
		{
			psrcrow -= psrc->Pitch;
			if (startx)
			{
				pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
				pdestrow += 4;
			}
			for (unsigned i = 0; i < psrc->Width; i++, psrcrow+=3, pdestrow+=4)
			{
				pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
			}
			if (endx)
			{
				pdestrow[0] = psrcrow[-3]; pdestrow[1] = psrcrow[-2]; pdestrow[2] = psrcrow[-1];  pdestrow[3] = 255;
			}
		}
		break;

    default:
        SF_DEBUG_WARNING1(1, "GFx::ImagePacker: unsupported image format %d", format);
        break;
    }
}

void ImagePackerImpl::Finish()
{
    ImagePackParams::TextureConfig PackTextureConfig;
    pImpl->GetTextureConfig(&PackTextureConfig);
    Packer.SetWidth (2+PackTextureConfig.TextureWidth);
    Packer.SetHeight(2+PackTextureConfig.TextureHeight);
    Packer.Clear();

    for (unsigned i = 0; i < InputImages.GetSize(); i++)
        Packer.AddRect(2+InputImages[i].pImage->GetSize().Width, 2+InputImages[i].pImage->GetSize().Height, i);

    Packer.Pack();
/*
    if (mImageCreateInfo.
        .pRenderConfig)
        PackTextureConfig.SizeOptions = (mImageCreateInfo.pRenderConfig->GetRendererCapBits() & Render::Renderer::Cap_TexNonPower2) ? 
        ImagePackParams::PackSize_1 : ImagePackParams::PackSize_PowerOf2;
*/
    for (unsigned i = 0; i < Packer.GetNumPacks(); i++)
    {
        unsigned imgWidth = 0, imgHeight = 0;
        const Render::RectPacker::PackType& pack = Packer.GetPack(i);
        for (unsigned j = 0; j < pack.NumRects; j++)
        {
            const Render::RectPacker::RectType& rect = Packer.GetRect(pack, j);
            Render::ImagePlane imageIn;
			ImageData tmpd;
			InputImages[rect.Id].pRawImage->GetImageData(&tmpd);
            tmpd.GetPlane(0, &imageIn);
			
            Render::ImageRect irect (rect.x, rect.y, imageIn.GetSize());

            if (irect.x2 >= imgWidth) imgWidth = irect.x2;
            if (irect.y2 >= imgHeight) imgHeight = irect.y2;
        }
        if (PackTextureConfig.SizeOptions == ImagePackParams::PackSize_PowerOf2)
        {
            unsigned w = 1; while (w < imgWidth) { w <<= 1; }
            unsigned h = 1; while (h < imgHeight) { h <<= 1; }
            imgWidth = w;
            imgHeight = h;
        }
        else if (PackTextureConfig.SizeOptions == ImagePackParams::PackSize_4)
        {
			imgHeight = (imgHeight + 3) & ~3;
			imgWidth = (imgWidth + 3) & ~3;
        }
        else if (PackTextureConfig.SizeOptions == ImagePackParams::PackSize_128)
        {
            imgHeight = (imgHeight + 127) & ~127;
            imgWidth = (imgWidth + 127) & ~127;
        }

        if (PackTextureConfig.Square)
        {
           imgWidth = imgHeight = Alg::Max<unsigned>(imgHeight, imgWidth);
        }

        Ptr<RawImage> pPackImage = *RawImage::Create (Image_R8G8B8A8, 1, ImageSize(imgWidth, imgHeight), 0);
        ImageData     packImageData;
        ImagePlane    packImagePlane0;
        pPackImage->GetImageData(&packImageData);
        packImageData.GetPlane(0, &packImagePlane0);

        for (unsigned j = 0; j < pack.NumRects; j++)
        {
            const Render::RectPacker::RectType& rect = Packer.GetRect(pack, j);
            Render::ImagePlane imageIn;
            //InputImages[rect.Id].ImgData.GetPlane(0, &imageIn);
			ImageData tmpd;
			InputImages[rect.Id].pRawImage->GetImageData(&tmpd);
			tmpd.GetPlane(0, &imageIn);
            CopyImage(tmpd.GetFormat(), &packImagePlane0, &imageIn, rect);
        }

        ResourceId          textureId = pIdGen->GenerateNextId();

        //XXX mImageCreateInfo.SetTextureUsage(0);
        Ptr<Image>   pimageInfo = *pImageCreator->CreateImage(mImageCreateInfo,
            Ptr<WrapperImageSource>(*new WrapperImageSource(pPackImage)));
        Ptr<ImageResource> pimageRes  = 
            *SF_HEAP_NEW(mImageCreateInfo.pHeap) ImageResource(pimageInfo.GetPtr(), Resource::Use_Bitmap);

        for (unsigned j = 0; j < pack.NumRects; j++)
        {
            const Render::RectPacker::RectType& rect = Packer.GetRect(pack, j);
            Render::ImageBase* pimagein = InputImages[rect.Id].pImage;
            Render::ImageRect irect (rect.x, rect.y, pimagein->GetSize());
            ResourceDataNode *presNode = InputImages[rect.Id].pResNode;

            Ptr<Resource> pres = 
                *SF_HEAP_NEW(mImageCreateInfo.pHeap) SubImageResource(pimageRes, textureId,                                                                      
                irect, mImageCreateInfo.pHeap);
            pBindData->SetResourceBindData(presNode, pres);
        }
    }
    
    for (unsigned i = 0; i < Packer.GetNumFailed(); i++)
    {
        RawImage* psrcImage =  InputImages[Packer.GetFailed(i)].pRawImage;
        unsigned imgWidth = psrcImage->GetSize().Width, imgHeight = psrcImage->GetSize().Height;
        
        if (PackTextureConfig.SizeOptions == ImagePackParams::PackSize_PowerOf2)
        {
            unsigned w = 1; while (w < imgWidth) { w <<= 1; }
            unsigned h = 1; while (h < imgHeight) { h <<= 1; }
            imgWidth = w;
            imgHeight = h;
        }
        else if (PackTextureConfig.SizeOptions == ImagePackParams::PackSize_4)
        {
            imgHeight = (imgHeight + 3) & ~3;
            imgWidth = (imgWidth + 3) & ~3;
        }
        else if (PackTextureConfig.SizeOptions == ImagePackParams::PackSize_128)
        {
            imgHeight = (imgHeight + 127) & ~127;
            imgWidth = (imgWidth + 127) & ~127;
        }
        
        if (PackTextureConfig.Square)
        {
            imgWidth = imgHeight = Alg::Max<unsigned>(imgHeight, imgWidth);
        }

        Render::RectPacker::RectType rect;
        rect.Id = rect.x = rect.y = 0;
        Ptr<RawImage> pPackImage = *RawImage::Create(Image_R8G8B8A8, 1, ImageSize(imgWidth, imgHeight), 0, mImageCreateInfo.pHeap);
        ImageData     packImageData;
        ImagePlane    packImagePlane0;
        pPackImage->GetImageData(&packImageData);
        packImageData.GetPlane(0, &packImagePlane0);

        Render::ImagePlane imageIn;
        ImageData tmpd;
        psrcImage->GetImageData(&tmpd);
        tmpd.GetPlane(0, &imageIn);

        CopyImage(tmpd.GetFormat(), &packImagePlane0, &imageIn, rect);

        ResourceId          textureId = pIdGen->GenerateNextId();
        Ptr<Image>   pimageInfo = *pImageCreator->CreateImage(mImageCreateInfo,
            Ptr<WrapperImageSource>(*new WrapperImageSource(pPackImage)));
        Ptr<ImageResource> pimageRes  = 
            *SF_HEAP_NEW(mImageCreateInfo.pHeap) ImageResource(pimageInfo.GetPtr(), Resource::Use_Bitmap);

        ResourceDataNode *presNode = InputImages[Packer.GetFailed(i)].pResNode;
        Render::ImageRect irect (rect.x, rect.y, psrcImage->GetSize());

        Ptr<Resource> pres = 
            *SF_HEAP_NEW(mImageCreateInfo.pHeap) SubImageResource(pimageRes, textureId,                                                                      
            irect, mImageCreateInfo.pHeap);
        pBindData->SetResourceBindData(presNode, pres);
    }


}
}} // namespace Scaleform { namespace GFx {
