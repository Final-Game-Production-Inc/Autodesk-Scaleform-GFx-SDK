/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_Config.h
Content     :   
Created     :   2012/11/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS4_Config_H
#define INC_SF_PS4_Config_H

#include "Kernel/SF_Types.h"
#include <sdk_version.h>

#include <gnm.h>
#include "../../../../redgnmx/redgnmx.h"
#include <sdk_version.h>

// Uncomment this to get a version of the HAL that uses LCUE. This will
// be removed in favor of a project setting in the future.
//#define SF_PS4_USE_LCUE

#ifdef SF_PS4_USE_LCUE
    // In SDK 1.600, lcue is built-in, and doesn't need a separate include
    #if SCE_ORBIS_SDK_VERSION < 0x01600051u
        #include <gnmx/lcue.h>
        typedef sce::Gnmx::LCUE::GraphicsContext        sceGnmxContextType;
        typedef sce::Gnmx::LCUE::ShaderResourceOffsets  sceGnmxSROType;
        #define sceGnmxGenerateSROTable                 sce::Gnmx::LCUE::generateSROTable
    #else
        #include "../../../../redgnmx/redgnmx/lwcue_base.h"
        typedef sce::Gnmx::GfxContext					sceGnmxContextType;
        typedef sce::Gnmx::InputResourceOffsets			sceGnmxSROType;
        #define sceGnmxGenerateSROTable                                                  sce::Gnmx::generateInputResourceOffsetTable
    #endif
#else
typedef sce::Gnmx::GfxContext sceGnmxContextType;
#endif

namespace Scaleform { namespace Render { namespace PS4 {

// Orbis SDK 0.820 renamed dataformats. Use the new ones in source, but make defines for the old one, so 0.810 SDK will still work.
#if SCE_ORBIS_SDK_VERSION <= 0x00810030u
    #define kDataFormatB8G8R8A8Unorm     DATA_FORMAT_B8_G8_R8_A8_UNORM
    #define kDataFormatR8G8B8A8Unorm     DATA_FORMAT_R8_G8_B8_A8_UNORM
    #define kDataFormatA8Unorm           DATA_FORMAT_A8_UNORM
    #define kDataFormatBc1Unorm          DATA_FORMAT_BC1_UNORM
    #define kDataFormatBc2Unorm          DATA_FORMAT_BC2_UNORM
    #define kDataFormatBc3Unorm          DATA_FORMAT_BC3_UNORM
    #define kDataFormatInvalid           DATA_FORMAT_INVALID
    #define kDataFormatR32G32Float       DATA_FORMAT_R32_G32_FLOAT
    #define kDataFormatR8Unorm           DATA_FORMAT_R8_UNORM
    #define kDataFormatR8G8Unorm         DATA_FORMAT_R8_G8_UNORM
    #define kDataFormatR8Sint            DATA_FORMAT_R8_SINT
    #define kDataFormatR8G8Sint          DATA_FORMAT_R8_G8_SINT
    #define kDataFormatR8G8B8A8Sint      DATA_FORMAT_R8_G8_B8_A8_SINT
    #define kDataFormatR16Sint           DATA_FORMAT_R16_SINT
    #define kDataFormatR16G16Sint        DATA_FORMAT_R16_G16_SINT
    #define kDataFormatR16G16B16A16Sint  DATA_FORMAT_R16_G16_B16_A16_SINT
    #define kDataFormatR16Uint           DATA_FORMAT_R16_UINT
    #define kDataFormatR16G16Uint        DATA_FORMAT_R16_G16_UINT                
    #define kDataFormatR16G16B16A16Uint  DATA_FORMAT_R16_G16_B16_A16_UINT
    #define kDataFormatR32Uint           DATA_FORMAT_R32_UINT
    #define kDataFormatR32G32Uint        DATA_FORMAT_R32_G32_UINT
    #define kDataFormatR32G32B32Uint     DATA_FORMAT_R32_G32_B32_UINT
    #define kDataFormatR32G32B32A32Uint  DATA_FORMAT_R32_G32_B32_A32_UINT
    #define kDataFormatR32Float          DATA_FORMAT_R32_FLOAT
    #define kDataFormatR32G32Float       DATA_FORMAT_R32_G32_FLOAT
    #define kDataFormatR32G32B32Float    DATA_FORMAT_R32_G32_B32_FLOAT
    #define kDataFormatR32G32B32A32Float DATA_FORMAT_R32_G32_B32_A32_FLOAT
#endif

}}}; // Scaleform::Render::Orbis

#endif // INC_SF_Orbis_Config_H
