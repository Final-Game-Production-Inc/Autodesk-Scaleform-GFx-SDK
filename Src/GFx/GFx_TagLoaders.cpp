/**************************************************************************

Filename    :   GFx_TagLoaders.cpp
Content     :   GFxPlayer tag loaders implementation
Created     :   June 30, 2005
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_TagLoaders.h"

#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
#include "GFx/GFx_ImageResource.h"
#include "GFx/GFx_ImageSupport.h"

//#include "Render/Render_JPEGUtil.h"

#include "Render/ImageFiles/Render_ImageFileUtil.h"
#include "Render/Render_GlyphParam.h"

#include "GFx/GFx_DisplayObject.h"

#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_PlayerImpl.h"

#include "GFx/GFx_ButtonDef.h"
#include "GFx/GFx_SpriteDef.h"

#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_MorphCharacter.h"

#include "GFx/GFx_ShapeSwf.h"
#include "GFx/GFx_Stream.h"
//#include "GFx/GFx_Styles.h"
#include "GFx/GFx_DisplayList.h"
#include "GFxConfig.h"

#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_Audio.h"
#include "GFx/Audio/GFx_SoundTagsReader.h"

#include "GFx/AMP/Amp_Server.h"

#include <string.h> // for memset
#include <float.h>

#ifdef SF_MATH_H
#include SF_MATH_H
#else
#include <math.h>
#endif

namespace Scaleform { namespace GFx {

// ***** Tag Loader Tables

TagLoaderFunctionType SWF_TagLoaderTable[Tag_SWF_TagTableEnd] =
{
    // [0]
    GFx_EndLoader,
    0,
    GFx_DefineShapeLoader,
    0,
    GFx_PlaceObjectLoader,
    GFx_RemoveObjectLoader,
    GFx_DefineBitsJpegLoader,
    GFx_ButtonCharacterLoader,
    GFx_JpegTablesLoader,
    GFx_SetBackgroundColorLoader,

    // [10]
    GFx_DefineFontLoader,
    GFx_DefineTextLoader,
    GFx_DoActionLoader,
    GFx_DefineFontInfoLoader,
#ifdef GFX_ENABLE_SOUND
    GFx_DefineSoundLoader,
    GFx_StartSoundLoader,
    0,
    GFx_ButtonSoundLoader,
    GFx_SoundStreamHeadLoader,
    GFx_SoundStreamBlockLoader,
#else
    0,
    0,
    0,
    0,
    0,
    0,
#endif

    // [20]
    GFx_DefineBitsLossless2Loader,
    GFx_DefineBitsJpeg2Loader,
    GFx_DefineShapeLoader,
    0,
    GFx_NullLoader,   // "protect" tag; we're not an authoring tool so we don't care.
    0,
    GFx_PlaceObject2Loader,
    0,
    GFx_RemoveObject2Loader,
    0,

    // [30]
    0,
    0,
    GFx_DefineShapeLoader,
    GFx_DefineTextLoader,
    GFx_ButtonCharacterLoader,
    GFx_DefineBitsJpeg3Loader,
    GFx_DefineBitsLossless2Loader,
    GFx_DefineEditTextLoader,
    0,    
    GFx_SpriteLoader,

    // [40]
    0,
    0,
    0,
    GFx_FrameLabelLoader,
    0,
#ifdef GFX_ENABLE_SOUND
    GFx_SoundStreamHeadLoader,
#else
    0,
#endif
    GFx_DefineShapeMorphLoader,
    0,
    GFx_DefineFontLoader,
    0,

    // [50]
    0,
    0,
    0,
    0,
    0,
    0,
    GFx_ExportLoader,
    GFx_ImportLoader,
    0,
    GFx_DoInitActionLoader,

    // [60]
#ifdef GFX_ENABLE_VIDEO
    GFx_DefineVideoStream,
#else
    0,
#endif
    0,
    GFx_DefineFontInfoLoader,
    GFx_DebugIDLoader,
    0,
    0,
    GFx_SetTabIndexLoader,
    0,
    0,
    GFx_FileAttributesLoader,

    // [70]
    GFx_PlaceObject3Loader,
    GFx_ImportLoader,
    0,
    0, // DefineFontAlignZones
    GFx_CSMTextSettings,
    GFx_DefineFontLoader,
    0, // SymbolClass (AS3 only, see AS3Support)
    GFx_MetadataLoader,
    GFx_Scale9GridLoader,
    0,

    // [80]
    0,
    0,
    0, // DoABC (AS3 only, see AS3Support)
    GFx_DefineShapeLoader,
    GFx_DefineShapeMorphLoader,
    0,
    GFx_DefineSceneAndFrameLabelData,
    GFx_DefineBinaryData,
    0,// DefineFontName
    0,// StartSound2

    // [90]
    GFx_DefineBitsJpeg3Loader, // Jpeg4 (SWF10)
    0 // DefineFont4 (SWF10)
};

TagLoaderFunctionType GFx_GFX_TagLoaderTable[Tag_GFX_TagTableEnd - Tag_GFX_TagTableBegin] =
{
    // [1000]
    GFx_ExporterInfoLoader,
    0, // old DefineExternalImageLoader
    GFx_FontTextureInfoLoader,
    GFx_DefineExternalGradientImageLoader,
    GFx_DefineGradientMapLoader,
    GFx_DefineFontLoader,
#ifdef GFX_ENABLE_SOUND
    GFx_DefineExternalSoundLoader,
    GFx_DefineExternalStreamSoundLoader,
#else
    0,
    0,
#endif
    GFx_DefineSubImageLoader,
    GFx_DefineExternalImageLoader
};

bool RegisterTagLoader(unsigned tagType, TagLoaderFunctionType tagLoaderFunc)
{
    if (tagType >= sizeof(SWF_TagLoaderTable)/sizeof(SWF_TagLoaderTable[0]))
        return false;
    SWF_TagLoaderTable[tagType] = tagLoaderFunc;
    return true;
}

//
// Tag implementations
//

// Silently ignore the contents of this tag.
void    SF_STDCALL GFx_NullLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED2(p, tagInfo);
}

// Label the current frame of m with the name from the Stream.
void    SF_STDCALL GFx_FrameLabelLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    
    StringDH name(p->GetLoadHeap());
    p->GetStream()->ReadString(&name);
    p->AddFrameName(name, p->GetLogState());

    p->LogParse("  Frame label: \"%s\"\n", name.ToCStr());
}


void    SF_STDCALL GFx_SetBackgroundColorLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_SetBackgroundColor);
    SF_ASSERT(p);

    SetBackgroundColorTag* pt = p->AllocTag<SetBackgroundColorTag>();
    pt->Read(p);
    p->AddExecuteTag(pt);
}


// Load JPEG compression tables that can be used to load
// images further along in the Stream.
void    SF_STDCALL GFx_JpegTablesLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_JpegTables);

    Ptr<ImageFileHandlerRegistry> imageFileReg = p->GetLoadStates()->GetImageFileHandlerRegistry();
    if (!imageFileReg)
        p->LogError("Image file handler registry is not installed - can't load jpeg image data");
    else
    {
        Render::ImageFileHandler* jpegHandler = imageFileReg->GetReader(Render::ImageFile_JPEG);
        if (!jpegHandler)
            p->LogError("Jpeg System is not installed - can't load jpeg image data");
        else
        {
            SF_ASSERT(jpegHandler->IsReader() && jpegHandler->GetFormat() == Render::ImageFile_JPEG);

            // When Flash CS3 saves SWF8-, it puts this tag with zero length (no data in it).
            // In this case we should do nothing here, and create the JPEG image differently
            // in GFx_DefineBitsJpegLoader (AB)
            if (tagInfo.TagLength > 0)
            {
                Ptr<Render::JPEG::TablesHeader> jpegHeaders = *SF_HEAP_NEW(p->GetLoadHeap()) 
                    Render::JPEG::TablesHeader(p->GetLoadHeap(), tagInfo.TagLength);
                p->GetStream()->ReadToBuffer(jpegHeaders->Data, tagInfo.TagLength);
                p->SetJpegHeader(jpegHeaders);
            }
        }
    }
}


// A JPEG image without included tables; those should be in an
// existing JPEGInput object stored in the pMovie.
void    SF_STDCALL GFx_DefineBitsJpegLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineBitsJpeg);

    UInt16  bitmapResourceId = p->GetStream()->ReadU16();

    //
    // Read the image data.
    //
    Ptr<Render::ImageSource> pimageSrc;

    if (p->IsLoadingImageData())
    {
        Ptr<ImageFileHandlerRegistry> imageFileReg = p->GetLoadStates()->GetImageFileHandlerRegistry();
        if (!imageFileReg)
            p->LogError("Image file handler registry is not installed - can't load jpeg image data");
        else
        {
            Render::ImageFileHandler* jpegHandler = imageFileReg->GetReader(Render::ImageFile_JPEG);
            if (!jpegHandler)
                p->LogError("Jpeg System is not installed - can't load jpeg image data");
            else
            {
                SF_ASSERT(jpegHandler->IsReader() && jpegHandler->GetFormat() == Render::ImageFile_JPEG);
                Render::JPEG::AbstractReader* jpegReader = static_cast<Render::JPEG::AbstractReader*>(jpegHandler);

                p->GetStream()->SyncFileStream();

                Render::JPEG::TablesHeader* pth =  p->GetJpegHeader();
                if (pth)
                {
                    //pjin->DiscardPartialBuffer();

                    Render::ImageCreateArgs args;
                    args.pHeap = p->GetLoadHeap();
                    //args.Use = info.Use;
                    //args.pManager = pTextureManager; //?
                    pimageSrc = *jpegReader->CreateImageSource(p->GetStream()->GetUnderlyingFile(), args, pth, tagInfo.TagLength - 2);
                }
                else
                {
                    // if tag 8 was not loaded or has zero length - just read
                    // jpeg as usually.
                    Render::ImageCreateArgs args;
                    args.pHeap = p->GetLoadHeap();
                    //args.Use = info.Use;
                    //args.pManager = pTextureManager; //?
                    pimageSrc = *jpegReader->CreateImageSource(p->GetStream()->GetUnderlyingFile(), args, 0, tagInfo.TagLength - 2);
                }
            }
        }
    }
    else
    {
        // Empty image.
    }

    // Create a unique resource for the image and add it.
    p->AddImageResource(ResourceId(bitmapResourceId), pimageSrc);
}


void    SF_STDCALL GFx_DefineBitsJpeg2Loader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineBitsJpeg2);
    
    UInt16  bitmapResourceId = p->ReadU16();

    p->LogParse("  GFx_DefineBitsJpeg2Loader: charid = %d pos = 0x%x\n",
                bitmapResourceId, p->Tell());

    //
    // Read the image data.
    //
    Ptr<Render::ImageSource> pimageSrc;

    if (p->IsLoadingImageData())
    {
        Ptr<ImageFileHandlerRegistry> imageFileReg = p->GetLoadStates()->GetImageFileHandlerRegistry();
        if (!imageFileReg)
            p->LogError("Image file handler registry is not installed - can't load jpeg image data");
        else
        {
            Render::ImageFileHandler* jpegHandler = imageFileReg->GetReader(Render::ImageFile_JPEG);
            if (!jpegHandler)
                p->LogError("Jpeg System is not installed - can't load jpeg image data");
            else
            {
                SF_ASSERT(jpegHandler->IsReader() && jpegHandler->GetFormat() == Render::ImageFile_JPEG);
                Render::JPEG::AbstractReader* jpegReader = static_cast<Render::JPEG::AbstractReader*>(jpegHandler);
                Render::ImageCreateArgs args;
                args.pHeap = p->GetLoadHeap();
                //args.Use = info.Use;
                //args.pManager = pTextureManager; //?
                pimageSrc = *jpegReader->CreateImageSource(p->GetStream()->GetUnderlyingFile(), args, 0, tagInfo.TagLength - 2, true);
            }
        }
    }
    else
    {
        // Empty image.
    }

    // Create a unique resource for the image and add it.
    p->AddImageResource(ResourceId(bitmapResourceId), pimageSrc);
}


// Redundant code to compute GFx_UndoPremultiplyTable.
//#if defined(SF_ENABLE_ZLIB)
//// Helpers used for un-premultiplication.
//// SFW RGB+A data is stored with RGB channels pre-multiplied by alpha.
//// This lookup table is used to convert those efficiently.
//static int *    GFx_GetPremultiplyAlphaUndoTable()
//{
//    static  int     PremultiplyTable[256];
//    static  bool    TableValid = 0;
//
//    if (!TableValid)
//    {
//        PremultiplyTable[0] = 255 * 256;
//        for(unsigned i=1; i<=255; i++)
//        {
//            PremultiplyTable[i] = (255 * 256) / i;
//        }
//        TableValid = 1;
//
//FILE* fd = fopen("dt", "wt");
//for(unsigned i=0; i<=255; i++)
//{
//if(i)
//{
//    if(((i * PremultiplyTable[i]) >> 8) > 255)
//    {
//fprintf(fd, "\n========", PremultiplyTable[i]);
//    }
//}
//fprintf(fd, "%5d,", PremultiplyTable[i]);
//}
//fclose(fd);
//
//    }
//    return PremultiplyTable;
//}
//#endif


//static UInt16 GFx_UndoPremultiplyTable[256] = 
//{
//    65280,65280,32640,21760,16320,13056,10880, 9325, 8160, 7253, 6528, 5934, 5440, 5021, 4662, 4352, 
//     4080, 3840, 3626, 3435, 3264, 3108, 2967, 2838, 2720, 2611, 2510, 2417, 2331, 2251, 2176, 2105, 
//     2040, 1978, 1920, 1865, 1813, 1764, 1717, 1673, 1632, 1592, 1554, 1518, 1483, 1450, 1419, 1388, 
//     1360, 1332, 1305, 1280, 1255, 1231, 1208, 1186, 1165, 1145, 1125, 1106, 1088, 1070, 1052, 1036, 
//     1020, 1004,  989,  974,  960,  946,  932,  919,  906,  894,  882,  870,  858,  847,  836,  826,  
//      816,  805,  796,  786,  777,  768,  759,  750,  741,  733,  725,  717,  709,  701,  694,  687,  
//      680,  672,  666,  659,  652,  646,  640,  633,  627,  621,  615,  610,  604,  598,  593,  588,  
//      582,  577,  572,  567,  562,  557,  553,  548,  544,  539,  535,  530,  526,  522,  518,  514,  
//      510,  506,  502,  498,  494,  490,  487,  483,  480,  476,  473,  469,  466,  462,  459,  456,  
//      453,  450,  447,  444,  441,  438,  435,  432,  429,  426,  423,  421,  418,  415,  413,  410,  
//      408,  405,  402,  400,  398,  395,  393,  390,  388,  386,  384,  381,  379,  377,  375,  373,  
//      370,  368,  366,  364,  362,  360,  358,  356,  354,  352,  350,  349,  347,  345,  343,  341,  
//      340,  338,  336,  334,  333,  331,  329,  328,  326,  324,  323,  321,  320,  318,  316,  315,  
//      313,  312,  310,  309,  307,  306,  305,  303,  302,  300,  299,  298,  296,  295,  294,  292,  
//      291,  290,  288,  287,  286,  285,  283,  282,  281,  280,  278,  277,  276,  275,  274,  273,  
//      272,  270,  269,  268,  267,  266,  265,  264,  263,  262,  261,  260,  259,  258,  257,  256
//};


// Helper routine to undo pre-multiply by alpha, stored in SWF files.
//SF_INLINE void    UndoPremultiplyAlpha(UByte *prgb, UByte a)
//{
//    //int undoVal = GFx_PremultiplyTable[a]; //GFx_GetPremultiplyAlphaUndoTable()[a];
//    //// This can probably be optimized... (with binary ops and/or SSE/MMX)
//    //prgb[0] =  UByte( G_Clamp<int>((undoVal * (int)prgb[0]) >> 8, 0, 255) );
//    //prgb[1] =  UByte( G_Clamp<int>((undoVal * (int)prgb[1]) >> 8, 0, 255) );
//    //prgb[2] =  UByte( G_Clamp<int>((undoVal * (int)prgb[2]) >> 8, 0, 255) );
//
//    unsigned undoVal = GFx_UndoPremultiplyTable[a];
//    prgb[0] = UByte((undoVal * ((prgb[0] <= a) ? prgb[0] : a)) >> 8);
//    prgb[1] = UByte((undoVal * ((prgb[1] <= a) ? prgb[1] : a)) >> 8);
//    prgb[2] = UByte((undoVal * ((prgb[2] <= a) ? prgb[2] : a)) >> 8);
//}

// loads a DefineBitsJpeg3 tag. This is a jpeg file with an alpha
// channel using zlib compression.
void    SF_STDCALL GFx_DefineBitsJpeg3Loader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineBitsJpeg3 || tagInfo.TagType == Tag_DefineBitsJpeg4);

    UInt16  bitmapResourceId = p->ReadU16();

    UInt32  jpegSize      = p->ReadU32();
    if (tagInfo.TagType == Tag_DefineBitsJpeg4)
    {
        // deblocking parameter; ignored for now
        UInt16 db = p->ReadU16();
        p->LogParse("  GFx_DefineBitsJpeg4Loader: charid = %d pos = %d deblocking = %d\n",
            bitmapResourceId, p->Tell(), db);
    }
    else
    {
        p->LogParse("  GFx_DefineBitsJpeg3Loader: charid = %d pos = %d\n",
            bitmapResourceId, p->Tell());
    }

    // MA Debug
    //SF_DEBUG_MESSAGE1(1, "  GFx_DefineBitsJpeg3Loader: Begin jpeg at %d", alphaPosition - jpegSize);

    Ptr<Render::ImageSource> pimageSrc;
 
    if (p->IsLoadingImageData())
    {
#if !defined(SF_ENABLE_ZLIB) || !defined(SF_ENABLE_LIBJPEG)
        p->LogError("zlib or jpeglib are not linked - can't load jpeg+zipped image data!");
		SF_UNUSED(jpegSize);
#else
        Ptr<ImageFileHandlerRegistry> imageFileReg = p->GetLoadStates()->GetImageFileHandlerRegistry();
        if (!imageFileReg)
            p->LogError("Image file handler registry is not installed - can't load jpeg image data");
        else
        {
            ZlibSupportBase* pzlib = p->GetLoadStates()->GetZlibSupport();
            if (!pzlib)
                p->LogError("ZlibState is not set - can't load zipped image data");
            else 
            {
                Render::ImageFileHandler* jpegHandler = imageFileReg->GetReader(Render::ImageFile_JPEG);
                if (!jpegHandler)
                    p->LogError("Jpeg System is not installed - can't load jpeg image data");
                else
                {
                    SF_ASSERT(jpegHandler->IsReader() && jpegHandler->GetFormat() == Render::ImageFile_JPEG);
                    Render::JPEG::AbstractReader* jpegReader = static_cast<Render::JPEG::AbstractReader*>(jpegHandler);
                    Render::ImageCreateArgs args;
                    args.pHeap = p->GetLoadHeap();
                    //args.Use = info.Use;
                    //args.pManager = pTextureManager; //?
                    int jpegDataOff = p->Tell();
                    //p->SetPosition(jpegDataOff + alphaPosition);

                    UInt64 sz = tagInfo.TagLength - (jpegDataOff - tagInfo.TagDataOffset);

                    File* file = p->GetStream()->GetUnderlyingFile();
                    Ptr<Render::Image> img = 
                        *SF_HEAP_NEW(args.GetHeap()) MemoryBufferJpegImageWithZlibAlphas
                            (pzlib, jpegReader, jpegSize, Image_R8G8B8A8, Render::ImageSize(), args.Use, args.GetUpdateSync(),
                             file, (UPInt)sz);

                    pimageSrc = *jpegReader->CreateWrapperImageSource(img);
                }
            }
        }
#endif
    }
    else
    {
        // Empty image data.
    }


    // Create a unique resource for the image and add it.
    p->AddImageResource(ResourceId(bitmapResourceId), pimageSrc);
}


void    SF_STDCALL GFx_DefineBitsLossless2Loader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_DefineBitsLossless || tagInfo.TagType == Tag_DefineBitsLossless2);

    UInt16  bitmapResourceId = p->ReadU16();
    UByte   bitmapFormat     = p->ReadU8();    // 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    UInt16  width            = p->ReadU16();
    UInt16  height           = p->ReadU16();

    p->LogParse("  DefBitsLossless2: tagInfo.TagType = %d, id = %d, fmt = %d, w = %d, h = %d\n",
            tagInfo.TagType,
            bitmapResourceId,
            bitmapFormat,
            width,
            height);

    Ptr<Render::ImageSource> pimageSrc;

    if (p->IsLoadingImageData())
    {
#ifndef SF_ENABLE_ZLIB
        p->LogError("Zlib is not linked - can't load zipped image data");
#else
        ZlibSupportBase* pzlib = p->GetLoadStates()->GetZlibSupport();
        if (!pzlib)
            p->LogError("Error: GFxZlibState is not set - can't load zipped image data\n");
        else 
        {
            if (tagInfo.TagType == Tag_DefineBitsLossless)
            {
                // RGB image data.

                if (bitmapFormat == 3)
                {
                    // 8-bit data, preceded by a palette.

                    UInt16 colorTableSize = p->ReadU8();
                    colorTableSize += 1;    // !! SWF stores one less than the actual size
                    int bufferBytes = tagInfo.TagDataOffset + tagInfo.TagLength - p->Tell();

                    pimageSrc = *SF_NEW ZlibImageSource(pzlib, p->GetUnderlyingFile(), ImageSize(width, height), 
                        ZlibImageSource::ColorMappedRGB, Render::Image_R8G8B8, colorTableSize, bufferBytes);
                }
                else if (bitmapFormat == 4)
                {
                    // 16 bits / pixel
                    int bufferBytes = tagInfo.TagDataOffset + tagInfo.TagLength - p->Tell();
                    pimageSrc = *SF_NEW ZlibImageSource(pzlib, p->GetUnderlyingFile(), ImageSize(width, height), 
                        ZlibImageSource::RGB16, Render::Image_R8G8B8, 0, bufferBytes);
                }
                else if (bitmapFormat == 5)
                {
                    // 32 bits / pixel, input is ARGB Format (???)
                    int bufferBytes = tagInfo.TagDataOffset + tagInfo.TagLength - p->Tell();
                    pimageSrc = *SF_NEW ZlibImageSource(pzlib, p->GetUnderlyingFile(), ImageSize(width, height), 
                        ZlibImageSource::RGB24, Render::Image_R8G8B8, 0, bufferBytes);
                }
            }
            else // Tag_DefineBitsLossless2
            {
                // RGBA image data.
                SF_ASSERT(tagInfo.TagType == Tag_DefineBitsLossless2);

                if (bitmapFormat == 3)
                {
                    // 8-bit data, preceded by a palette.
                    UInt16 colorTableSize = p->ReadU8();
                    colorTableSize += 1;    // !! SWF stores one less than the actual size
                    int bufferBytes = tagInfo.TagDataOffset + tagInfo.TagLength - p->Tell();

                    pimageSrc = *SF_NEW ZlibImageSource(pzlib, p->GetUnderlyingFile(), ImageSize(width, height), 
                        ZlibImageSource::ColorMappedRGBA, Render::Image_R8G8B8A8, colorTableSize, bufferBytes);
                }
                else if (bitmapFormat == 4)
                {
                    // should be 555.
                    // Is this combination not supported?

                    // 16 bits / pixel
                    int bufferBytes = tagInfo.TagDataOffset + tagInfo.TagLength - p->Tell();

                    pimageSrc = *SF_NEW ZlibImageSource(pzlib, p->GetUnderlyingFile(), ImageSize(width, height), 
                        ZlibImageSource::RGB16, Render::Image_R8G8B8A8, 0, bufferBytes);
                }
                else if (bitmapFormat == 5)
                {
                    // 32 bits / pixel, input is ARGB format
                    int bufferBytes = tagInfo.TagDataOffset + tagInfo.TagLength - p->Tell();
                    pimageSrc = *SF_NEW ZlibImageSource(pzlib, p->GetUnderlyingFile(), ImageSize(width, height), 
                        ZlibImageSource::RGBA, Render::Image_R8G8B8A8, 0, bufferBytes);
                }
            }
        } 
#endif // SF_ENABLE_ZLIB
    }
    else
    {
        // Empty image in icreateInfo.
    }
    

    // Create a unique resource for the image and add it.
    p->AddImageResource(ResourceId(bitmapResourceId), pimageSrc);    
}


void    SF_STDCALL GFx_DefineShapeLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT( tagInfo.TagType == Tag_DefineShape  ||
             tagInfo.TagType == Tag_DefineShape2 ||
             tagInfo.TagType == Tag_DefineShape3 ||
             tagInfo.TagType == Tag_DefineShape4 );

    UInt16  characterId = p->ReadU16();
    p->LogParse("  ShapeLoader: id = %d\n", characterId);

    Ptr<ConstShapeWithStyles> shp = *SF_HEAP_NEW_ID(p->GetLoadHeap(), StatMD_ShapeData_Mem) ConstShapeWithStyles;
    int shapeOffset = p->GetStream()->Tell();
    shp->Read(p, tagInfo.TagType, tagInfo.TagLength - (shapeOffset - tagInfo.TagDataOffset), true);

    Ptr<SwfShapeCharacterDef>  ch = 
        *SF_HEAP_NEW_ID(p->GetLoadHeap(), StatMD_CharDefs_Mem) SwfShapeCharacterDef(shp);



    p->LogParse("  bound rect:");      
    p->GetStream()->LogParseClass(ch->GetBoundsLocal());

    p->AddResource(ResourceId(characterId), ch.GetPtr());
}

void    SF_STDCALL GFx_DefineShapeMorphLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT( tagInfo.TagType == Tag_DefineShapeMorph ||
             tagInfo.TagType == Tag_DefineShapeMorph2 );
#ifdef SF_RENDER_ENABLE_MORPHING
    UInt16 characterId = p->ReadU16();
    p->LogParse("  ShapeMorphLoader: id = %d\n", characterId);

    Ptr<MorphCharacterDef> morph = 
        *SF_HEAP_NEW_ID(p->GetLoadHeap(), StatMD_CharDefs_Mem) MorphCharacterDef;
    morph->Read(p, tagInfo, true);
    p->AddResource(ResourceId(characterId), morph);
#else
    p->LogError("ShapeMorphLoader failed due to SF_NO_MORPHING_SUPPORT macro");
#endif
}


//
// FontResource loaders
//

// Load a DefineFont or DefineFont2 tag.
void    SF_STDCALL GFx_DefineFontLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT( tagInfo.TagType == Tag_DefineFont  ||
             tagInfo.TagType == Tag_DefineFont2 ||
             tagInfo.TagType == Tag_DefineFont3 ||
             tagInfo.TagType == Tag_DefineCompactedFont );

    UInt16  fontId = p->ReadU16();

    p->LogParse("  Font: id = %d\n", fontId);
    // Note that we always create FontData separately from font. In addition to
    // allowing for different font texture bindings, this lets us substitute
    // fonts based on names even when they are not imported.

    // FontComptactor is not compatible with stripped fonts that come from gfx files.
    bool stripped_fonts = false;
    const ExporterInfo* ei = p->GetExporterInfo();
    if (ei && ei->ExportFlags & ExporterInfo::EXF_GlyphsStripped)
        stripped_fonts = true;

    Ptr<Font> pf;
    if (tagInfo.TagType == Tag_DefineCompactedFont)
    {
#ifdef GFX_ENABLE_COMPACTED_FONTS
        FontDataCompactedGfx* pfd = SF_HEAP_NEW(p->GetLoadHeap()) FontDataCompactedGfx;
        pf = *pfd;
        pfd->Read(p, tagInfo);
#else
        p->LogError("Trying to load compacted font with SF_NO_COMPACTED_FONT_SUPPORT defined.");
        return;
#endif //#ifdef GFX_ENABLE_COMPACTED_FONTS
    }
#ifdef GFX_ENABLE_FONT_COMPACTOR
    else if ((tagInfo.TagType == Tag_DefineFont2 || tagInfo.TagType == Tag_DefineFont3) 
             && !stripped_fonts
             && p->GetLoadStates()->GetFontCompactorParams())
    {
        FontDataCompactedSwf* pfd = SF_HEAP_NEW(p->GetLoadHeap()) FontDataCompactedSwf;
        pf = *pfd;
        pfd->Read(p, tagInfo);
    }
#endif //#ifdef GFX_ENABLE_FONT_COMPACTOR
    else
    {
        FontData* pfd = SF_HEAP_NEW(p->GetLoadHeap()) FontData;
        pf = *pfd;
        pfd->Read(p, tagInfo);
    }
   
    p->AddFontDataResource(ResourceId(fontId), pf);
}


// Load a SwdId tag, a 16 byte unique hash that matches up with the hash stored in a .swd file.
void    SF_STDCALL GFx_DebugIDLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DebugID);

    String strSwdId;
    char acHex[3];
    for (UPInt i = 0; i < 16; ++i)
    {
        strSwdId += SFitoa(p->ReadU8(), acHex, 3, 16);
    }
    SF_AMP_CODE(AmpServer::GetInstance().AddSwf(p->GetLoadTaskData()->GetSwdHandle(), strSwdId, p->GetFileURL());)
}

// Load a DefineFontInfo tag.  This adds information to an
// existing FontResource.
void    SF_STDCALL GFx_DefineFontInfoLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineFontInfo || tagInfo.TagType == Tag_DefineFontInfo2);

    UInt16  fontId = p->ReadU16();

    // We need to get at the font data somehow!!!

    // Here we assume that the real font type is FontData. 
    // DefineFontInfo and DefineFontInfo2 tags only go with DefineFont tag which we 
    // don't convert to compacted form.
    FontData* pfd = (FontData*)p->GetFontData(ResourceId(fontId));

    if (pfd)
    {
        pfd->ReadFontInfo(p->GetStream(), tagInfo.TagType);
    }
    else
    {
        p->LogError("GFx_DefineFontInfoLoader - can't find FontResource w/ id %d", fontId);
    }
}

void    SF_STDCALL GFx_PlaceObjectLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT( tagInfo.TagType == Tag_PlaceObject );
    p->LogParse("  PlaceObject\n");
    Stream* pin = p->GetStream();
    UPInt dataSz = PlaceObjectTag::ComputeDataSize(pin);
    PlaceObjectTag* ptag = p->AllocTag<PlaceObjectTag>(dataSz);
    ExecuteTag::LoadData(pin, ptag->pData, dataSz);   
    ptag->CheckForCxForm(dataSz);
    p->AddExecuteTag(ptag);
}

void    SF_STDCALL GFx_PlaceObject2Loader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT( tagInfo.TagType == Tag_PlaceObject2 );
    p->LogParse("  PlaceObject2Tag\n");
    Stream* pin = p->GetStream();
    UPInt dataSz = PlaceObject2Tag::ComputeDataSize(pin, p->GetVersion());
    bool hasEventHandlers = PlaceObject2Tag::HasEventHandlers(pin);
    if (hasEventHandlers)
        dataSz += sizeof(PlaceObject2Tag::EventArrayType*);
    PlaceObject2Tag* ptag = NULL;

    SF_ASSERT(!hasEventHandlers || !(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3));

    // if tag has event handlers and this AS2 SWF, then we need to use pAS2Support to create
    // the appropriate tag.
    if (hasEventHandlers && !(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3))
    {
        if (!p->GetLoadStates()->pAS2Support)
            p->LogError("GFx_PlaceObject2Loader - AS2 support is not installed. Tag is skipped.");
        else
        {
            ptag = p->GetLoadStates()->pAS2Support->AllocPlaceObject2Tag(p, dataSz, (UInt8)p->GetVersion());
        }
    }
    else if (p->GetVersion() >= 6)
        ptag = p->AllocTag<PlaceObject2Tag>(dataSz);
    else
        ptag = p->AllocTag<PlaceObject2Taga>(dataSz);
    if (!ptag)
        return;

    if (hasEventHandlers)
    {
        ExecuteTag::LoadData(pin, ptag->pData, dataSz-sizeof(PlaceObject2Tag::EventArrayType*), sizeof(GFxPlaceObjectBase::EventArrayType*));
        PlaceObject2Tag::RestructureForEventHandlers(ptag->pData);
    }
    else
        ExecuteTag::LoadData(pin, ptag->pData, dataSz);
    p->AddExecuteTag(ptag);
}

void    SF_STDCALL GFx_PlaceObject3Loader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT( tagInfo.TagType == Tag_PlaceObject3 );
    p->LogParse("  PlaceObject3Tag\n");
    Stream* pin = p->GetStream();
    UPInt dataSz = PlaceObject3Tag::ComputeDataSize(pin);
    bool hasEventHandlers = PlaceObject2Tag::HasEventHandlers(pin);
    if (hasEventHandlers)
        dataSz += sizeof(PlaceObject3Tag::EventArrayType*);
    
    PlaceObject3Tag* ptag = NULL;

    SF_ASSERT(!hasEventHandlers || !(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3));

    // if tag has event handlers and this AS2 SWF, then we need to use pAS2Support to create
    // the appropriate tag.
    if (hasEventHandlers && !(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3))
    {
        if (!p->GetLoadStates()->pAS2Support)
            p->LogError("GFx_PlaceObject3Loader - AS2 support is not installed. Tag is skipped.");
        else
        {
            ptag = p->GetLoadStates()->pAS2Support->AllocPlaceObject3Tag(p, dataSz);
        }
    }
    else
        ptag = p->AllocTag<PlaceObject3Tag>(dataSz);
    if (!ptag)
        return;
    if (hasEventHandlers)
    {
        ExecuteTag::LoadData(pin, ptag->pData, dataSz-sizeof(PlaceObject3Tag::EventArrayType*), sizeof(PlaceObject3Tag::EventArrayType*));
        PlaceObject2Tag::RestructureForEventHandlers(ptag->pData);
    }
    else
        ExecuteTag::LoadData(pin, ptag->pData, dataSz);
    p->AddExecuteTag(ptag);
}


// Create and initialize a sprite, and add it to the pMovie.
void    SF_STDCALL GFx_SpriteLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_Sprite);
            
    ResourceId characterId = ResourceId(p->ReadU16());
    p->LogParse("  sprite\n  char id = %d\n", characterId.GetIdIndex());

    // @@ combine SpriteDef with MovieDefImpl
    Ptr<SpriteDef> ch = 
        *SF_HEAP_NEW_ID(p->GetLoadHeap(), StatMD_CharDefs_Mem) SpriteDef(p->GetDataDef_Unsafe());
    ch->Read(p, characterId);
    p->AddCharacter(characterId, ch);
}





//
// EndTag
//

// EndTag doesn't actually need to exist.

void    SF_STDCALL GFx_EndLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED2(p, tagInfo.TagType);
    SF_ASSERT(tagInfo.TagType == Tag_End);
    SF_ASSERT(p->Tell() == p->GetTagEndPosition());
}

void    SF_STDCALL GFx_RemoveObjectLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT( tagInfo.TagType == Tag_RemoveObject );

    RemoveObjectTag*   ptag = NULL;

    if (!(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3))
    {
        // AS2
        if (!p->GetLoadStates()->pAS2Support)
            p->LogError("GFx_RemoveObjectLoader - AS2 support is not installed. Tag is skipped.");
        else
        {
            ptag = p->GetLoadStates()->pAS2Support->AllocRemoveObjectTag(p);
        }
    }
    else
    {
        // AS3
        ptag = p->AllocTag<RemoveObjectTag>();
    }
    if (!ptag)
        return;

    ptag->Read(p);
    p->LogParse("  RemoveObject(%d, %d)\n", ptag->Id, ptag->Depth);
    p->AddExecuteTag(ptag);
}

void    SF_STDCALL GFx_RemoveObject2Loader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT( tagInfo.TagType == Tag_RemoveObject2 );

    RemoveObject2Tag*   ptag = NULL;
    if (!(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3))
    {
        // AS2
        if (!p->GetLoadStates()->pAS2Support)
            p->LogError("GFx_PlaceObject3Loader - AS2 support is not installed. Tag is skipped.");
        else
        {
            ptag = p->GetLoadStates()->pAS2Support->AllocRemoveObject2Tag(p);
        }
    }
    else
    {
        // AS3
        ptag = p->AllocTag<RemoveObject2Tag>();
    }
    if (!ptag)
        return;

    ptag->Read(p);
    p->LogParse("  RemoveObject2(%d)\n", ptag->Depth);
    p->AddExecuteTag(ptag);
}


void    SF_STDCALL GFx_ButtonCharacterLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT( tagInfo.TagType == Tag_ButtonCharacter ||
             tagInfo.TagType == Tag_ButtonCharacter2 );

    int characterId = p->ReadU16();
    p->LogParse("  button GFxCharacter loader: CharId = %d\n", characterId);

    Ptr<ButtonDef> ch = 
        *SF_HEAP_NEW_ID(p->GetLoadHeap(), StatMD_CharDefs_Mem) ButtonDef;
    ch->Read(p, tagInfo.TagType);
    p->AddResource(ResourceId(characterId), ch);
}

#ifdef GFX_ENABLE_VIDEO

void    SF_STDCALL GFx_DefineVideoStream(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_DefineVideoStream);
    LoadStates* pstates = p->GetLoadStates();
    Video::VideoBase* pvideoState = pstates->GetVideoPlayerState();
    if (pvideoState)
        pvideoState->ReadDefineVideoStreamTag(p, tagInfo);
    else
    {
        p->GetStream()->LogParse("GFx_DefineVideoStream: Video library is not set.\n");
        p->GetStream()->LogTagBytes();
    }
}

#endif

// Handling this tag is necessary for files produced by FlashPaper. This tag may be inside
// DefineSprite tag, and removing SetTabIndex by gfxeport causes 
// incorrect length of DefineSprite tag 
void    SF_STDCALL GFx_SetTabIndexLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_SetTabIndex);
    SF_ASSERT (4 == p->GetTagEndPosition() - p->Tell());    
    UInt16 depth = p->ReadU16();
    UInt16 tabIndex = p->ReadU16();
    SF_UNUSED2(depth, tabIndex);
    p->LogParse("SetTabIndex (unused) \n");
}

//
// *** Export and Import Tags
//

// Load an export Tag (for exposing internal resources of m)
void    SF_STDCALL GFx_ExportLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_Export);

    unsigned count = p->ReadU16();
    p->LogParse("  export: count = %d\n", count);

    // Read the exports.
    for (unsigned i = 0; i < count; i++)
    {
        UInt16      id          = p->ReadU16();
        StringDH symbolName(p->GetLoadHeap());

        p->GetStream()->ReadString(&symbolName);
        
        p->LogParse("  export: id = %d, name = %s\n", id, symbolName.ToCStr());

        ResourceId     rid(id);
        ResourceHandle hres;
        if (p->GetResourceHandle(&hres, rid))
        {
            // Add export to the list.
            p->ExportResource(symbolName, rid, hres);

            // Should we check export types?
            // This may be impossible with handles.
        }
        else         
        {
            // This is really a debug error, since we expect Flash files to
            // be consistent and include their exported characters.
           // SF_DEBUG_WARNING1(1, "Export loader failed to export resource '%s'",
           //                       psymbolName );
            p->LogError("Don't know how to export Resource '%s'",
                        symbolName.ToCStr());
        }        
    }
}


// Load an import Tag (for pulling in external resources)
void    SF_STDCALL GFx_ImportLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_Import || tagInfo.TagType == Tag_Import2);

    Stream * pin = p->GetStream();    

    String sourceUrl;
    pin->ReadString(&sourceUrl);

    if (tagInfo.TagType == Tag_Import2)
    {
        unsigned val = p->ReadU16();
        SF_UNUSED(val);
        SF_DEBUG_WARNING1(val != 1, "Unexpected attribute in ImportAssets2 - 0x%X instead of 1", val);
    }

    int       count = pin->ReadU16();

    p->LogParse( ((tagInfo.TagType != Tag_Import2) ?
                    "  importAssets: SourceUrl = %s, count = %d\n" :
                    "  importAssets2: SourceUrl = %s, count = %d\n" ),
                    sourceUrl.ToCStr(), count);
  
    ImportData* pimportData = p->AllocMovieDefClass<ImportData>();
    pimportData->Frame     = p->GetLoadingFrame();
    pimportData->SourceUrl = sourceUrl;

    //ImportData* pimportData = new ImportData(psourceUrl,
    //                                               pdataDef->GetLoadingFrame());   

    // Get the imports.
    for (int i = 0; i < count; i++)
    {
        String   symbolName;
        UInt16      id = pin->ReadU16();

        pin->ReadString(&symbolName);
        p->LogParse("  import: id = %d, name = %s\n", id, symbolName.ToCStr());

        // And add ResourceId so that it's handle can be referenced by new load operations.
        ResourceHandle rh = p->AddNewResourceHandle(ResourceId(id));
        // Add import symbol to data.
        pimportData->AddSymbol(symbolName.ToCStr(), id, rh.GetBindIndex());
    }

    // Pass completed ImportData to GFxMovieDataDef.
    p->AddImportData(pimportData);


    // Add InitAction import tag, so that init actions are processed correctly.
    GFxInitImportActions*  ptag = p->AllocTag<GFxInitImportActions>();
    ptag->SetImportIndex(pimportData->ImportIndex);
    p->AddInitAction(ResourceId(), ptag);  
}


//
// *** SWF 8
//

void    SF_STDCALL GFx_FileAttributesLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_FileAttributes);

    UInt16 attrVal = p->ReadU16();
    p->SetFileAttributes(attrVal);

    // Log values for the user.
#ifdef GFX_VERBOSE_PARSE
    if (attrVal)
    {
        p->LogParse("  fileAttr:");

        char separator = ' ';
        if (attrVal & MovieDef::FileAttr_UseNetwork)
        {
            p->LogParse("%cUseNetwork", separator);
            separator = ',';
        }
        if (attrVal & MovieDef::FileAttr_UseActionScript3)
        {
            p->LogParse("%cUseActionScript3", separator);
        }
        if (attrVal & MovieDef::FileAttr_HasMetadata)
        {
            p->LogParse("%cHasMetadata", separator);
        }
        p->LogParse("\n");
    }
#endif

}

void    SF_STDCALL GFx_MetadataLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_Metadata);

    // Determine the size of meta-data and read it into the buffer.
    int     size = p->GetTagEndPosition() - p->Tell();
    UByte*  pbuff = (UByte*)SF_ALLOC(size + 1, Stat_Default_Mem);
    if (pbuff)
    {           
        for(int i=0; i<size; i++)
            pbuff[i] = p->ReadU8();
        p->SetMetadata(pbuff, (unsigned)size);

        pbuff[Alg::Min<int>(size,255)] = 0;        
        p->LogParse("  metadata: %s\n", pbuff);
        SF_FREE(pbuff);       
    }
}

void    SF_STDCALL GFx_ExporterInfoLoader(LoadProcess* p, const TagInfo& tagInfo)
{   
    SF_ASSERT(tagInfo.TagType == Tag_ExporterInfo);    

    // This tag should no longer be called because it is assumed to always be processed 
    // by the file-header loader through the GFxExporterInfoImpl::ReadExporterInfoTag
    // function.
    SF_ASSERT(0);    
    SF_UNUSED2(p, tagInfo.TagType);

    // Utilizes the tag 1000 (unused in normal SWF):
    // See GFxExporterInfoImpl::ReadExporterInfoTag for implementation.

    //GFxExporterInfoImpl sinfo;
    // sinfo.ReadExporterInfoTag(p, tagInfo.TagType);
    // sinfo.SetData(version, (Loader::FileFormatType)bitmapFormat, pswf, pfx, flags);
    // m->SetExporterInfo(sinfo);
}


// A helper method that generates image file info and resource data for the
// image based on specified arguments, returning a handle.
static ResourceHandle GFx_CreateImageFileResourceHandle(
                            LoadProcess* p, ResourceId rid,
                            const char* pimageFileName, const char* pimageExportName, 
                            UInt16 bitmapFormat,
                            UInt16 targetWidth, UInt16 targetHeight)
{
    ResourceHandle rh;
    
    // Create a concatenated file name.
    /* // WE no longer translate the filename here, as it is done through
       // a local translator interface during image creation -
       // see (GFxImageResourceCreator::CreateResource).

    String         relativePath;

    // Determine the file name we should use base on a relative path.
    if (GFxURLBuilder::IsPathAbsolute(pimageFileName))
        relativePath = pimageFileName;
    else
    {
        relativePath = p->GetFileURL();
        if (!GFxURLBuilder::ExtractFilePath(&relativePath))
            relativePath = "";
        relativePath += pimageFileName;
    }
    */

    // Fill in file information.
    Ptr<ImageFileInfo> pfi = *new ImageFileInfo;
    if (pfi)
    {
        pfi->FileName       = pimageFileName;
        pfi->ExportName     = pimageExportName;
        pfi->pExporterInfo  = p->GetExporterInfo();
        pfi->Format         = (FileTypeConstants::FileFormatType) bitmapFormat;
        pfi->TargetWidth    = targetWidth;
        pfi->TargetHeight   = targetHeight;

        // !AB: need to set Use_FontTextures for images used as
        // font textures to provide correct conversion to A8 format,
        // if necessary.
        if (rid.GetIdType() == ResourceId::IdType_FontImage)
            pfi->Use = Resource::Use_FontTexture;

        // Add resource id and data.      
        rh = p->AddDataResource(rid,
            ImageFileResourceCreator::CreateImageFileResourceData(pfi));
    }
    return rh;
}


void    SF_STDCALL GFx_DefineExternalImageLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineExternalImage);

    // Utilizes the tag 1009 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1009
    // CharacterID      UI32
    // BitmapsFormat    UI16            0 - Default, as in 1001 tag
    //                                  1 - TGA
    //                                  2 - DDS
    // TargetWidth      UI16
    // TargetHeight     UI16
    // FileNameLen      UI8             without extension, only name
    // FileName         UI8[FileNameLen]

    Stream* pin = p->GetStream();

    unsigned    bitmapResourceId    = p->ReadU32();
    UInt16  bitmapFormat        = p->ReadU16();
    UInt16  origWidth           = p->ReadU16();
    UInt16  origHeight          = p->ReadU16();

    String imageExportName, imageFileName;
    pin->ReadStringWithLength(&imageExportName);
    pin->ReadStringWithLength(&imageFileName);

    pin->LogParse("  DefineExternalImage: tagInfo.TagType = %d, id = 0x%X, fmt = %d, name = '%s', exp = '%s', w = %d, h = %d\n",
        tagInfo.TagType,
        bitmapResourceId,
        bitmapFormat,
        imageFileName.ToCStr(),
        imageExportName.ToCStr(),
        origWidth,
        origHeight);

    // Register the image file info.
    GFx_CreateImageFileResourceHandle(p, ResourceId(bitmapResourceId & (0xFFFF | ResourceId::IdType_DynFontImage)),
                                      imageFileName.ToCStr(), imageExportName.ToCStr(), bitmapFormat,
                                      origWidth, origHeight);
}

void    SF_STDCALL GFx_DefineSubImageLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineSubImage);

    // utilizes the tag 1008 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1008
    // CharacterID      UI16
    // ImageCharacterID UI16
    // x1             UI16
    // y1              UI16
    // x2            UI16
    // y2           UI16

    unsigned bitmapResourceId   = p->ReadU16();
    unsigned baseImageId        = p->ReadU16();
    UInt16   left               = p->ReadU16();
    UInt16   top                = p->ReadU16();
    UInt16   right              = p->ReadU16();
    UInt16   bottom             = p->ReadU16();

    Ptr<SubImageResourceInfo> psubimage = *new SubImageResourceInfo;
    psubimage->ImageId = ResourceId(baseImageId | ResourceId::IdType_DynFontImage);
    psubimage->Rect = Render::ImageRect(left, top, right, bottom);

    ResourceData rdata = SubImageResourceCreator::CreateSubImageResourceData(psubimage);
    p->AddDataResource(ResourceId(bitmapResourceId), rdata);
}

#ifdef GFX_ENABLE_SOUND

void  SF_STDCALL GFx_StartSoundLoader(LoadProcess* p, const TagInfo& tagInfo)
// Load a StartSound tag.
{
    SF_UNUSED(tagInfo);    
    SF_ASSERT(tagInfo.TagType == Tag_StartSound);
    AudioBase* paudio = p->GetLoadStates()->GetAudio();
    if (paudio)
    {
        SF_ASSERT(paudio->GetSoundTagsReader());
        paudio->GetSoundTagsReader()->ReadStartSoundTag(p,tagInfo);
    }
    else
    {
        p->GetStream()->LogParse("GFx_StartSoundLoader: Audio library is not set.\n");
        p->GetStream()->LogTagBytes();
    }
}

void  SF_STDCALL GFx_DefineSoundLoader(LoadProcess* p, const TagInfo& tagInfo)
// Load a DefineSound tag.
{
    SF_UNUSED(tagInfo); 
    SF_ASSERT(tagInfo.TagType == Tag_DefineSound);
    AudioBase* paudio = p->GetLoadStates()->GetAudio();
    if (paudio)
    {
        SF_ASSERT(paudio->GetSoundTagsReader());
        paudio->GetSoundTagsReader()->ReadDefineSoundTag(p,tagInfo);
    }
    else
    {
        p->GetStream()->LogParse("GFx_DefineSoundLoader: Audio library is not set.\n");
        p->GetStream()->LogTagBytes();
    }
}

void    SF_STDCALL GFx_ButtonSoundLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo); 
    SF_ASSERT(tagInfo.TagType == Tag_ButtonSound);
    AudioBase* paudio = p->GetLoadStates()->GetAudio();
    if (paudio)
    {
        SF_ASSERT(paudio->GetSoundTagsReader());
        paudio->GetSoundTagsReader()->ReadButtonSoundTag(p,tagInfo);
    }
    else
    {
        p->GetStream()->LogParse("GFx_ButtonSoundLoader: Audio library is not set.\n");
        p->GetStream()->LogTagBytes();
    }

}
void SF_STDCALL GFx_SoundStreamHeadLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);    
    SF_ASSERT(tagInfo.TagType == Tag_SoundStreamHead || tagInfo.TagType == Tag_SoundStreamHead2);

    AudioBase* paudio = p->GetLoadStates()->GetAudio();
    if (paudio)
    {
        SF_ASSERT(paudio->GetSoundTagsReader());
        paudio->GetSoundTagsReader()->ReadSoundStreamHeadTag(p,tagInfo);
    }
    else
    {
        p->GetStream()->LogParse("GFx_SoundStreamHeadLoader: Audio library is not set.\n");
        p->GetStream()->LogTagBytes();
    }
}

void SF_STDCALL GFx_SoundStreamBlockLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);    
    SF_ASSERT(tagInfo.TagType == Tag_SoundStreamBlock);

    AudioBase* paudio = p->GetLoadStates()->GetAudio();
    if (paudio)
    {
        SF_ASSERT(paudio->GetSoundTagsReader());
        paudio->GetSoundTagsReader()->ReadSoundStreamBlockTag(p,tagInfo);
    }
    else
    {
        p->GetStream()->LogParse("GFx_SoundStreamBlockLoader: Audio library is not set.\n");
        p->GetStream()->LogTagBytes();
    }
}

void    SF_STDCALL GFx_DefineExternalSoundLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineExternalSound);

    AudioBase* paudio = p->GetLoadStates()->GetAudio();
    if (paudio)
    {
        SF_ASSERT(paudio->GetSoundTagsReader());
        paudio->GetSoundTagsReader()->ReadDefineExternalSoundTag(p,tagInfo);
    }
    else
    {
        p->GetStream()->LogParse("GFx_DefineExternalSoundLoader: Audio library is not set.\n");
        p->GetStream()->LogTagBytes();
    }
}

void    SF_STDCALL GFx_DefineExternalStreamSoundLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineExternalStreamSound);
    AudioBase* paudio = p->GetLoadStates()->GetAudio();
    if (paudio)
    {
        SF_ASSERT(paudio->GetSoundTagsReader());
        paudio->GetSoundTagsReader()->ReadDefineExternalStreamSoundTag(p,tagInfo);
    }
    else
    {
        p->GetStream()->LogParse("GFx_DefineExternalStreamSoundLoader: Audio library is not set.\n");
        p->GetStream()->LogTagBytes();
    }
}
#endif // SF_NO_SOUND

void    SF_STDCALL GFx_FontTextureInfoLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED2(p, tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_FontTextureInfo);

    // Glyphs' texture info tags
    // utilizes the tag 1002 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1002
    // TextureID        UI32            Texture ID
    // TextureFormat    UI16            0 - Default, as in 1001 tag
    //                                  1 - TGA
    //                                  2 - DDS
    // FileNameLen      UI8             name of file with texture's image (without extension)
    // FileName         UI8[FileNameLen]
    // TextureWidth     UI16
    // TextureHeight    UI16
    // PadPixels        UI8             
    // NominalGlyphSz   UI16            Nominal height of glyphs
    // NumTexGlyphs     UI16            Number of texture glyphs
    // TexGlyphs        TEXGLYPH[NumTexGlyphs]
    // NumFonts         UI16            Number of fonts using this texture
    // Fonts            FONTINFO[NumFonts]  Font infos
    //
    // FONTINFO
    // FontId           UI16
    // NumGlyphs        UI16            Number of texture glyphs in the font from the current texture
    // GlyphIndices     GLYPHIDX[NumGlyphs] Mapping of font glyph's indices to textures' ones (TexGlyphs)
    //
    // GLYPHIDX
    // IndexInFont      UI16            Index in font
    // IndexInTexture   UI16            Index in texture
    //
    // TEXGLYPH:
    // UvBoundsLeft     FLOAT
    // UvBoundsTop      FLOAT
    // UvBoundsRight    FLOAT
    // UvBoundsBottom   FLOAT
    // UvOriginX        FLOAT
    // UvOriginY        FLOAT

    Stream*      pin = p->GetStream();    
    MemoryHeap*    pheap = p->GetLoadHeap();
    
    unsigned    textureId       = pin->ReadU32();
    UInt16  bitmapFormat    = pin->ReadU16();    
    String imageFileName;
    pin->ReadStringWithLength(&imageFileName);

    // Load texture glyph configuration.
    FontPackParams::TextureConfig  tgc;
    tgc.TextureWidth   = pin->ReadU16();
    tgc.TextureHeight  = pin->ReadU16();
    tgc.PadPixels      = pin->ReadU8();
    tgc.NominalSize    = pin->ReadU16();

    if (pin->IsVerboseParse())
    {
        pin->LogParse("  FontTextureInfo: tagInfo.TagType = %d, id = 0x%X, fmt = %d, name = '%s', w = %d, h = %d\n",
                      tagInfo.TagType, textureId,
                      bitmapFormat, imageFileName.ToCStr(),
                      tgc.TextureWidth, tgc.TextureHeight);
    }


    // Fill in file information.
    SF_ASSERT((textureId & ResourceId::IdType_FontImage) == ResourceId::IdType_FontImage);
    ResourceHandle rh = GFx_CreateImageFileResourceHandle(
                                p, ResourceId(textureId),
                                imageFileName.ToCStr(), "", bitmapFormat,
                                (UInt16) tgc.TextureWidth, (UInt16) tgc.TextureHeight);   

    // Load texture glyphs info first
    Array<TextureGlyph> texGlyphsInTexture;
    unsigned numTexGlyphs = pin->ReadU16();
    unsigned i;

    if (pin->IsVerboseParse())
    {
        pin->LogParse("  PadPixels = %d, nominal glyph size = %d, numTexGlyphs = %d\n",
            tgc.PadPixels, tgc.NominalSize, numTexGlyphs);
    }

    for (i = 0; i < numTexGlyphs; ++i)
    {
        // load TEXGLYPH
        float uvBoundsLeft      = pin->ReadFloat();
        float uvBoundsTop       = pin->ReadFloat();
        float uvBoundsRight     = pin->ReadFloat();
        float uvBoundsBottom    = pin->ReadFloat();
        float uvOriginX         = pin->ReadFloat();
        float uvOriginY         = pin->ReadFloat();

        if (pin->IsVerboseParse())
        {
            pin->LogParse("  TEXGLYPH[%d]: uvBnd.x1 = %f, uvBnd.y1 = %f,"
                " uvBnd.x2 = %f, uvBnd.y2 = %f\n",
                i, uvBoundsLeft, uvBoundsTop, uvBoundsRight, uvBoundsBottom);
            pin->LogParse("                uvOrigin.x = %f, uvOrigin.y = %f\n",
                uvOriginX, uvOriginY);
        }

        // create TextureGlyph
        TextureGlyph texGlyph;
        if (rh.IsIndex())
        {
            texGlyph.BindIndex = rh.GetBindIndex();
        }
        else
        {
            if (rh.GetResourcePtr() && rh.GetResourcePtr()->GetResourceType() == Resource::RT_Image)
            {
                Image* pimage = static_cast<Image*>(static_cast<ImageResource*>(rh.GetResourcePtr())->GetImage());
                texGlyph.pImage = pimage;
            }
        }
        //texGlyph.SetImageResource(rh);
        texGlyph.UvBounds = RectF(uvBoundsLeft, uvBoundsTop,
                                  uvBoundsRight, uvBoundsBottom);
        texGlyph.UvOrigin = PointF(uvOriginX, uvOriginY);
        texGlyphsInTexture.PushBack(texGlyph);
    }

    // load fonts' info
    unsigned numFonts = pin->ReadU16();
    if (pin->IsVerboseParse())
    {
        pin->LogParse("  NumFonts = %d\n", numFonts);
    }
    
    for (i = 0; i < numFonts; ++i)
    {
        // Load FONTINFO
        UInt16  fontId = pin->ReadU16();
        Font*   pfd    = p->GetFontData(ResourceId(fontId));
        if (!pfd)
        {
            SF_DEBUG_WARNING1(1, "FontTextureInfoLoader: can't find font, fontId = (%d)\n",
                                  fontId);
            continue;
        }
        //FontData* pfd = static_cast<FontData*>(pf); //? are all fonts FontData here? (AB)

        // Get/Create font texture data.
        // When fonts are spread among multiple textures, texture glyphs would
        // already be created by the previous tag.
        Ptr<TextureGlyphData> ptextureGlyphData = static_cast<TextureGlyphData*>(pfd->GetTextureGlyphData());
        if (!ptextureGlyphData) 
        {
            if ((ptextureGlyphData = *SF_HEAP_NEW(pheap) TextureGlyphData(pfd->GetGlyphShapeCount(), true)))
            {
                ptextureGlyphData->SetTextureConfig(tgc);
                pfd->SetTextureGlyphData(ptextureGlyphData);
            }
        }
        
        if (ptextureGlyphData)
            ptextureGlyphData->AddTexture(ResourceId(textureId), rh);
        
        unsigned numGlyphsInFont = pin->ReadU16();
        for (unsigned i_GF = 0; i_GF < numGlyphsInFont; ++i_GF)
        {
            // load GLYPHIDX
            unsigned indexInFont    = pin->ReadU16();
            unsigned indexInTexture = pin->ReadU16();
            
            if (ptextureGlyphData)
                ptextureGlyphData->AddTextureGlyph(
                    indexInFont, texGlyphsInTexture[indexInTexture]);
        }
    }

    pin->LogParse("\n");
}


void    SF_STDCALL GFx_DefineExternalGradientImageLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineExternalGradient);

    // Utilizes the tag 1003 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1001
    // GradientID       UI16
    // BitmapsFormat    UI16            0 - Default, as in 1001 tag
    //                                  1 - TGA
    //                                  2 - DDS
    // GradientSize     UI16
    // FileNameLen      UI8             without extension, only name
    // FileName         UI8[FileNameLen]

    Stream*  pin              = p->GetStream();

    unsigned    gradientId       = pin->ReadU16();
    unsigned    bitmapResourceId = gradientId | ResourceId::IdType_GradientImage;
    UInt16      bitmapFormat     = pin->ReadU16();
    UInt16      gradSize         = pin->ReadU16();
    
    String   imageFileName;
    pin->ReadStringWithLength(&imageFileName);

    pin->LogParse("  DefineExternalGradientImage: tagInfo.TagType = %d, id = 0x%X, fmt = %d, name = '%s', size = %d\n",
        tagInfo.TagType,
        bitmapResourceId,
        bitmapFormat,
        imageFileName.ToCStr(),
        gradSize);

    // Register image resource data.
    GFx_CreateImageFileResourceHandle(p, ResourceId(bitmapResourceId),
                                      imageFileName.ToCStr(), "", bitmapFormat,
                                      0, 0);
    // ImageCreator::CreateImage will substute targetWidth/targetHeight with real
    // values (we pass 0,0 since we don't know either this is a linear or radial gradient)
}


void    SF_STDCALL GFx_DefineGradientMapLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);    
    SF_ASSERT(tagInfo.TagType == Tag_DefineGradientMap);

    p->LogWarning("Deprecated tag 1004 - DefineGradientMapLoader encountered, ignored");

    /*

    // Utilizes the tag 1004 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1004
    // NumGradients     UI16            
    // Indices          UI16[NumGradients]

    unsigned    numGradients = pin->ReadU16();

    pin->LogParse("  DefineGradientMap: tagInfo.TagType = %d, num = %d\n",
        tagInfo.TagType,
        numGradients);
    GradientParams* pgrParams = pin->GetLoader()->GetGradientParams();
    if (pgrParams == 0)
    {
        GradientParams grParams;
        pin->GetLoader()->SetGradientParams(&grParams);
        pgrParams = pin->GetLoader()->GetGradientParams();
    }
    SF_ASSERT(pgrParams);
    pgrParams->GradientMap.SetSize(numGradients);

    for (unsigned i = 0; i < numGradients; ++i)
    {
        UInt16 v = pin->ReadU16();
        pgrParams->GradientMap.pData[i] = v;
    }
    */
}


void SF_STDCALL GFx_Scale9GridLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineScale9Grid);

    Stream*  pin = p->GetStream();    

    RectF r;
    unsigned  refId = pin->ReadU16();
                  pin->ReadRect(&r);

    if (pin->IsVerboseParse())
    {
        p->LogParse("Scale9GridLoader, id=%d, x1=%d, y1=%d, x2=%d, y2=%d\n", 
            refId, int(r.x1), int(r.y1), int(r.x2), int(r.y2));
    }

    if (r.x1 >= r.x2)
    {
        p->LogWarning("Scale9Grid for resource=%d has negative width %f", refId, (r.x2 - r.x1) / 20);
        return;
    }

    if (r.y1 >= r.y2)
    {
        p->LogWarning("Scale9Grid for resource=%d has negative height %f", refId, (r.y2 - r.y1) / 20);
        return;
    }

    ResourceHandle handle;
    if (p->GetResourceHandle(&handle, ResourceId(refId)))
    {
        Resource* rptr = handle.GetResourcePtr();
        if (rptr)
        { 
            if (rptr->GetResourceType() == Resource::RT_SpriteDef)
            {
                SpriteDef* sp = static_cast<SpriteDef*>(rptr);
                sp->SetScale9Grid(r);
            }
            else if (rptr->GetResourceType() == Resource::RT_ButtonDef)
            {
                ButtonDef* b = static_cast<ButtonDef*>(rptr);
                b->SetScale9Grid(r);
            }
        }
    }
    
}

void    SF_STDCALL GFx_DoActionLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    if (!(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3))
    {
        if (!p->GetLoadStates()->pAS2Support)
            p->LogError("GFx_DoActionLoader - AS2 support is not installed. Tag is skipped.");
        else
        {
            p->GetLoadStates()->pAS2Support->DoActionLoader(p, tagInfo);
        }
    }
    else
        p->LogError("GFx_DoActionLoader - DoAction tag shouldn't appear in AS3 swf. Tag is skipped.");
}

void    SF_STDCALL GFx_DoInitActionLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    if (!(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3))
    {
        if (!p->GetLoadStates()->pAS2Support)
            p->LogError("GFx_DoInitActionLoader - AS2 support is not installed. Tag is skipped.");
        else
        {
            p->GetLoadStates()->pAS2Support->DoInitActionLoader(p, tagInfo);
        }
    }
    else
        p->LogError("GFx_DoInitActionLoader - DoInitAction tag shouldn't appear in AS3 swf. Tag is skipped.");
}

void    SF_STDCALL GFx_DefineSceneAndFrameLabelData(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineSceneAndFrameLabelData);
    if (p->HasScenes()) // not first call, ignore
        return;

    Stream* in = p->GetStream();
    UInt32 sceneCount = in->ReadVU32();

    p->LogParse("  Scene and Frame Label Data, numscenes = %d\n", sceneCount);

    for (UInt32 i = 0; i < sceneCount; ++i)
    {
        UInt32 off = in->ReadVU32();
        StringDH name(p->GetLoadHeap());
        in->ReadString(&name);

        p->LogParse("   Scene[%d] = { %d, \"%s\" }\n", i,  off, name.ToCStr());
        p->AddScene(name, off);
    }
    UInt32 frameLabelCount = in->ReadVU32();
    p->LogParse("  frameLabelCount = %d\n", frameLabelCount);
    unsigned sceneIdx = 0;
    MovieDataDef::SceneInfo* curScene = p->GetScene(sceneIdx);
    for (UInt32 i = 0; i < frameLabelCount; ++i)
    {
        UInt32 num = in->ReadVU32();
        StringDH label(p->GetLoadHeap());
        in->ReadString(&label);

        p->LogParse("   Label[%d] = { %d, \"%s\" }\n", i, num, label.ToCStr());

        // check if this frame within the current scene
        MovieDataDef::SceneInfo* nextScene;
        while((nextScene = p->GetScene(sceneIdx + 1)) != NULL && num >= nextScene->Offset)
        {
            curScene->NumFrames = nextScene->Offset;
            curScene = nextScene;
            ++sceneIdx;
        }
        curScene->AddFrameLabel(label, num);
    }
    curScene->NumFrames = p->GetTotalFrames() - curScene->Offset;
}

void    SF_STDCALL GFx_DefineBinaryData(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineBinaryData);

    p->LogError("Tag 'DefineBinaryData' (87) is not supported, potentially 'TLF text' fields are used. Switch to 'Classic Text'.");

    int characterId = p->ReadU16();
    p->LogParse("  DefineBinaryData: CharId = %d\n", characterId);
    p->ReadU32(); // skip reserved.

    if (p->IsVerboseParse())
    {
        p->GetStream()->LogTagBytes();
    }

    // create a fake character to prevent errors, such as 'resource with id=... not found'. 
    Ptr<ButtonDef> ch = 
        *SF_HEAP_NEW_ID(p->GetLoadHeap(), StatMD_CharDefs_Mem) ButtonDef;
    p->AddResource(ResourceId(characterId), ch);
}
}} // Scaleform::GFx

