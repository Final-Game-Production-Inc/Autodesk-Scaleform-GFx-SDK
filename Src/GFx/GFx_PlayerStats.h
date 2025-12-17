/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_PlayerStats.h
Content     :   Definitions of GFxPlayer Stat constants.
Created     :   June 21, 2008
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_PLAYERSTATS_H
#define INC_SF_GFX_PLAYERSTATS_H

#include "GFxConfig.h"
#include "GFx/GFx_Stats.h"

namespace Scaleform { namespace GFx {

// ***** Movie Statistics Identifiers

// Specific statistics.
enum StatMovieData
{
    StatMD_Default = StatGroup_GFxMovieData,

    // Memory for a movie.
    StatMD_Mem,
      StatMD_CharDefs_Mem,
      StatMD_ShapeData_Mem,
      StatMD_Tags_Mem,
      StatMD_Fonts_Mem,
      StatMD_Images_Mem,
#ifdef GFX_ENABLE_SOUND
      StatMD_Sounds_Mem,
#endif
      StatMD_ASBinaryData_Mem,
      StatMD_Other_Mem,

    // Different
    StatMD_Time,
      StatMD_Load_Tks,
      StatMD_Bind_Tks
};

enum StatMovieView
{
    StatMV_Default = StatGroup_GFxMovieView,

    // MovieView memory.
    StatMV_Mem,
      StatMV_MovieClip_Mem,
      StatMV_ActionScript_Mem,
          StatMV_ASString_Mem,
      StatMV_Text_Mem,
      StatMV_XML_Mem,
      StatMV_Other_Mem,
      StatMV_VM,
        StatMV_VM_VM_Mem,
        StatMV_VM_CallFrame_Mem,
        StatMV_VM_VTable_Mem,
        StatMV_VM_SlotInfo_Mem,
            StatMV_VM_SlotInfoHash_Mem,
        StatMV_VM_CTraits_Mem,
        StatMV_VM_Class_Mem,
        StatMV_VM_ITraits_Mem,
        StatMV_VM_Instance_Mem,
        StatMV_VM_AbcFile_Mem,
        StatMV_VM_AbcConstPool_Mem,
        StatMV_VM_VMAbcFile_Mem,
        StatMV_VM_Tracer_Mem,

    // MovieView Timings.
    StatMV_Tks,
      StatMV_Advance_Tks,
        StatMV_Action_Tks,
          StatMV_Seek_Tks,
        StatMV_Timeline_Tks,
        StatMV_Input_Tks,
            StatMV_Mouse_Tks,
      StatMV_ScriptCommunication_Tks,
        StatMV_GetVariable_Tks,
        StatMV_SetVariable_Tks,
        StatMV_Invoke_Tks,
          StatMV_InvokeAction_Tks,
      StatMV_Display_Tks,
        StatMV_FontThrash_Tks,
        StatMV_GradientGen_Tks,
        StatMV_FontMiss_Tks,

    // Counters.
    StatMV_Counters,
      StatMV_Invoke_Cnt,
      StatMV_MCAdvance_Cnt,
      StatMV_Tessellate_Cnt
};


enum StatIME
{
    StatIME_Default = StatGroup_GFxIME,

    // IME memory.
    StatIME_Mem
};


enum StatFontCache
{
    StatFC_Default = StatGroup_GFxFontCache,

    // Font Cache memory.
    StatFC_Mem,
        StatFC_Batch_Mem,
        StatFC_GlyphCache_Mem,
        StatFC_Other_Mem
};

enum StatAmp
{
    StatAmp_Default = StatGroup_GFxAmp,

    StatAmp_Mem,
        StatAmp_ProfileFrame,
        StatAmp_Server,
        StatAmp_Message,
        StatAmp_Callstack,
        StatAmp_InstrBuffer,
};

}} // Scaleform::GFx

#endif // ! INC_SF_GFX_PLAYERSTATS_H
