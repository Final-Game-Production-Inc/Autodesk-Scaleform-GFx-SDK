/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_TagLoaders.h
Content     :   GFxPlayer tag loader implementation
Created     :   June 30, 2005
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_TAGLOADERS_H
#define INC_SF_GFX_TAGLOADERS_H

#include "Kernel/SF_Types.h"
#include "GFx/GFx_Tags.h"

namespace Scaleform { namespace GFx {

// *** Forward Decralations
class Stream;
class LoadProcess;
class MovieDataDef;
struct TagInfo;

//
// ***** File Loader callback functions.
//

// Tag loader functions.
void    SF_STDCALL GFx_NullLoader(LoadProcess* p, const TagInfo& tagInfo);

void    SF_STDCALL GFx_SetBackgroundColorLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_JpegTablesLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineBitsJpegLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineBitsJpeg2Loader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineBitsJpeg3Loader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineShapeLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineShapeMorphLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineFontLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DebugIDLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineFontInfoLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineTextLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineEditTextLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_PlaceObjectLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_PlaceObject2Loader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_PlaceObject3Loader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineBitsLossless2Loader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_SpriteLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_EndLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_RemoveObjectLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_RemoveObject2Loader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DoActionLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_ButtonCharacterLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_FrameLabelLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_ExportLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_ImportLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineSoundLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_StartSoundLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_ButtonSoundLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_SoundStreamHeadLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_SoundStreamBlockLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DoInitActionLoader(LoadProcess* p, const TagInfo& tagInfo);
#ifdef GFX_ENABLE_VIDEO
void    SF_STDCALL GFx_DefineVideoStream(LoadProcess* p, const TagInfo& tagInfo);
#endif
void    SF_STDCALL GFx_SetTabIndexLoader(LoadProcess* p, const TagInfo& tagInfo);
// SWF 8
void    SF_STDCALL GFx_MetadataLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_FileAttributesLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_CSMTextSettings(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_Scale9GridLoader(LoadProcess* p, const TagInfo& tagInfo);
// Stripper custom tags loaders
void    SF_STDCALL GFx_ExporterInfoLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineExternalImageLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_FontTextureInfoLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineExternalGradientImageLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineGradientMapLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineSubImageLoader(LoadProcess* p, const TagInfo& tagInfo);
#ifdef GFX_ENABLE_SOUND
void    SF_STDCALL GFx_DefineExternalSoundLoader(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineExternalStreamSoundLoader(LoadProcess* p, const TagInfo& tagInfo);
#endif
// SoundStreamLoader(); // head, head2, block
void    SF_STDCALL GFx_DefineSceneAndFrameLabelData(LoadProcess* p, const TagInfo& tagInfo);
void    SF_STDCALL GFx_DefineBinaryData(LoadProcess* p, const TagInfo& tagInfo);

// ***** Tag Lookup Tables

typedef void (SF_STDCALL *TagLoaderFunctionType)(LoadProcess*, const TagInfo&);

extern TagLoaderFunctionType SWF_TagLoaderTable[Tag_SWF_TagTableEnd];
extern TagLoaderFunctionType GFx_GFX_TagLoaderTable[Tag_GFX_TagTableEnd - Tag_GFX_TagTableBegin];

// to register optional tag loader function
extern bool RegisterTagLoader(unsigned tagType, TagLoaderFunctionType tagLoaderFunc);

}} // Scaleform::GFx

#endif
