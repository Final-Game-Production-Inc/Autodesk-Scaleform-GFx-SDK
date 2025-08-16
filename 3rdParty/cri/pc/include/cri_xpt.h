/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 1998-2010 CRI Middleware Co., Ltd.
 *
 * Library  : CRI Middleware Library
 * Module   : CRI Common Header for Windows
 * File     : cri_xpt.h
 * Date     : 2010-05-19
 * Version  : 1.17
 *
 ****************************************************************************/
#ifndef CRI_INCL_CRI_XPT_H
#define CRI_INCL_CRI_XPT_H

#if defined(__BORLANDC__)
#define XPT_TGT_BCB
#else
#define XPT_TGT_PC
#define XPT_TGT_PC_PRO
#endif

#define XPT_CCS_LEND
#define XPT_SUPPORT_MULTICHANNEL

#if defined(XPT_TGT_PC)
	#if defined(_MSC_VER)
		#if (_MSC_VER >= 1400)	/* Visual Studio 2005 or later */
			#if defined(WIN64) || defined(_WIN64) || defined(_M_X64)
				#define CRI_TARGET_STR		"PCx64"		/* WIN64 */
			#elif defined(WIN32) || defined(_WIN32)
				#define CRI_TARGET_STR		"PCx86"		/* WIN32 */
			#else
				#error cri_xpt.h : Preprosessor definition WIN32/WIN64 should be defined.
			#endif
		#else
			#define CRI_TARGET_STR			"PC"		/* Visual Studio .NET 2003 or Visual C++ 6.0 */
		#endif
	#else
		#error cri_xpt.h : CRI_TARGET_STR is not defined by reason that the compiler is not assumed.
	#endif
#else
	#define CRI_TARGET_STR "PCBCB"
#endif

#include "cri_xpts_win.h"
#include "cri_xpt_post.h"

#endif  /* CRI_INCL_CRI_XPT_H */
/* End Of File */
