/**************************************************************************

Filename    :   GFx_StatsDecl.h
Content     :   Statistic ID declaration.
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   It's guranteed that this file will be included only once 
                from SF/Kernel/Stats.cpp, so, the include guards are not 
                necessary. Its content will also be inside namespace Scaleform

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

SF_DECLARE_MEMORY_STAT(Stat_Image_Mem, "Image", Stat_Mem)
#ifdef GFX_ENABLE_SOUND
SF_DECLARE_MEMORY_STAT(Stat_Sound_Mem, "Sound", Stat_Mem)
#endif
SF_DECLARE_MEMORY_STAT(Stat_String_Mem, "String", Stat_Mem)
#ifdef GFX_ENABLE_VIDEO
SF_DECLARE_MEMORY_STAT(Stat_Video_Mem, "Video", Stat_Mem)
#endif

SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(Stat_Debug_Mem, "Debug Memory", Stat_Mem)
SF_DECLARE_MEMORY_STAT(Stat_DebugHUD_Mem, "Debug HUD", Stat_Debug_Mem)
SF_DECLARE_MEMORY_STAT(Stat_DebugTracker_Mem, "Debug Tracker", Stat_Debug_Mem)
SF_DECLARE_MEMORY_STAT(Stat_StatBag_Mem, "StatBag", Stat_Debug_Mem)

