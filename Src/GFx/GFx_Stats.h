/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Stats.h
Content     :   Statistic IDs
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_Stats_H
#define INC_SF_GFX_Stats_H

// ***** Stat Group Identifiers
//
// Stat identifiers are organized into groups corresponding to different
// sub-systems such as the renderer, MovieDef, etc. Every group receives
// its own Id range, allowing identifiers within that system to be added
// without having to modify StatId (those identifiers are normally defined
// as a part of a separate enumeration in their own header file).

// All identifier constants must follow this naming convention:
// - Start with a system group prefix such as StatGroup_ (for group ids),
//   StatHeap_, StatRenderer_, GFxStatMV_, etc.
// - User one of the following endings to identify stat types:
//     - _Mem for MemoryStat.
//     - _Tks for TimerStat.
//     - _Cnt for CounterStat.
//---------------------------------------------------------------------------
enum StatGroup
{
    StatGroup_Renderer             = 1 << 6,
    StatGroup_RenderGen            = 2 << 6,
    StatGroup_GFxFontCache         = 3 << 6,
    StatGroup_GFxMovieData         = 4 << 6,
    StatGroup_GFxMovieView         = 5 << 6,
    StatGroup_GFxRenderCache       = 6 << 6,
    StatGroup_GFxPlayer            = 7 << 6,
    StatGroup_GFxIME               = 8 << 6,
    StatGroup_GFxAmp               = 9 << 6,

    // General Memory
    Stat_Image_Mem = Stat_Default_Mem + 1,
#ifdef GFX_ENABLE_SOUND
    Stat_Sound_Mem,
#endif
    Stat_String_Mem,
#ifdef GFX_ENABLE_VIDEO
    Stat_Video_Mem,
#endif
    // Memory allocated for debugging purposes.
    Stat_Debug_Mem,
        Stat_DebugHUD_Mem,
        Stat_DebugTracker_Mem,
        Stat_StatBag_Mem,
};


// ***** Identifiers for important heaps
//
// This IDs can be associated with additional statically defined 
// data to display. In particular, the heaps can be associated with
// a colors to be displayed in the HUD.
//---------------------------------------------------------------------------
enum HeapId
{
    HeapId_Global      = HeapId_Default,
    HeapId_MovieDef,
    HeapId_MovieView,
    HeapId_MovieData,
    HeapId_Images,
    HeapId_OtherHeaps,
    HeapId_HUDHeaps,
    HeapId_Video,
};

#endif
