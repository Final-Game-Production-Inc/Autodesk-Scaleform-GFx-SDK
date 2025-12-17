/**************************************************************************

Filename    :   PVR_ImageReader.cpp
Content     :   PVR Image file format reader implementation
Created     :   Mar 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "PVR_ImageFile.h"
#include "Render_ImageFileUtil.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace PVR {

// The following taken from PVRSDK's PVRTexture.h
enum PVR2PixelType
{
	MGLPT_ARGB_4444 = 0x00,
	MGLPT_ARGB_1555,
	MGLPT_RGB_565,
	MGLPT_RGB_555,
	MGLPT_RGB_888,
	MGLPT_ARGB_8888,
	MGLPT_ARGB_8332,
	MGLPT_I_8,
	MGLPT_AI_88,
	MGLPT_1_BPP,
	MGLPT_VY1UY0,
	MGLPT_Y1VY0U,
	MGLPT_PVRTC2,
	MGLPT_PVRTC4,
	MGLPT_PVRTC2_2,
	MGLPT_PVRTC2_4,

	OGL_RGBA_4444= 0x10,
	OGL_RGBA_5551,
	OGL_RGBA_8888,
	OGL_RGB_565,
	OGL_RGB_555,
	OGL_RGB_888,
	OGL_I_8,
	OGL_AI_88,
	OGL_PVRTC2,
	OGL_PVRTC4,

	// OGL_BGRA_8888 extension
	OGL_BGRA_8888,
	OGL_A_8,

	D3D_DXT1 = 0x20,
	D3D_DXT2,
	D3D_DXT3,
	D3D_DXT4,
	D3D_DXT5,

	D3D_RGB_332,
	D3D_AI_44,
	D3D_LVU_655,
	D3D_XLVU_8888,
	D3D_QWVU_8888,

	//10 bits per channel
	D3D_ABGR_2101010,
	D3D_ARGB_2101010,
	D3D_AWVU_2101010,

	//16 bits per channel
	D3D_GR_1616,
	D3D_VU_1616,
	D3D_ABGR_16161616,

	//HDR formats
	D3D_R16F,
	D3D_GR_1616F,
	D3D_ABGR_16161616F,

	//32 bits per channel
	D3D_R32F,
	D3D_GR_3232F,
	D3D_ABGR_32323232F,

	// Ericsson
	ETC_RGB_4BPP,
	ETC_RGBA_EXPLICIT,
	ETC_RGBA_INTERPOLATED,

	// DX10


	ePT_DX10_R32G32B32A32_FLOAT= 0x50,
	ePT_DX10_R32G32B32A32_UINT ,
	ePT_DX10_R32G32B32A32_SINT,

	ePT_DX10_R32G32B32_FLOAT,
	ePT_DX10_R32G32B32_UINT,
	ePT_DX10_R32G32B32_SINT,

	ePT_DX10_R16G16B16A16_FLOAT ,
	ePT_DX10_R16G16B16A16_UNORM,
	ePT_DX10_R16G16B16A16_UINT ,
	ePT_DX10_R16G16B16A16_SNORM ,
	ePT_DX10_R16G16B16A16_SINT ,

	ePT_DX10_R32G32_FLOAT ,
	ePT_DX10_R32G32_UINT ,
	ePT_DX10_R32G32_SINT ,

	ePT_DX10_R10G10B10A2_UNORM ,
	ePT_DX10_R10G10B10A2_UINT ,

	ePT_DX10_R11G11B10_FLOAT ,

	ePT_DX10_R8G8B8A8_UNORM ,
	ePT_DX10_R8G8B8A8_UNORM_SRGB ,
	ePT_DX10_R8G8B8A8_UINT ,
	ePT_DX10_R8G8B8A8_SNORM ,
	ePT_DX10_R8G8B8A8_SINT ,

	ePT_DX10_R16G16_FLOAT ,
	ePT_DX10_R16G16_UNORM ,
	ePT_DX10_R16G16_UINT ,
	ePT_DX10_R16G16_SNORM ,
	ePT_DX10_R16G16_SINT ,

	ePT_DX10_R32_FLOAT ,
	ePT_DX10_R32_UINT ,
	ePT_DX10_R32_SINT ,

	ePT_DX10_R8G8_UNORM ,
	ePT_DX10_R8G8_UINT ,
	ePT_DX10_R8G8_SNORM ,
	ePT_DX10_R8G8_SINT ,

	ePT_DX10_R16_FLOAT ,
	ePT_DX10_R16_UNORM ,
	ePT_DX10_R16_UINT ,
	ePT_DX10_R16_SNORM ,
	ePT_DX10_R16_SINT ,

	ePT_DX10_R8_UNORM,
	ePT_DX10_R8_UINT,
	ePT_DX10_R8_SNORM,
	ePT_DX10_R8_SINT,

	ePT_DX10_A8_UNORM,
	ePT_DX10_R1_UNORM,
	ePT_DX10_R9G9B9E5_SHAREDEXP,
	ePT_DX10_R8G8_B8G8_UNORM,
	ePT_DX10_G8R8_G8B8_UNORM,

	ePT_DX10_BC1_UNORM,
	ePT_DX10_BC1_UNORM_SRGB,

	ePT_DX10_BC2_UNORM,
	ePT_DX10_BC2_UNORM_SRGB,

	ePT_DX10_BC3_UNORM,
	ePT_DX10_BC3_UNORM_SRGB,

	ePT_DX10_BC4_UNORM,
	ePT_DX10_BC4_SNORM,

	ePT_DX10_BC5_UNORM,
	ePT_DX10_BC5_SNORM,

	//ePT_DX10_B5G6R5_UNORM,			// defined but obsolete - won't actually load in DX10
	//ePT_DX10_B5G5R5A1_UNORM,
	//ePT_DX10_B8G8R8A8_UNORM,
	//ePT_DX10_B8G8R8X8_UNORM,

	// OpenVG

	/* RGB{A,X} channel ordering */
	ePT_VG_sRGBX_8888  = 0x90,
	ePT_VG_sRGBA_8888,
	ePT_VG_sRGBA_8888_PRE,
	ePT_VG_sRGB_565,
	ePT_VG_sRGBA_5551,
	ePT_VG_sRGBA_4444,
	ePT_VG_sL_8,
	ePT_VG_lRGBX_8888,
	ePT_VG_lRGBA_8888,
	ePT_VG_lRGBA_8888_PRE,
	ePT_VG_lL_8,
	ePT_VG_A_8,
	ePT_VG_BW_1,

	/* {A,X}RGB channel ordering */
	ePT_VG_sXRGB_8888,
	ePT_VG_sARGB_8888,
	ePT_VG_sARGB_8888_PRE,
	ePT_VG_sARGB_1555,
	ePT_VG_sARGB_4444,
	ePT_VG_lXRGB_8888,
	ePT_VG_lARGB_8888,
	ePT_VG_lARGB_8888_PRE,

	/* BGR{A,X} channel ordering */
	ePT_VG_sBGRX_8888,
	ePT_VG_sBGRA_8888,
	ePT_VG_sBGRA_8888_PRE,
	ePT_VG_sBGR_565,
	ePT_VG_sBGRA_5551,
	ePT_VG_sBGRA_4444,
	ePT_VG_lBGRX_8888,
	ePT_VG_lBGRA_8888,
	ePT_VG_lBGRA_8888_PRE,

	/* {A,X}BGR channel ordering */
	ePT_VG_sXBGR_8888,
	ePT_VG_sABGR_8888 ,
	ePT_VG_sABGR_8888_PRE,
	ePT_VG_sABGR_1555,
	ePT_VG_sABGR_4444,
	ePT_VG_lXBGR_8888,
	ePT_VG_lABGR_8888,
	ePT_VG_lABGR_8888_PRE,

	// max cap for iterating
	END_OF_PIXEL_TYPES,

	MGLPT_NOTYPE = 0xff

};

/*****************************************************************************
* constants
*****************************************************************************/

//const UInt32 PVRTEX_MIPMAP		    = (1<<8);		// has mip map levels
//const UInt32 PVRTEX_TWIDDLE		    = (1<<9);		// is twiddled
//const UInt32 PVRTEX_BUMPMAP		    = (1<<10);		// has normals encoded for a bump map
//const UInt32 PVRTEX_TILING		    = (1<<11);		// is bordered for tiled pvr
//const UInt32 PVRTEX_CUBEMAP		    = (1<<12);		// is a cubemap/skybox
//const UInt32 PVRTEX_FALSEMIPCOL	    = (1<<13);		// are there false coloured MIP levels
//const UInt32 PVRTEX_VOLUME		    = (1<<14);		// is this a volume texture
const UInt32 PVRTEX_ALPHA			= (1<<15);		// v2.1 is there transparency info in the texture
//const UInt32 PVRTEX_VERTICAL_FLIP	= (1<<16);		// v2.1 is the texture vertically flipped

const UInt32 PVRTEX_PIXELTYPE		= 0xff;			// pixel type is always in the last 16bits of the flags
const UInt32 PVRTEX_IDENTIFIER	    = 0x21525650;	// the pvr identifier is the characters 'P','V','R'
//
//const UInt32 PVRTEX_V1_HEADER_SIZE  = 44;			// old header size was 44 for identification purposes
const UInt32 PVRTEX_V2_HEADER_SIZE  = 52;           // added, v2 header size.
//
//const UInt32 PVRTC2_MIN_TEXWIDTH	= 16;
//const UInt32 PVRTC2_MIN_TEXHEIGHT	= 8;
//const UInt32 PVRTC4_MIN_TEXWIDTH	= 8;
//const UInt32 PVRTC4_MIN_TEXHEIGHT	= 8;
//const UInt32 ETC_MIN_TEXWIDTH		= 4;
//const UInt32 ETC_MIN_TEXHEIGHT		= 4;
//const UInt32 DXT_MIN_TEXWIDTH		= 4;
//const UInt32 DXT_MIN_TEXHEIGHT		= 4;

enum PVR3PixelType
{
        ePVRTPF_PVRTCI_2bpp_RGB,
        ePVRTPF_PVRTCI_2bpp_RGBA,
        ePVRTPF_PVRTCI_4bpp_RGB,
        ePVRTPF_PVRTCI_4bpp_RGBA,
        ePVRTPF_PVRTCII_2bpp,
        ePVRTPF_PVRTCII_4bpp,
        ePVRTPF_ETC1,
        ePVRTPF_DXT1,
        ePVRTPF_DXT2,
        ePVRTPF_DXT3,
        ePVRTPF_DXT4,
        ePVRTPF_DXT5,

        //These formats are identical to some DXT formats.
        ePVRTPF_BC1 = ePVRTPF_DXT1,
        ePVRTPF_BC2 = ePVRTPF_DXT3,
        ePVRTPF_BC3 = ePVRTPF_DXT5,

        //These are currently unsupported:
        ePVRTPF_BC4,
        ePVRTPF_BC5,
        ePVRTPF_BC6,
        ePVRTPF_BC7,
        ePVRTPF_UYVY,
        ePVRTPF_YUY2,
        ePVRTPF_BW1bpp,
        ePVRTPF_SharedExponentR9G9B9E5,
        ePVRTPF_RGBG8888,
        ePVRTPF_GRGB8888,
        ePVRTPF_ETC2_RGB,
        ePVRTPF_ETC2_RGBA,
        ePVRTPF_ETC2_RGB_A1,
        ePVRTPF_EAC_R11_Unsigned,
        ePVRTPF_EAC_R11_Signed,
        ePVRTPF_EAC_RG11_Unsigned,
        ePVRTPF_EAC_RG11_Signed,
        ePVRTPF_NumCompressedPFs
};

enum EPVRTColourSpace
{
    ePVRTCSpacelRGB,
    ePVRTCSpacesRGB
};

enum EPVRTVariableType
{
    ePVRTVarTypeUnsignedByteNorm,
    ePVRTVarTypeSignedByteNorm,
    ePVRTVarTypeUnsignedByte,
    ePVRTVarTypeSignedByte,
    ePVRTVarTypeUnsignedShortNorm,
    ePVRTVarTypeSignedShortNorm,
    ePVRTVarTypeUnsignedShort,
    ePVRTVarTypeSignedShort,
    ePVRTVarTypeUnsignedIntegerNorm,
    ePVRTVarTypeSignedIntegerNorm,
    ePVRTVarTypeUnsignedInteger,
    ePVRTVarTypeSignedInteger,
    ePVRTVarTypeFloat,
    ePVRTVarTypeNumVarTypes
};

union PixelType
{
    struct LowHigh
    {
        UInt32 Low;
        UInt32 High;
    } Part;
    UInt64 PixelTypeID;
    UInt8 PixelTypeChar[8];
};

struct PVRHeaderInfo
{
    UInt32 HeaderSize;			/*!< size of the structure */
    UInt32 Height;				/*!< height of surface to be created */
    UInt32 Width;				/*!< width of input surface */
    UInt32 MipMapCount;			/*!< number of mip-map levels requested */
    UInt32 pfFlags;				/*!< pixel format flags */
    UInt32 TextureDataSize;		/*!< Total size in bytes */
    UInt32 BitCount;			/*!< number of bits per pixel  */
    UInt32 RBitMask;			/*!< mask for red bit */
    UInt32 GBitMask;			/*!< mask for green bits */
    UInt32 BBitMask;			/*!< mask for blue bits */
    UInt32 AlphaBitMask;		/*!< mask for alpha channel */
    UInt32 PVR;					/*!< magic number identifying pvr file */
    UInt32 NumSurfs;			/*!< the number of surfaces present in the pvr */   

    ImageFormat Format;         // Added: The image format.

    //PVR3
    UInt32  Version;
    UInt64  PVR3Format;
    UInt32  ColorSpace;
    UInt32  ChannelType;
    UInt32  Depth;
    UInt32  NumFases;
    UInt32  MetaDataSize;
};

// Temporary PVR image class used as a data source, allowing
// direct initialization of RawImage or Texture with image data.

class PVRFileImageSource : public FileImageSource
{
    UByte             ImageDesc; 
    PVRHeaderInfo   HeaderInfo;
    
public:
    PVRFileImageSource(File* file, ImageFormat format)
        : FileImageSource(file,format), ImageDesc(0)
    {
    }
    virtual      ~PVRFileImageSource(){}
    bool ReadHeader();
    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    virtual unsigned        GetMipmapCount() const { return Alg::Max<unsigned>(1, HeaderInfo.MipMapCount); }
};


// *** PVR Format loading

static const UByte* ParseUInt32(const UByte* buf, UInt32* pval)
{
    *pval = Alg::ByteUtil::LEToSystem(*(UInt32*)buf);    
    return buf + 4;
}

static const UByte* ParseUInt64(const UByte* buf, UInt64* pval)
{
    *pval = Alg::ByteUtil::LEToSystem(*(UInt64*)buf);    
    return buf + 8;
}

static bool Image_ParsePVRHeader(PVRHeaderInfo* pinfo, const UByte* buf, const UByte** pdata)
{
    SF_UNUSED(pdata);
    ImagePlane p0;
    UInt32 head;
    buf = ParseUInt32(buf, &head);
    if(head == 0x50565203 ||
        head == 0x03525650) //PVR3
    {
        pinfo->Version = 0x50565203; // always use PVRTEX3_IDENT.
        buf = ParseUInt32(buf, &pinfo->pfFlags);
        buf = ParseUInt64(buf,&pinfo->PVR3Format);
        buf = ParseUInt32(buf, &pinfo->ColorSpace);
        buf = ParseUInt32(buf, &pinfo->ChannelType);
        buf = ParseUInt32(buf, &pinfo->Height);
        buf = ParseUInt32(buf, &pinfo->Width);
        buf = ParseUInt32(buf, &pinfo->Depth);
        buf = ParseUInt32(buf, &pinfo->NumSurfs);
        buf = ParseUInt32(buf, &pinfo->NumFases);
        buf = ParseUInt32(buf, &pinfo->MipMapCount);
        buf = ParseUInt32(buf, &pinfo->MetaDataSize);
        pinfo->Format = Image_None;

        PixelType pt;
        pt.PixelTypeID = pinfo->PVR3Format;
        if (pt.Part.High == 0)
        {
            switch(pt.PixelTypeID)
            {
            case ePVRTPF_PVRTCI_2bpp_RGBA:
                pinfo->Format = Image_PVRTC_RGBA_2BPP;
                break;
            case ePVRTPF_PVRTCI_2bpp_RGB:
                pinfo->Format = Image_PVRTC_RGB_2BPP;
                break;
            case ePVRTPF_PVRTCI_4bpp_RGBA:
                pinfo->Format = Image_PVRTC_RGBA_4BPP;
                break;
            case ePVRTPF_PVRTCI_4bpp_RGB:
                pinfo->Format = Image_PVRTC_RGB_4BPP;
                break;
            case ePVRTPF_ETC1:
                pinfo->Format = Image_ETC1_RGB_4BPP; break;

            }
        }
        else
        {
            char* c = (char*)pt.PixelTypeChar;
            // can be replaced with if (pt.PixelTypeID == 578721384203708274)
            if (c[0]=='r' && c[1]=='g' && c[2] == 'b' && c[3] == 'a' && 
                c[4] == 8 && c[5] == 8 && c[6] == 8 && c[7] == 8)
                pinfo->Format = Image_R8G8B8A8;
            else if (c[0]=='r' && c[1]=='g' && c[2] == 'b' && c[3] == 0 && 
                c[4] == 8 && c[5] == 8 && c[6] == 8 && c[7] == 0)
                pinfo->Format = Image_R8G8B8;
            else if (c[0]=='a' && c[1]==0 && c[2] == 0 && c[3] == 0 && 
                c[4] == 8 && c[5] == 0 && c[6] == 0 && c[7] == 0)
                pinfo->Format = Image_A8;
        }
    }
    else //PVR2
    {
        pinfo->HeaderSize = head;
        SF_ASSERT( pinfo->HeaderSize == PVRTEX_V2_HEADER_SIZE ); 
    buf = ParseUInt32(buf, &pinfo->Height);
    buf = ParseUInt32(buf, &pinfo->Width);
    buf = ParseUInt32(buf, &pinfo->MipMapCount);
    buf = ParseUInt32(buf, &pinfo->pfFlags);
    buf = ParseUInt32(buf, &pinfo->TextureDataSize);
    buf = ParseUInt32(buf, &pinfo->BitCount);
    buf = ParseUInt32(buf, &pinfo->RBitMask);
    buf = ParseUInt32(buf, &pinfo->GBitMask);
    buf = ParseUInt32(buf, &pinfo->BBitMask);
    buf = ParseUInt32(buf, &pinfo->AlphaBitMask);
    buf = ParseUInt32(buf, &pinfo->PVR);
    buf = ParseUInt32(buf, &pinfo->NumSurfs);

    // Make sure that the magic is correct.
    if ( pinfo->PVR != PVRTEX_IDENTIFIER )
        return false;

    // Figure out the image format.
    pinfo->Format = Image_None;
    switch( pinfo->pfFlags & PVRTEX_PIXELTYPE )
    {
    case OGL_PVRTC2:
    case MGLPT_PVRTC2:
        if ( pinfo->pfFlags & PVRTEX_ALPHA)
            pinfo->Format = Image_PVRTC_RGBA_2BPP;
        else
            pinfo->Format = Image_PVRTC_RGB_2BPP;
        break;

    case OGL_PVRTC4:
    case MGLPT_PVRTC4:
        if ( pinfo->pfFlags & PVRTEX_ALPHA)
            pinfo->Format = Image_PVRTC_RGBA_4BPP;
        else
            pinfo->Format = Image_PVRTC_RGB_4BPP;
        break;
    case ETC_RGB_4BPP:        pinfo->Format = Image_ETC1_RGB_4BPP; break;
            
    // Uncompressed formats.
    case OGL_RGBA_8888:       pinfo->Format = Image_R8G8B8A8; break;
    case OGL_RGB_888:         pinfo->Format = Image_R8G8B8; break;
    case OGL_I_8:              pinfo->Format = Image_A8; break;        
    }
    }
    SF_ASSERT(pinfo->Format != Image_None); // Unsupported format
    return pinfo->Format != Image_None;
}

bool PVRFileImageSource::Decode( ImageData* pdest, CopyScanlineFunc copyScanline, void* arg ) const
{
    SF_UNUSED2(arg, copyScanline);
    if (!seekFileToDecodeStart())
        return false;
    if (HeaderInfo.Version == 0x50565203) // Skip metadata for PVR3
    {
        pFile->LSeek(FilePos + HeaderInfo.MetaDataSize);
    }
    if (pdest->HasSeparateMipmaps())
    {
        unsigned mipWidth = Size.Width;
        unsigned mipHeight = Size.Height;
        for (unsigned m = 0; m < pdest->GetMipLevelCount(); m++)
        {
            ImagePlane mipPlane;
            pdest->GetMipLevelPlane(m, 0, &mipPlane);
            //For compressed formats copy mipmap data directly (only compressed types are supported right now).
            SF_ASSERT(pdest->Format == Format);
            if (pFile->Read(mipPlane.pData, (int)mipPlane.DataSize) != (int)mipPlane.DataSize)
                return false;
            mipWidth /= 2;
            mipHeight /= 2;
        }
    }
    else
    {
        //SF_ASSERT(pdest->pPlanes->DataSize == HeaderInfo.TextureDataSize);
        if (pFile->Read(pdest->pPlanes->pData, (int)pdest->pPlanes->DataSize) != (int)pdest->pPlanes->DataSize)
            return false;
    }
    return true;

}

bool PVRFileImageSource::ReadHeader()
{
    UByte buf[256];
    if (pFile->Read(buf, PVRTEX_V2_HEADER_SIZE) != (int)PVRTEX_V2_HEADER_SIZE)
        return false;

    if (!Image_ParsePVRHeader(&HeaderInfo, buf, 0))
        return false;

    if (Format == Image_None)
        Format =  HeaderInfo.Format;
    Size = ImageSize(HeaderInfo.Width, HeaderInfo.Height);
    FilePos = pFile->LTell();
    return true;
}
bool FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerArgSize) const
{
    FileHeaderReader<PVRTEX_V2_HEADER_SIZE> header(file, headerArg, headerArgSize);
    if (!header)
        return false;
    UInt32* headerData = (UInt32*)header.GetPtr();
    if ((headerData[0]  != PVRTEX_V2_HEADER_SIZE || //PVR V2 
        headerData[11] != PVRTEX_IDENTIFIER) &&
        (headerData[0] != 0x50565203 &&          //PVR V3
        headerData[0] != 0x03525650)            
        )
        return false;
    return true;
}

ImageSource* FileReader::ReadImageSource(File* file, const ImageCreateArgs& args) const
{
    if (!file || !file->IsValid())
        return 0;

    PVRFileImageSource* source = SF_NEW PVRFileImageSource(file, args.Format);
    if (source && !source->ReadHeader())
    {
        source->Release();
        return 0;
    }
    return source; 
   
}

// Instance singleton.
FileReader FileReader::Instance;

}}}

