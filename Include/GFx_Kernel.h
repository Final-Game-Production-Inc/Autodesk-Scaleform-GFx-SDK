/**************************************************************************

Filename    :   GFx_Kernel.h
Content     :   Convenience header collection for  
Created     :   July 2010 
Authors     :   Automatically generated 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
#ifndef INC_GFx_Kernel_H 
#define INC_GFx_Kernel_H 
 
#include "../Src/Kernel/SF_AllocAddr.h" 		
#include "../Src/Kernel/SF_Allocator.h" 		
#include "../Src/Kernel/SF_AllocInfo.h" 		
#include "../Src/Kernel/SF_AmpInterface.h" 		
#include "../Src/Kernel/SF_ArrayPaged.h" 		
#include "../Src/Kernel/SF_ArrayStaticBuff.h" 		
#include "../Src/Kernel/SF_AutoPtr.h" 		
#include "../Src/Kernel/SF_Debug.h" 		
#include "../Src/Kernel/SF_File.h" 		
#include "../Src/Kernel/SF_HeapNew.h" 		
#include "../Src/Kernel/SF_KeyCodes.h" 
#include "../Src/Kernel/SF_ListAlloc.h" 		
#include "../Src/Kernel/SF_Locale.h" 		
#include "../Src/Kernel/SF_Log.h" 		
#include "../Src/Kernel/SF_Math.h" 		
#include "../Src/Kernel/SF_Memory.h" 		
#include "../Src/Kernel/SF_MemoryHeap.h" 		
#include "../Src/Kernel/SF_RadixTree.h" 		
#include "../Src/Kernel/SF_Random.h" 		
#include "../Src/Kernel/SF_Range.h" 		
#include "../Src/Kernel/SF_RefCount.h" 		
#include "../Src/Kernel/SF_StackMemPool.h" 		
#include "../Src/Kernel/SF_Stats.h" 		
#include "../Src/Kernel/SF_Std.h" 		
#include "../Src/Kernel/SF_String.h" 		
#include "../Src/Kernel/SF_SysAlloc.h" 		
#include "../Src/Kernel/SF_SysFile.h" 		
#include "../Src/Kernel/SF_System.h" 		
#include "../Src/Kernel/SF_Timer.h" 		
#include "../Src/Kernel/SF_Types.h" 		
#include "../Src/Kernel/HeapMH/HeapMH_MemoryHeap.h" 		
#include "../Src/Kernel/HeapMH/HeapMH_SysAllocMalloc.h" 		
#include "../Src/Kernel/HeapPT/HeapPT_MemoryHeap.h" 		
#ifdef SF_OS_3DS
	#include "../Src/Kernel/HeapPT/HeapPT_SysAlloc3DS.h" 		
#endif
#if !defined(SF_OS_3DS) && !defined(SF_OS_WII)
#include "../Src/Kernel/HeapPT/HeapPT_SysAllocMalloc.h"
#endif 		
#ifdef SF_OS_LINUX
	#include "../Src/Kernel/HeapPT/HeapPT_SysAllocMMAP.h" 		
#endif
#ifdef SF_OS_PS3
	#include "../Src/Kernel/HeapPT/HeapPT_SysAllocPS3.h" 		
#endif
#ifdef SF_OS_WII
	#include "../Src/Kernel/HeapPT/HeapPT_SysAllocWii.h" 		
#endif
#if defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
	#include "../Src/Kernel/HeapPT/HeapPT_SysAllocWinAPI.h" 		
#endif		
 
#endif     // INC_GFx_Kernel_H 
