/**************************************************************************

PublicHeader:   None
Filename    :   GFx_Tags.h
Content     :   Tags constants used by the SWF/GFX Files
Created     :   June 30, 2005
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX__TAGS_H
#define INC_SF_GFX__TAGS_H

namespace Scaleform { namespace GFx {

// ***** SWF/GFX File Tag Constants

enum TagType
{
    // Add the standard loaders.
    Tag_End                      = 0,
    Tag_EndFrame                 = 1,
    Tag_DefineShape              = 2,
    Tag_PlaceObject              = 4,
    Tag_RemoveObject             = 5,
    Tag_DefineBitsJpeg           = 6,
    Tag_ButtonCharacter          = 7,
    Tag_JpegTables               = 8,
    Tag_SetBackgroundColor       = 9,
    Tag_DefineFont               = 10,
    Tag_DefineText               = 11,
    Tag_DoAction                 = 12,
    Tag_DefineFontInfo           = 13,
    Tag_DefineSound              = 14,
    Tag_StartSound               = 15,
    Tag_ButtonSound              = 17,
    Tag_SoundStreamHead          = 18,
    Tag_SoundStreamBlock         = 19,
    Tag_DefineBitsLossless       = 20,
    Tag_DefineBitsJpeg2          = 21,
    Tag_DefineShape2             = 22,
    Tag_Protect                  = 24,
    Tag_PlaceObject2             = 26,
    Tag_RemoveObject2            = 28,
    Tag_DefineShape3             = 32,
    Tag_DefineText2              = 33,
    Tag_DefineEditText           = 37,
    Tag_ButtonCharacter2         = 34,
    Tag_DefineBitsJpeg3          = 35,
    Tag_DefineBitsLossless2      = 36,
    Tag_Sprite                   = 39,
    Tag_FrameLabel               = 43,
    Tag_SoundStreamHead2         = 45,
    Tag_DefineShapeMorph         = 46,
    Tag_DefineFont2              = 48,
    Tag_Export                   = 56,
    Tag_Import                   = 57,
    Tag_DoInitAction             = 59,
    Tag_DefineVideoStream        = 60,
    Tag_DefineFontInfo2          = 62,
    Tag_DebugID					 = 63,
    Tag_SetTabIndex              = 66,
    // SWF8 starts (may need to add more tags)
    Tag_FileAttributes           = 69,
    Tag_PlaceObject3             = 70,
    Tag_Import2                  = 71,
    Tag_CSMTextSettings          = 74,
    Tag_DefineFont3              = 75,
    Tag_SymbolClass              = 76,
    Tag_Metadata                 = 77,
    Tag_DefineScale9Grid         = 78,
    Tag_DoAbc                    = 82,
    Tag_DefineShape4             = 83,
    Tag_DefineShapeMorph2        = 84,
    Tag_DefineSceneAndFrameLabelData = 86,
    Tag_DefineBinaryData         = 87,
    Tag_DefineFontName           = 88,
    Tag_StartSound2              = 89,
    Tag_DefineBitsJpeg4          = 90,
    Tag_DefineFont4              = 91,

    // Size of table for lookup arrays.
    Tag_SWF_TagTableEnd,

    // GFx Extension tags.
    Tag_ExporterInfo             = 1000,
    Tag_FontTextureInfo          = 1002,
    Tag_DefineExternalGradient   = 1003,
    Tag_DefineGradientMap        = 1004,
    Tag_DefineCompactedFont      = 1005,
    Tag_DefineExternalSound      = 1006,
    Tag_DefineExternalStreamSound = 1007,
    Tag_DefineSubImage           = 1008,
    Tag_DefineExternalImage      = 1009,
    
    // Size of table for lookup arrays.
    Tag_GFX_TagTableEnd,
    Tag_GFX_TagTableBegin        = 1000,
};

}} // Scaleform::GFx

#endif
