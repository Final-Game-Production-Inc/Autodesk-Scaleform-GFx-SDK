/**********************************************************************

Filename    :   GFxIMEIdMap.h
Content     :   Contains definition for classes needed for Registry Query. 
Third party IME's register themselves with different locale Id's on
different machines. Therefore we can't identify an IME from it's locale
Id consistently. To solve this problem, we use the layout text and 
IME name registry key to identify supported IME's and store a map between 
IME tags and locale id's. This allows us to use the tags for identifying 
IME's which is invariant to the system being used. This also enables us to 
let the messages for unsupported IME's pass through unhindered and hence the UI
windows corresponding to those messages are no longer suppressed. 

Created     :   Jun 17, 2008
Authors     :   Ankur Mohan

Notes       :   
History     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
#include "GFxConfig.h"
#ifndef SF_NO_IME_SUPPORT

#include "GFx_IMEIdMap.h"
#include "GFx_IMEManagerWin32.h"
#include "GFx/GFx_PlayerImpl.h"


namespace Scaleform {
	namespace GFx {
		namespace IME{
// List of supported Input Languages
/*
InputLangProps SupportedInputLanguages[] = 
{
{"US"                                   , "English"         		,GFxIME_English             ,0},
{"English (United States)"              , "English"         		,GFxIME_English             ,0},
{"Chinese (Taiwan)"						, "Chinese (Traditional)"   ,GFxIME_Ch_Trad				,0},
{"Chinese (Traditional, Taiwan)"		, "Chinese (Traditional)"   ,GFxIME_Ch_Trad				,0},
{"\xE4\xB8\xAD\xE6\x96\x87\x28\xE5\x8F\xB0\xE6\xB9\xBE\x29"			
, "Chinese (Traditional)"	,GFxIME_Ch_Trad				,0}, //		"中文(台湾)"	
{"\xE4\xB8\xAD\xE6\x96\x87\x28\xE4\xB8\xAD\xE5\x9B\xBD\x29"			
, "Chinese (Simplified)"	,GFxIME_Ch_Simp				,0}, //		"中文(中国)"
{"Japanese (Japan)"						, "Japanese"        		,GFxIME_Jp					,0},
{"Japanese"								, "Japanese"        		,GFxIME_Jp					,0},
{"Korean (Korea)"						, "Korean 2000"     		,GFxIME_Kr		            ,0},
{"Korean"								, "Korean 2000"     		,GFxIME_Kr		            ,0},
{"Chinese (PRC)"						, "Chinese (Simplified)"    ,GFxIME_Ch_Simp				,0},
{"Chinese (Simplified, PRC)"			, "Chinese (Simplified)"    ,GFxIME_Ch_Simp				,0}
};
*/
/*
InputLangProps SupportedInputLanguages[] = 
{
{0										, "English"         		,GFxIME_English             ,0x00000409},
{0										, "Chinese (Traditional)"   ,GFxIME_Ch_Trad				,0x00000404},
{0										, "Japanese"        		,GFxIME_Jp					,0x00000411},
{0										, "Korean 2000"     		,GFxIME_Kr		            ,0x00000412},
{0										, "Chinese (Simplified)"    ,GFxIME_Ch_Simp				,0x00000804},
};
*/

// List of supported IME's
InputLangProps ReserveSupportedIMEs[]=
{
	{"Chinese (Traditional) - Phonetic"     , "Phonetic"        			,GFxIME_Ch_Trad_Phonetic    ,0},
	{"phon.ime"								, "Phonetic"        			,GFxIME_Ch_Trad_Phonetic	,0},
	{"TINTLGNT.IME"							, "New Phonetic"    			,GFxIME_Ch_Trad_NewPhonetic ,0},
	// When office IME 2003 IMe's are installed (TINTSETP.msi), the registry entry for New Phonetic is replaced and
	// the new ime file name is the entry below
	{"MSTCIPHA.IME"							, "New Phonetic"    			,GFxIME_Ch_Trad_NewPhonetic ,0},
	{"Chinese (Traditional) - New Phonetic" , "New Phonetic"    			,GFxIME_Ch_Trad_NewPhonetic ,0},
	{"Chinese (Traditional) - New Phonetic 2010" , "New Phonetic 2010"    	,GFxIME_Ch_Trad_NewPhonetic ,0},
	{"Chinese (Traditional) - ChangJie"     , "ChangJie"        			,GFxIME_Ch_Trad_ChangJie    ,0},
	{"chajei.ime"							, "ChangJie"        			,GFxIME_Ch_Trad_ChangJie    ,0},
	{"Chinese (Traditional) - New ChangJie" , "New ChangJie"    			,GFxIME_Ch_Trad_NewChangJie ,0},
	{"Chinese (Traditional) - New ChangJie 2010" , "New ChangJie 2010"    	,GFxIME_Ch_Trad_NewChangJie ,0},

	{"CINTLGNT.IME"							, "New ChangJie"    			,GFxIME_Ch_Trad_NewChangJie ,0},
	{"IMTCC14.IME"							, "New ChangJie 2010"    		,GFxIME_Ch_Trad_NewChangJie2010 ,0},
	{"IMTCP14.IME"							, "New Phonetic 2010"    		,GFxIME_Ch_Trad_NewPhonetic2010 ,0},
	{"IMTCQ14.IME"							, "New Quick 2010"    			,GFxIME_Ch_Trad_NewQuick2010 ,0},
	// When office IME 2003 IMe's are installed (CINTSETP.msi), the registry entry for New changjie is replaced and
	// the new ime file name is the entry below
	{"MSTCICJA.IME"							, "New ChangJie"    			,GFxIME_Ch_Trad_NewChangJie ,0},
	{"Chinese (Traditional) - Big5 Code"    , "Big5"            			,GFxIME_NotSupported        ,0},
	{"Chinese (Traditional) - Array"        , "Array"           			,GFxIME_Ch_Trad_Array       ,0},
	{"Chinese Traditional Array (version 6.0)", "Array"           			,GFxIME_Ch_Trad_Array       ,0},
	{"winar30.ime"							, "Array"           			,GFxIME_Ch_Trad_Array       ,0},
	{"Chinese (Traditional) - Quick"        , "Quick"           			,GFxIME_Ch_Trad_Quick       ,0},
	{"quick.ime"							, "Quick"           			,GFxIME_Ch_Trad_Quick       ,0},
	{"Chinese (Traditional) - DaYi"         , "DaYi"            			,GFxIME_Ch_Trad_DaYi        ,0},
	{"Chinese Traditional DaYi (version 6.0)","DaYi"            			,GFxIME_Ch_Trad_DaYi        ,0},
	{"dayi.ime"								, "DaYi"            			,GFxIME_Ch_Trad_DaYi        ,0},
	{"Chinese (Traditional) - Unicode"      , "Unicode"         			,GFxIME_NotSupported        ,0},
	{"Chinese (Traditional) - US Keyboard"  , "Unknown"         			,GFxIME_Ch_Trad				,0},
	{"Chinese (Traditional) - Alphanumeric" , "Alphanumeric"    			,GFxIME_NotSupported        ,0},

	{"Microsoft IME"						, "Japanese Microsoft IME"		,GFxIME_Jp					,0},
	{"imjp81.ime"							, "Japanese Microsoft IME"		,GFxIME_Jp					,0},
	{"Japanese Input System (MS-IME2002)"   , "MS-IME 2002"     			,GFxIME_Jp_2002         	,0},
	{"imjp9.ime"							, "Microsoft IME Standard 2003" ,GFxIME_Jp_2003         	,0},
	{"imjp8.ime"							, "Microsoft IME Standard 2002" ,GFxIME_Jp_2002         	,0},
//	{"Microsoft Office IME 2007"            , "MS-IME 2007"     			,GFxIME_Jp_2007         	,0},	// use imename for jap ime 2007 as opposed to "Microsoft Office IME 2007"
	{"imjp12.ime"							, "MS-IME 2007"     			,GFxIME_Jp_2007         	,0},// since the same string could also be used for Korean Office IME 2007.
	{"imjp14.ime"							, "Japanese IME 2010"     		,GFxIME_Jp_2010         	,0},

	{"ATOK 2008"							, "ATOK 2008"					,GFxIME_Jp_ATOK2008			,0},
	{"ATOK 2009"							, "ATOK 2009"					,GFxIME_Jp_ATOK2009			,0},
	{"ATOK22W.IME"							, "ATOK 2009"					,GFxIME_Jp_ATOK2009			,0},
	{"Google Japanese Input"				, "Google Japanese Input"		,GFxIME_Jp_GOOG2010			,0},
			
	{"Korean Input System (IME 2000)"       , "Korean 2000"     			,GFxIME_Kr		            ,0},
	{"Korean"								, "Korean"     					,GFxIME_Kr		            ,0},
	{"imekr61.ime"							, "Korean IME 2000"     		,GFxIME_Kr_2000		        ,0},
	{"imekr70.ime"							, "Korean IME 2003"     		,GFxIME_Kr_2003		        ,0},
	{"imkr12.ime"							, "Korean IME 2007"     		,GFxIME_Kr_2007		        ,0},
	{"imkr14.ime"							, "Korean IME 2010"     		,GFxIME_Kr_2010		        ,0},
	{"imekr.ime"							, "Korean IME 2002"     		,GFxIME_Kr_2002		        ,0},
	{"Microsoft IME"						, "Korean Microsoft IME"		,GFxIME_Kr					,0},
	// Microsoft Korean IME 8.0.6001 on Korean Vista.
	{"Microsoft \xed\x95\x9c\xea\xb8\x80 \xec\x9e\x85\xeb\xa0\xa5\xea\xb8\xb0", "Korean Microsoft IME", GFxIME_Kr_8_6001, 0},

	{"Chinese (Simplified) - US Keyboard"	, "Chinese (Simplified)"    	,GFxIME_Ch_Simp				,0},
	{"Chinese (Simplified) - QuanPin"       , "Quan Pin"        			,GFxIME_Ch_Simp_QuanPin     ,0},
	{"Chinese Simplified QuanPin (version 6.0)", "Quan Pin"        			,GFxIME_Ch_Simp_QuanPin     ,0},
	{"winpy.ime"							, "Quan Pin"        			,GFxIME_Ch_Simp_QuanPin     ,0},
	{"Chinese (Simplified) - ShuangPin"     , "Shuang Pin"      			,GFxIME_Ch_Simp_ShuangPin   ,0},
	{"Chinese Simplified ShuangPin (version 6.0)", "Shuang Pin"      		,GFxIME_Ch_Simp_ShuangPin   ,0},
	{"winsp.ime"							, "Shuang Pin"      			,GFxIME_Ch_Simp_ShuangPin   ,0},
	{"Chinese (Simplified) - ZhengMa"   , "ZhengMa"         			    ,GFxIME_Ch_Simp_ZhengMa     ,0},
	{"Chinese Simplified ZhengMa (version 6.0)", "Zheng Ma"        			,GFxIME_Ch_Simp_ZhengMa     ,0},
	{"winzm.ime"							, "ZhengMa"         			,GFxIME_Ch_Simp_ZhengMa     ,0},
	{"Chinese (Simplified) - NeiMa"         , "NeiMa"           			,GFxIME_NotSupported        ,0},
	{"Chinese (Simplified) - Microsoft Pinyin IME 3.0", "MSPinyin IME 3.0"	,GFxIME_Ch_Simp_MSPinyin_3_0 ,0},
	{"Chinese (Simplified) - Microsoft Pinyin IME", "MSPinyin IME"			,GFxIME_Ch_Simp_MSPinyin	,0},
	{"Chinese (Simplified) - Microsoft Pinyin New Experience Input Style", "MSPinyin IME",GFxIME_Ch_Simp_MSPinyin	,0},
	{"pintlgnt.ime"                         , "Microsoft Pinyin IME 3.0"	,GFxIME_Ch_Simp_MSPinyin_3_0            ,0},
	{"IMSC40A.IME"							, "Microsoft Pinyin IME 3.0"	,GFxIME_Ch_Simp_MSPinyin_3_0            ,0},
    {"IMSC12li.IME"							, "Microsoft Pinyin IME 2007"	,GFxIME_Ch_Simp_MSPinyin_2007           ,0},
	{"IMSC14.IME"							, "Microsoft Pinyin IME 2010"	,GFxIME_Ch_Simp_MSPinyin1_2010           ,0},
// At the time of the last testing (Aug 24, 2010), this IME doesn't provide reading string information, though candidate list works 
// fine.
//	{"IMSCE14.IME"							, "Microsoft Pinyin IME Simple Fast2010"	,GFxIME_Ch_Simp_MSPinyin2_2010  ,0},

	// "中文(简体) - 微软拼音输入法"
	{"\xe4\xb8\xad\xe6\x96\x87(\xe7\xae\x80\xe4\xbd\x93) - \xe5\xbe\xae\xe8\xbd\xaf\xe6\x8b\xbc\xe9\x9f\xb3\xe8\xbe\x93\xe5\x85\xa5\xe6\xb3\x95"
	, "Microsoft Pinyin IME 3.0"	,GFxIME_Ch_Simp_MSPinyin_3_0            ,0},

	{"winabc.ime"                           , "ABC IME"         			,GFxIME_Ch_Simp_ABC         ,0},
	{"winabcx.ime"                          , "itABC IME v5.30"         	,GFxIME_Ch_Simp_ABC         ,0},
	{"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE7\xBE\x8E\xE5\xBC\x8F\xE9\x94\xAE\xE7\x9B\x98"    
	, "MSPinyin IME 3.0",GFxIME_Ch_Simp_MSPinyin_3_0,0}, //  中文 (简体) - 美式键盘"

	{"Chinese (Simplified) - Wubi86"        , "Wubi86"          			,GFxIME_NotSupported        ,0},
	{"Chinese (Simplified) - Wubi98"        , "Wubi98"          			,GFxIME_NotSupported        ,0},

	{"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE8\xB0\xB7\xE6\xAD\x8C\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95"    
	, "Google Pinyin"   ,GFxIME_GooglePinyin        ,0}, // 中文 (简体) - 谷歌拼音输入法"

	{"\xE8\xB0\xB7\xE6\xAD\x8C\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95"    
	, "Google Pinyin"   ,GFxIME_GooglePinyin        ,0}, // 谷歌拼音输入法"
	
	{"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE6\x90\x9C\xE7\x8B\x97\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95"     
	, "Sogou Pinyin"    ,GFxIME_SogouPinyin         ,0},   // 中文 (简体) - 搜狗拼音输入法

	{"\xE8\xB0\xB7\xE6\xAD\x8C\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95 2.0"    
	, "Google Pinyin"   ,GFxIME_GooglePinyin        ,0}, // 谷歌拼音输入法 2.0"

	{"GOOGLEPINYIN2.IME", "Google Pinyin"   ,GFxIME_GooglePinyin        ,0}, // 谷歌拼音输入法 2

    {"\xE8\xB0\xB7\xE6\xAD\x8C\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95 2", "Google Pinyin"   ,GFxIME_GooglePinyin        ,0}, // 谷歌拼音输入法 2

	{"\xE6\x90\x9C\xE7\x8B\x97\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95"     
	, "Sogou Pinyin"    ,GFxIME_SogouPinyin         ,0},   // 搜狗拼音输入法

    {"sogoupy.ime"                           , "Sogou Pinyin IME"         			,GFxIME_SogouPinyin         ,0},

	{"\xE4\xB8\xAD\xE6\x96\x87(\xE7\xAE\x80\xE4\xBD\x93) - \xE6\x90\x9C\xE7\x8B\x97\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95"                           
	, "Sogou Pinyin IME"         			,GFxIME_SogouPinyin         ,0},  //中文(简体) - 搜狗拼音输入法

	{"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE5\x8F\x8C\xE6\x8B\xBC"
	, "Shuang Pin"      ,GFxIME_Ch_Simp_ShuangPin   ,0}, // 中文 (简体) - 双拼 ShuangPin on Chinese XP machine


	{"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE9\x83\x91\xE7\xA0\x81"
	, "Zheng Ma"        ,GFxIME_Ch_Simp_ZhengMa     ,0},	 // 中文 (简体) - 郑码	ZhengMa on Chinese XP Machine 

	{"\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95"
	, "Unknown"         ,GFxIME_Ch_Simp_Pinyin03    ,0},	 // 拼音输入法	Pinyin 03 

	{"\xE4\xB8\xAD\xE6\x96\x87 - QQ\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95"
	, "QQPinyin"        ,GFxIME_Ch_Simp_QQPinyin    ,0},	 // 中文 - QQ拼音输入法	QQ Pinyin 

	{"\xE4\xB8\xAD\xE6\x96\x87 - QQ\xE4\xBA\x94\xE7\xAC\x94\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95"
	, "QQPinyin"        ,GFxIME_Ch_Simp_QQPinyin    ,0},	 // 中文 - QQ五笔输入法	QQ Pinyin 
	

	{"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE5\xBF\xB5\xE9\x9D\x92\xE7\xB9\x81\xE9\xAB\x94\xE4\xBA\x94\xE7\xAD\x86 2.03"
	, "NianQing"        ,GFxIME_Ch_Simp_NianQing    ,0}	, // 中文 (简体) - 念青繁體五筆 2.03

	// Note that on English XP machine, the WuBi86 and WuBi98 IME's install themselves with corrupted
	// characters in the layout text field, so they may not match the string below. On Chinese XP though
	// this works fine. 
	{"\xE5\xBE\xAE\xE8\xBD\xAF\xE7\x8E\x8B\xE7\xA0\x81\xE4\xBA\x94\xE7\xAC\x94\x38\x36\xE7\x89\x88"
	, "WuBi86"			, GFxIME_Ch_Simp_WuBi86, 0},	// WuBi86

	{"WINWB86.IME",								"WuBi86"			, GFxIME_Ch_Simp_WuBi86, 0},     // WuBi98

	{"\xE5\xBE\xAE\xE8\xBD\xAF\xE7\x8E\x8B\xE7\xA0\x81\xE4\xBA\x94\xE7\xAC\x94\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95\x20\x39\x38\xE7\x89\x88"
	, "WuBi98"			, GFxIME_Ch_Simp_WuBi98, 0},     // WuBi98


	{"WINWB98.IME",								"WuBi98"			, GFxIME_Ch_Simp_WuBi98, 0} ,    // WuBi98

	{"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE5\x8A\xA0\xE5\x8A\xA0\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95\x35\x2E\x30"     
	, "JJ Pinyin"    ,GFxIME_Ch_Simp_JJ         ,0},   // 中文 (简体) - 搜狗拼音输入法

	{"JJ.IME",								"JJ Pinyin"			    , GFxIME_Ch_Simp_JJ, 0},     // JJ
    // Boshiamy Input Method
    {"LIUNT.IME",							"Wu Xia"			    , GFxIME_Ch_Trad_WuXia, 0},     // JJ

	// "中文(繁体) - 仓颉"
	{"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe4\xbb\x93\xe9\xa2\x89", "ChangJie", GFxIME_Ch_Trad_ChangJie, 0},

	// 中文(繁体) - 注音
	{"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe6\xb3\xa8\xe9\x9f\xb3", "Phonetic",	GFxIME_Ch_Trad_Phonetic, 0},


	// "中文(繁体) - 新注音"
	{"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe6\x96\xb0\xe6\xb3\xa8\xe9\x9f\xb3", "New Phonetic", GFxIME_Ch_Trad_NewPhonetic, 0},

	// "中文(繁体) - 速成"
	{"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe9\x80\x9f\xe6\x88\x90",	"Quick",	GFxIME_Ch_Trad_Quick, 0},

	// "中文(繁体) - 新速成"
	{"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe6\x96\xb0\xe9\x80\x9f\xe6\x88\x90", "New Quick",	GFxIME_Ch_Trad_NewQuick, 0},

	// "简体中文郑码(版本 6.0"
	{"\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe9\x83\x91\xe7\xa0\x81(\xe7\x89\x88\xe6\x9c\xac 6.0", "Zheng Ma 6.0", GFxIME_Ch_Simp_ZhengMa, 0},

	// 简体中文全拼(版本 6.0)
	{"\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe5\x85\xa8\xe6\x8b\xbc(\xe7\x89\x88\xe6\x9c\xac 6.0)", "Shuang Pin 6.0", GFxIME_Ch_Simp_ShuangPin, 0},

	// "简体中文双拼(版本 6.0)"
	{"\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe5\x8f\x8c\xe6\x8b\xbc(\xe7\x89\x88\xe6\x9c\xac 6.0)", "Quan Pin 6.0", GFxIME_Ch_Simp_QuanPin, 0},

	// 简体中文郑码(版本 6.0)
	{"\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe9\x83\x91\xe7\xa0\x81(\xe7\x89\x88\xe6\x9c\xac 6.0)", "Zheng Ma 6.0", GFxIME_Ch_Simp_ZhengMa, 0},

	// "微软拼音输入法 2007"
	{"\xe5\xbe\xae\xe8\xbd\xaf\xe6\x8b\xbc\xe9\x9f\xb3\xe8\xbe\x93\xe5\x85\xa5\xe6\xb3\x95 2007", "Microsoft Pinyin IME 2007" ,GFxIME_Ch_Simp_MSPinyin_2007	,0},

	// "中文(繁体) - 新仓颉"
	{"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe6\x96\xb0\xe4\xbb\x93\xe9\xa2\x89", "New ChangJie", GFxIME_Ch_Trad_NewChangJie, 0},

	// "繁体中文大易(版本 6.0)"
	{"\xe7\xb9\x81\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe5\xa4\xa7\xe6\x98\x93(\xe7\x89\x88\xe6\x9c\xac 6.0)", "DaYi 6.0", GFxIME_Ch_Trad_DaYi, 0},

	// "繁体中文行列(版本 6.0)"
	{"\xe7\xb9\x81\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe8\xa1\x8c\xe5\x88\x97(\xe7\x89\x88\xe6\x9c\xac 6.0)", "Array 6.0", GFxIME_Ch_Trad_Array, 0},

	// On Korean Vista
	{"\xec\xa4\x91\xea\xb5\xad\xec\x96\xb4 (\xeb\xb2\x88\xec\xb2\xb4) - ChangJie",	"ChangJie",	GFxIME_Ch_Trad_ChangJie, 0},

	// "Microsoft 입력기"
	{"\x4D\x69\x63\x72\x6f\x73\x6f\x66\x74 \xEC\x9e\x85\xeb\xa0\xa5\xea\xb8\xb0", "Korean IME 2007", GFxIME_Kr_2007		        ,0},

	{"\xec\xa4\x91\xea\xb5\xad\xec\x96\xb4 (\xeb\xb2\x88\xec\xb2\xb4) - Phonetic",	"Phonetic",	GFxIME_Ch_Trad_Phonetic, 0},

	//
	{"\xec\xa4\x91\xea\xb5\xad\xec\x96\xb4 (\xeb\xb2\x88\xec\xb2\xb4) - New Phonetic",	"New Phonetic",	GFxIME_Ch_Trad_NewPhonetic, 0},

	// "중국어 (번체) - New ChangJie"
	{"\xec\xa4\x91\xea\xb5\xad\xec\x96\xb4 (\xeb\xb2\x88\xec\xb2\xb4) - New ChangJie",	"New ChangJie",	GFxIME_Ch_Trad_NewChangJie, 0},

	// On Chinese Traditional Vista
	// "中文 (繁體) - 倉頡"
	{"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe5\x80\x89\xe9\xa0\xa1",	"ChangJie",	GFxIME_Ch_Trad_ChangJie, 0},
	
	//  "中文 (繁體) - 新倉頡"
	{"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe6\x96\xb0\xe5\x80\x89\xe9\xa0\xa1",	"New ChangJie",	GFxIME_Ch_Trad_NewChangJie, 0},

	//	"中文 (繁體) - 注音"
	{"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe6\xb3\xa8\xe9\x9f\xb3",	"Phonetic",	GFxIME_Ch_Trad_Phonetic, 0},

	//	"中文 (繁體) - 新注音"
	{"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe6\x96\xb0\xe6\xb3\xa8\xe9\x9f\xb3",	"New Phonetic",	GFxIME_Ch_Trad_NewPhonetic, 0},

	//	"中文 (繁體) - 速成"	
	{"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe9\x80\x9f\xe6\x88\x90",	"Quick",	GFxIME_Ch_Trad_Quick, 0},

	//	"中文 (繁體) - 新速成"	
	{"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe6\x96\xb0\xe9\x80\x9f\xe6\x88\x90",	"New Quick",	GFxIME_Ch_Trad_NewQuick, 0},

	//	"中文繁體大易 (version 6.0)"	
	{"\xe4\xb8\xad\xe6\x96\x87\xe7\xb9\x81\xe9\xab\x94\xe5\xa4\xa7\xe6\x98\x93 (version 6.0)",	"DaYi",	GFxIME_Ch_Trad_DaYi, 0},

	//	"中文繁體行列 (version 6.0)"		
	{"\xe4\xb8\xad\xe6\x96\x87\xe7\xb9\x81\xe9\xab\x94\xe8\xa1\x8c\xe5\x88\x97 (version 6.0)",	"Array",	GFxIME_Ch_Trad_Array, 0},

	//	 "中文 (簡體) - 微軟拼音輸入法"		
	{ "\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb0\xa1\xe9\xab\x94) - \xe5\xbe\xae\xe8\xbb\x9f\xe6\x8b\xbc\xe9\x9f\xb3\xe8\xbc\xb8\xe5\x85\xa5\xe6\xb3\x95",
		"Microsoft Pinyin IME 3.0",	GFxIME_Ch_Simp_MSPinyin_3_0, 0},

	//	 "中文簡體全拼 (version 6.0)"		
	{ "\xe4\xb8\xad\xe6\x96\x87\xe7\xb0\xa1\xe9\xab\x94\xe5\x85\xa8\xe6\x8b\xbc (version 6.0)", "Shuang Pin",	GFxIME_Ch_Simp_ShuangPin, 0},

	//	"中文簡體鄭碼 (version 6.0)"		
	{ "\xe4\xb8\xad\xe6\x96\x87\xe7\xb0\xa1\xe9\xab\x94\xe9\x84\xad\xe7\xa2\xbc (version 6.0)", "Zheng Ma",	GFxIME_Ch_Simp_ZhengMa, 0},

	//	"中文簡體雙拼 (version 6.0)"		
	{ "\xe4\xb8\xad\xe6\x96\x87\xe7\xb0\xa1\xe9\xab\x94\xe9\x9b\x99\xe6\x8b\xbc (version 6.0)", "Quan Pin",	GFxIME_Ch_Simp_QuanPin, 0},

	// "Microsoft Pinyin IME 2007"
	{ "Microsoft Pinyin IME 2007", "Microsoft Pinyin IME 2007",	GFxIME_Ch_Simp_MSPinyin_2007, 0},



	// "New Chewing IME"
	{ "CHEWING.IME", "New Chewing IME",	GFxIME_Ch_Trad_NewChewing, 0}

};

StringToTagDef StringToTag[] = 
{
    {"GFxIME_English"               	, GFxIME_English} ,
    {"GFxIME_Ch_Trad_Phonetic"      	, GFxIME_Ch_Trad_Phonetic},
    {"GFxIME_Ch_Trad_NewPhonetic"   	, GFxIME_Ch_Trad_NewPhonetic},
    {"GFxIME_Ch_Trad_ChangJie"      	, GFxIME_Ch_Trad_ChangJie},
    {"GFxIME_Ch_Trad_NewChangJie"   	, GFxIME_Ch_Trad_NewChangJie},
	{"GFxIME_Ch_Trad_NewChangJie2010"   , GFxIME_Ch_Trad_NewChangJie2010},
	{"GFxIME_Ch_Trad_NewPhonetic2010"   , GFxIME_Ch_Trad_NewPhonetic2010},
	{"GFxIME_Ch_Trad_NewQuick2010"   	, GFxIME_Ch_Trad_NewQuick2010},
    {"GFxIME_Ch_Trad_DaYi"          	, GFxIME_Ch_Trad_DaYi},
    {"GFxIME_Ch_Trad_Array"         	, GFxIME_Ch_Trad_Array},
    {"GFxIME_Ch_Trad_Quick"         	, GFxIME_Ch_Trad_Quick},
    {"GFxIME_Ch_Trad_NewQuick"      	, GFxIME_Ch_Trad_NewQuick},

    {"GFxIME_Jp_2002"               	, GFxIME_Jp_2002},             
    {"GFxIME_Jp_2007"               	, GFxIME_Jp_2007}, 
	{"GFxIME_Jp_2010"               	, GFxIME_Jp_2010}, 
    {"GFxIME_Jp_2003"               	, GFxIME_Jp_2003},             
    {"GFxIME_Jp"                    	, GFxIME_Jp},                  
    {"GFxIME_Jp_ATOK2008"           	, GFxIME_Jp_ATOK2008},          
    {"GFxIME_Jp_GOOG2010"           	, GFxIME_Jp_GOOG2010},
		

    {"GFxIME_Kr_2000"               	, GFxIME_Kr_2000},  
    {"GFxIME_Kr"                    	, GFxIME_Kr},       
    {"GFxIME_Kr_2003"	            	, GFxIME_Kr_2003},	
    {"GFxIME_Kr_8_6001"             	, GFxIME_Kr_8_6001},
    {"GFxIME_Kr_2007"	            	, GFxIME_Kr_2007},	
	{"GFxIME_Kr_2010"	            	, GFxIME_Kr_2010},
    {"GFxIME_Kr_2002"	            	, GFxIME_Kr_2002},	
    
    {"GFxIME_Ch_Simp_MSPinyin_3_0"  	, GFxIME_Ch_Simp_MSPinyin_3_0},
    {"GFxIME_Ch_Simp_QuanPin"       	, GFxIME_Ch_Simp_QuanPin},
    {"GFxIME_Ch_Simp_ShuangPin"     	, GFxIME_Ch_Simp_ShuangPin},
    {"GFxIME_Ch_Simp_ZhengMa"       	, GFxIME_Ch_Simp_ZhengMa},
    {"GFxIME_SogouPinyin"           	, GFxIME_SogouPinyin},
    {"GFxIME_GooglePinyin"          	, GFxIME_GooglePinyin},
    {"GFxIME_Ch_Simp_MSPinyin"      	, GFxIME_Ch_Simp_MSPinyin},     
    {"GFxIME_Ch_Simp_ABC"           	, GFxIME_Ch_Simp_ABC},          
    {"GFxIME_Ch_Simp"               	, GFxIME_Ch_Simp},              
    {"GFxIME_Ch_Simp_MSPinyin_2007" 	, GFxIME_Ch_Simp_MSPinyin_2007},
	{"GFxIME_Ch_Simp_MSPinyin1_2010"	, GFxIME_Ch_Simp_MSPinyin1_2010},
	{"GFxIME_Ch_Simp_MSPinyin2_2010"	, GFxIME_Ch_Simp_MSPinyin2_2010},
    {"GFxIME_Ch_Simp_WuBi86"        	, GFxIME_Ch_Simp_WuBi86},
    {"GFxIME_Ch_Simp_WuBi98"        	, GFxIME_Ch_Simp_WuBi98},
    {"GFxIME_Ch_Simp_JJ"            	, GFxIME_Ch_Simp_JJ},   
    {"GFxIME_Ch_Trad_WuXia"         	, GFxIME_Ch_Trad_WuXia},
    {""                             	, GFxIME_NotSet}
};
int StringToTagSize = sizeof(StringToTag)/sizeof(StringToTag[0]);
// This records which one of the two: Ime file name or layout text registry fields was used to 
// identify the IME.
//int ImeFileNameOrLayoutText[sizeof(SupportedIMEs)/sizeof(InputLangProps)];
int ReserveSupportedIMEsSize = sizeof(ReserveSupportedIMEs)/sizeof(InputLangProps);

UInt32  IMEHashFunction(const char *pchar, size_t length)
{
	return (UInt32)String::BernsteinHashFunctionCIS(pchar, length) & 0x00FFFFFF;
};

bool operator == (GImeStringNode* pnode, const GImeStringKey &key)
{
	if (pnode->Size != key.Length)
		return 0;
	// very important to do a case insensitive comparison-
	// subkey name that got hashed could be E0200411 while 
	// we could be comparing against e0200411.
	return (!SFstricmp(&pnode->data.ItemNameOnSystem[0], key.str.ToCStr()));
}

GImeNamesManager::GImeNamesManager(GFxIMEManagerWin32* pimemanager): pImeMgr(pimemanager)
{
	Current				= &Root;
	CurrentInputLangTag	= GFxIME_NotSet;
	CurrentIMETag		= GFxIME_NotSet;
	CachedConversionMode		= 0xffffffff;
	// Add empty node into the hash table
	EmptyStringNode.Size        = 0;
	//  EmptyStringNode.pData       =  "";
	StringSet.Add(&EmptyStringNode);
	NumIme			= sizeof(SupportedIMEs)/sizeof(InputLangProps);
	//	NumLanguages	= sizeof(SupportedInputLanguages)/sizeof(InputLangProps);
	NumLanguages	= 0;
}

void    GImeNamesManager::CleanUp()
{
	delete [] ImeFileNameOrLayoutText;
	Current = Root.pNextAlloc;
	while(Current)
	{
		GImeStringNode* tmp = Current;
		tmp->Release();
		Current = Current->pNextAlloc;
		delete tmp;
	}
};

void GImeNamesManager::PopulateSupportedIMEs()
{
	for (int i = 0; i < ReserveSupportedIMEsSize; i++)
	{
		InputLangProps2 inputLanguage;
		inputLanguage.ItemNameCommon	= ReserveSupportedIMEs[i].ItemNameCommon;
		inputLanguage.ItemNameOnSystem	= ReserveSupportedIMEs[i].ItemNameOnSystem;
		inputLanguage.ItemTag			= ReserveSupportedIMEs[i].ItemTag;
		SupportedIMEs.PushBack(inputLanguage);
		NumIme++;
	}
}

void GImeNamesManager::AllocateImeNameOrLayoutTextArray()
{
	ImeFileNameOrLayoutText = new int[NumIme];
}

// This function is called either when the input language or ime is changed using our language bar or the windows language bar
// The contents of the language bar must be reset when the ime is enabled again since it might have been out of
// sync with the system language bar when ime was disabled.

void GImeNamesManager::OnInputLangChange(DWORD langId)
{

	wchar_t szLangName[MAX_PATH];

	CurrentInputLangTag = GetLangNamefromLangId(0x0000FFFF&langId);
	// Also does broadcastswitchlanguage.
	pImeMgr->SetCurrentInputLanguage(CurrentInputLangTag);

	if ( GetLocaleInfoW(MAKELCID(langId, SORT_DEFAULT), LOCALE_SLANGUAGE, szLangName, MAX_PATH) == 0 )
	{
		DWORD err = GetLastError();
		LogError(err, "GFxIME Error: Error Initializing Input Language Name ");
#ifndef GFC_NO_LANGBAR_SUPPORT
		pImeMgr->BroadcastSwitchLanguage("Unsupported");
#endif
	}
	else
	{
		// If the user uses the language bar to change the input language, we should change
		// the text in the InputLangTab of our language bar in action script.
		// Why do we have a BroadcastSwitchLanguage and BroadcastSetCurrentInputLanguage?
		// Answer: BroadcastSwitchLanguage relays a generic name for the new input language
		// that we can switch on in ActionScript to display the correct kind of status window.
		// BroadcastSetCurrentInputLanguage on the other hand relays the actual input language name 
		// Provided by the system that is used in the InputLangTab of the language bar.
		// for example, the input language name for "Chinese (Traditional) is system dependent 
		// and could be  "Chinese (Traditional, Taiwan)" or "Chinese (Taiwan)"
		// in either case, BroadcastSwitchLanguage would relay "Chinese (Traditional)".
#ifndef GFC_NO_LANGBAR_SUPPORT
		pImeMgr->BroadcastSetCurrentInputLanguage(String(szLangName).ToCStr());
#endif
		// Here we try to distinguish between a keyboard layout and an IME
		// on XP, the HKL for IME's seem to start with an 'E'..This is not always
		// true so need to be careful here. 
		// So, if the langid is not an IME, we look for the layout name.
		// The layout name is the Handle-specific info shown below (taken from Michael Kaplan's
		// Blog)
		//  An HKL is a pointer which is the combination of a
		//  language ID and device-specific information.  The bits are
		//  allocated as follows:
		//
		//       +-----------------------+-------------------------+
		//       |  Handle-specific info |      Language ID        |
		//       +-----------------------+-------------------------+
		//        31                   16 15                      0   bit
		//
		// So for example, suppose the user installs Luxemboroughish and Belgian French keyboard layouts
		// for English input language. The keyboard layouts for Luxembourgish is 0x0000046E 
		// and for Belgian French is 0x0000080c(on my computer). When these layouts are installed
		// for English input language, the corresponding HKls are 0x080c0409 for Belgian French and 
		// 0x046E0409 for Luxembourgish. Therefore, to match the HKls against the keyboard layout obtained
		// from the registry, we need to only use the higher 16 bits.

		if ((langId & 0xE0000000) == 0)
		{
			CurrentIMETag = GFxIME_NotSupported;
			UInt32 numInstalledHKls = (UInt32)HklLayoutTextMap.GetSize();
			for (UInt32 i = 0; i < numInstalledHKls; i++)
			{
				if (((langId&0xFFFF0000) >> 16) == HklLayoutTextMap[i].hkl)
				{
					GImeNamesManager::SetLastIMEName(String(HklLayoutTextMap[i].LayoutName));
                    HideReadingWindow();
					return; 
				}
                
			}
			GImeNamesManager::SetLastIMEName(String(szLangName));
		}

		else
		{
			CurrentIMETag = GFxIME_NotSupported;
			for (UInt32 i = 0; i < NumIme; i++)
			{
				if (langId == (UInt32)SupportedIMEs[i].Id)
				{
					CurrentIMETag = SupportedIMEs[i].ItemTag;
					if (ImeFileNameOrLayoutText[i] == 2)
						GImeNamesManager::SetLastIMEName(String(SupportedIMEs[i].ItemNameCommon));
					if (ImeFileNameOrLayoutText[i] == 1)
						GImeNamesManager::SetLastIMEName(String(SupportedIMEs[i].ItemNameOnSystem));
					HideReadingWindow();
                    ImeVersionId = 0;
                    if (CurrentIMETag == GFxIME_Ch_Trad_NewChangJie)
                    {
                        ImeVersionId = GetImeId();
                        pImeMgr->SetImeVersionId(ImeVersionId);
                    }
                    break;
				}
			}
			if (CurrentIMETag == GFxIME_NotSupported)
				GImeNamesManager::SetLastIMEName(String("Not Supported"));
		}

		// Show the status window
		// For Japanese and Korean, the status window is shown when the user clicks on the IMENameTab. 
		// for Chinese, the status window should only be shown for supported IME's and not for third party IME's
		// since it doesn't work correctly for them.
#ifndef GFC_NO_LANGBAR_SUPPORT
		if (CurrentInputLangTag != GFxIME_Jp && CurrentInputLangTag != GFxIME_Kr && CurrentIMETag != GFxIME_NotSupported
			&& !(CurrentIMETag & (UInt32)GFxIME_Ch_ThirdParty_Flag))
			pImeMgr->BroadcastDisplayStatusWindow();
#endif
	}

}
// private API provided by CHT IME. Available on version 6.0 or later.
UInt32 (WINAPI*_GetReadinString)(HIMC himc, UInt32 uReadingBufLen, LPWSTR lpwReadingBuf, PINT pnErrorIndex, BOOL* pfIsVertical, PUINT puMaxReadingLen);
BOOL (WINAPI*_ShowReadingWindow)(HIMC himc, BOOL bShow);

void    GImeNamesManager::HideReadingWindow(void)
{
    char szImeFile[MAX_PATH + 1];
    HKL kl = GetKeyboardLayout(0);
    if ( ImmGetIMEFileNameA( kl, szImeFile, sizeof( szImeFile ) - 1 ) <= 0 )
        return;
    HMODULE hIme = LoadLibraryA( szImeFile );
    if ( !hIme )
        return;
    _GetReadinString = (UInt32 (WINAPI*)(HIMC, UInt32, LPWSTR, PINT, BOOL*, PUINT))
        (GetProcAddress(hIme, "GetReadinString"));
    _ShowReadingWindow =(BOOL (WINAPI*)(HIMC, BOOL))
        (GetProcAddress(hIme, "ShowReadingWindow"));
    if ( _ShowReadingWindow )
    {
        HIMC himc;
        if ( NULL != ( himc = ImmGetContext( pImeMgr->hWnd ) ) )
        {
            _ShowReadingWindow( himc, false );
            ImmReleaseContext( pImeMgr->hWnd, himc );
        }
    }
}

int GImeNamesManager::GetImeId()
{

    DWORD 	dwVerSize;
    DWORD 	dwVerHandle;
    LPVOID	lpVerBuffer;
    LPVOID	lpVerData;
    UINT	cbVerData;
    char    szTmp[1024];

    DWORD (WINAPI*_GetFileVersionInfoSizeA)(LPCSTR, LPDWORD);
    BOOL (WINAPI*_GetFileVersionInfoA)(LPCSTR, DWORD, DWORD, LPVOID);
    BOOL (WINAPI*_VerQueryValueA)(LPVOID, LPCSTR, LPVOID, PUINT);
    HMODULE hVersion = NULL;

    HKL kl = GetKeyboardLayout(0);

    // HKL is pointer-sized on x64; only the low word carries the language id.
    DWORD dwLang = (DWORD)((DWORD_PTR)kl & 0xffff);

    if ( ImmGetIMEFileNameA( kl, szTmp, sizeof( szTmp ) - 1 ) <= 0 )
    {
        goto error;
    }

    hVersion = LoadLibraryA("Version" );
    if ( !hVersion )
        goto error;
    _GetFileVersionInfoSizeA = (DWORD(WINAPI*)(LPCSTR, LPDWORD))
        (GetProcAddress(hVersion, "GetFileVersionInfoSizeA"));

    _GetFileVersionInfoA =(BOOL (WINAPI*)(LPCSTR, DWORD, DWORD, LPVOID))
        (GetProcAddress(hVersion, "GetFileVersionInfoA"));

    _VerQueryValueA =(BOOL (WINAPI*)(LPVOID, LPCSTR, LPVOID, PUINT))
        (GetProcAddress(hVersion, "VerQueryValueA"));

    // If fetching any of the above functions fails, bail out.

    if (!_GetFileVersionInfoSizeA || !_GetFileVersionInfoA || !_VerQueryValueA)
        goto error;

    dwVerSize = _GetFileVersionInfoSizeA( szTmp, &dwVerHandle );
    if ( dwVerSize )
    {
        lpVerBuffer = (char*)SF_ALLOC(dwVerSize, StatIME_Mem);
        if ( lpVerBuffer )
        {
            if ( _GetFileVersionInfoA(szTmp, dwVerHandle, dwVerSize, lpVerBuffer ) )
            {
                if ( _VerQueryValueA( lpVerBuffer, "\\", &lpVerData, &cbVerData ) )
                {
#define pVerFixedInfo ((VS_FIXEDFILEINFO FAR*)lpVerData)
                    DWORD dwVer = pVerFixedInfo->dwFileVersionMS;
                    dwVer = ( dwVer & 0x00ff0000 ) << 8 | ( dwVer & 0x000000ff ) << 16;

                    int versionId = dwVer | dwLang;
                //    dwRet[1] = pVerFixedInfo->dwFileVersionLS;
                    SF_FREE(lpVerBuffer);
                    return versionId;

#undef pVerFixedInfo
                }
            }
        }
        SF_FREE(lpVerBuffer);
    }

    // The flow comes here in the following conditions
    // - Non Chinese IME input locale
    // - Older Chinese IME
    // - Other error cases
error:
    return 0;
}

bool	GImeNamesManager::HashIMENames(void){ return true;};

// This function is called by the vista and xp derived classes when they need to set the name of the 
// default ime for a certain input language. We need access to the members of GFxIMEManagerWin32 class 
// to invoke the required AS function. This function is declared as a friend of the GFxIMEManagerWin32
// so it can access it's member variables. 

void	GImeNamesManager::SetLastIMEName(String imeName)
{
#ifndef GFC_NO_LANGBAR_SUPPORT
	// Note: This function is called from OnActivated. OnActivated is called when the input language/ IME changes.
	// It's called once with information about the old language, and second time with information about the new language.
	//	if (pImeMgr->StatusWindowPath.GetLength() != 0)
	//		pImeMgr->pMovie->Invoke((pImeMgr->StatusWindowPath + ".RemoveStatusWindow").ToCStr(), "%d", 0); 
	pImeMgr->BroadcastRemoveStatusWindow();

	//	pImeMgr->pMovie->Invoke((pImeMgr->LanguageBarPath + ".SetLastIME").ToCStr(), "%s", imeName.ToCStr()); 
	pImeMgr->BroadcastSetIMEName(imeName.ToCStr());

	// At this time, set the current input language
	// If condition is needed to take care of situation where user switches to an unsupported IME
	// using the language bar or keyboard shortcut. In this case, don't display status window.

	// pImeMgr->SetCurrentInputLanguage(CurrentInputLangTag);

	// Third party IME's status window doesn't work (clicking on it causes the LANG_CHANGE message to not be 
	// sent anymore) so don't display it. 
	if (CurrentIMETag != GFxIME_NotSupported && !(CurrentIMETag & (UInt32)GFxIME_Ch_ThirdParty_Flag))
	{	
		SetConversionMode(CachedConversionMode);
	}
#else
	SF_UNUSED(imeName); // To prevent warning
#endif
}

#ifndef GFC_NO_LANGBAR_SUPPORT

// This function is called when the lang bar is loaded for the first time to initialize the input language and ime fields.
void	GImeNamesManager::OnLangBarLoaded()
{
	SetLastIMEName("Not Supported");
	for (UInt32 i = 0; i < NumIme; i++)
	{
		if ((CurrentIMETag != GFxIME_NotSupported) && (CurrentIMETag == SupportedIMEs[i].ItemTag) && 
			(SupportedIMEs[i].Id != 0))
		{
			if (ImeFileNameOrLayoutText[i] == 2)
				SetLastIMEName(String(SupportedIMEs[i].ItemNameCommon));
			if (ImeFileNameOrLayoutText[i] == 1)
				SetLastIMEName(String(SupportedIMEs[i].ItemNameOnSystem));
			break;
		}
	}	
}

// Set contents of the status window display.
void GImeNamesManager::SetConversionMode(UInt32 fdwConversion)
{
	// IMM functions are used to obtain conversion status for both XP and Vista.


	DWORD		fdwSentence;
	String	convStatus;

	HIMC hIMC = ImmGetContext(pImeMgr->hWnd);
	if (!hIMC) return;

	if (fdwConversion == -1)
	{
		ImmGetConversionStatus(hIMC, (LPDWORD)&fdwConversion, &fdwSentence);
	}


	String statusWindowPath	= pImeMgr->StatusWindowPath;

	if (CurrentInputLangTag == GFxIME_Ch_Simp	|| 
		CurrentInputLangTag == GFxIME_Ch_Trad	|| 
		CurrentInputLangTag == GFxIME_Kr)
	{
		if (fdwConversion & IME_CMODE_NATIVE)
		{
			convStatus += String("Native") + ",";
		}

		else
		{
			convStatus += String("Alphanumeric") + ",";
		}

		// Shape Mode

		if (fdwConversion & IME_CMODE_FULLSHAPE)
		{
			convStatus += String("full") + ",";
		}


		else
		{
			convStatus += String("half") + ",";
		}

		// Symbol Mode

		if (fdwConversion & IME_CMODE_SYMBOL)
		{                 
			convStatus += String("full") + ",";
		}
		else
		{ 
			convStatus += String("half") + ",";
		}

	}

	if (CurrentInputLangTag == GFxIME_Jp)
	{
		if (ImmGetOpenStatus(hIMC) == 0)
		{
            JapaneseIMEState = false;
			convStatus += String("DirectInput") + ",";
		}
		else
		{
			if (fdwConversion & IME_CMODE_NATIVE)
			{
                JapaneseIMEState = true;
				if ((fdwConversion & IME_CMODE_FULLSHAPE) && (fdwConversion & IME_CMODE_KATAKANA))
				{
					convStatus += String("Katakana_FullShape") + ",";
				}

				else if((fdwConversion & IME_CMODE_FULLSHAPE) && !(fdwConversion & IME_CMODE_KATAKANA))
				{
					convStatus += String("Hiragana_FullShape") + ",";
				}

				else
				{ 
					convStatus += String("Katakana_HalfShape") + ",";
				}
			}
			else
			{
				if (fdwConversion & IME_CMODE_FULLSHAPE)
				{
					convStatus += String("Alphanumeric_FullShape") + ",";
				}
				else
				{
					convStatus += String("Alphanumeric_HalfShape") + ",";
				}
			}
		}
	}

	pImeMgr->BroadcastSetConversionStatus(convStatus.ToCStr());

	ImmReleaseContext(pImeMgr->hWnd, hIMC);
}

#endif

GFxIMETag GImeNamesManager::GetImeTag(const char* name)
{
	// If name == NULL, obtain the hash key using system specific method- on XP use 
	// GetKeyboardLayoutName to get the string corresponding to the current input language
	// and the make a Key out of it. If the user has passed a non null name, then call the 
	// base class method and use the name to return the corresponding tag. 
	// that to make a key. 
	GImeStringKey key;
	size_t length = SFstrlen(name);
	key = GImeStringKey((char*)name, IMEHashFunction(name, length), length);

	GImeStringNode** pnode = StringSet.Get(key);
	if(*pnode)
	{
		return (*pnode)->data.ItemTag;
	}

	return GFxIME_NotSupported;
}

GFxIMETag GImeNamesManager::GetInputLangTagfromImeTag(GFxIMETag imeTag)
{
	if (imeTag & (UInt32)GFxIME_Jp_Flag)			return GFxIME_Jp;
	if (imeTag & (UInt32)GFxIME_Kr_Flag)			return GFxIME_Kr;
	if (imeTag & (UInt32)GFxIME_En_Flag)			return GFxIME_English;
	if (imeTag & (UInt32)GFxIME_Ch_Trad_Flag)		return GFxIME_Ch_Trad;
	if (imeTag & (UInt32)GFxIME_Ch_Simp_Flag)		return GFxIME_Ch_Simp;
	if (imeTag & (UInt32)GFxIME_NotSupported_Flag)	return GFxIME_NotSupported;

	return GFxIME_NotSupported;
}

GFxIMETag	GImeNamesManager::GetLangNamefromLangId(UInt32 langId)
{
	for (UInt32 i = 0; i < NumLanguages; i++)
	{
		if (langId == (UInt32)SupportedInputLanguages[i].Id)
			return SupportedInputLanguages[i].ItemTag;
	}
	return GFxIME_NotSupported;
}

bool GImeNamesManager::MakeKeyboardLayoutListFromRegistry(HKL* hkls, UInt32 numInstalledIMEs)
{
	DWORD       cSubKeys=0;                 // number of subkeys
	DWORD       i, retCode;  
	char        achKey[MAX_KEY_LENGTH];     // buffer for subkey name
	FILETIME    ftLastWriteTime;            // last write time 
	HKEY		hKey;
	String	IMENames;

	if( (retCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts"),
		0,
		KEY_READ,
		&hKey)) != ERROR_SUCCESS)
	{
		LogError(retCode, "GFxIME Error:Opening Registry Key failed ");
		return 1;
	}

	if (hKey == NULL)
		return 1;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                   // key handle 
		NULL,                   // buffer for class name 
		NULL,                   // size of class string 
		NULL,                   // reserved 
		&cSubKeys,              // number of subkeys 
		NULL,                   // longest subkey size 
		NULL,                   // longest class string 
		NULL,                   // number of values for this key 
		NULL,                   // longest value name 
		NULL,                   // longest value data 
		NULL,                   // security descriptor 
		&ftLastWriteTime);      // last write time 

	if (retCode != ERROR_SUCCESS)
	{
		LogError(retCode, "GFxIME Error:Retrieving subkey information failed ");
		return 1;	
	}
	// Enumerate the subkeys, until RegEnumKeyEx fails.

	if (cSubKeys)
	{
		HKEY subKey;
		for (i = 0; i < cSubKeys; i++) 
		{ 
			// Note:
			// If the lpName buffer is too small to receive the name of the key, the function returns ERROR_MORE_DATA (0x000000EA).
			DWORD   cbName = MAX_KEY_LENGTH; 
			wchar_t subKeyLtVal[MAX_KEY_LENGTH]; // Layout Text
			wchar_t subKeyIfVal[MAX_KEY_LENGTH]; // IME File
			char    utf8BufLt[MAX_KEY_LENGTH*2];
			char    utf8BufIf[MAX_KEY_LENGTH*2];
			DWORD   subKeyValSize = MAX_KEY_LENGTH;

			retCode = RegEnumKeyExA(hKey, i,
				achKey, 
				&cbName, 
				NULL, 
				NULL, 
				NULL, 
				&ftLastWriteTime); 

			if (retCode != ERROR_SUCCESS) 
			{
				LogError(retCode, "GFxIME Error:Enumerating Registry Subkeys Failed ");
				continue;
			}

			// The registry key name is a 32-bit HKL value; parse it into an
			// unsigned int and zero-extend into the pointer-sized HKL member.
			unsigned int hklValue = 0;
			sscanf_s(achKey, "%x", &hklValue);
			Un.hkl = (HKL)(UINT_PTR)hklValue;
			
			retCode = RegOpenKeyExA(hKey, achKey, NULL, KEY_QUERY_VALUE, &subKey );

			if (retCode != ERROR_SUCCESS) 
			{
				LogError(retCode, "GFxIME Error:Opening Registry subkey Failed ");
				continue;
			}

			retCode = RegQueryValueExW(subKey, L"Layout Text", NULL, NULL, (LPBYTE)subKeyLtVal, &subKeyValSize);

			// Clear buffer if the required data couldn't be obtained successfully.
			if (retCode != ERROR_SUCCESS) 
			{
			//	LogError(retCode, "GFxIME Error:Obtaining Layout Text Failed "); // don't log this one for now since it's
				// always generated.
				memset(subKeyLtVal, 0, MAX_KEY_LENGTH*2);
			}

			subKeyValSize = MAX_KEY_LENGTH; // Very important to reset the size of buffer here because this is overwritten by the 
			// the RegQueryValueEx function with the size of the data written after each call. 

			retCode = RegQueryValueExW(subKey, L"IME File", NULL, NULL, (LPBYTE)subKeyIfVal, &subKeyValSize);

			// Registry has both keyboard layouts as well as IMEs.  If the first hex digit is 0,
			// the entry is a keyboard layout. If it's E, it's a CJK input method. The IME File
			// field only exists for IME, so we print the error message only for IME's

			if (retCode != ERROR_SUCCESS) 
			{
				if ((Un.val & 0xE000000) == 0xE000000)
					LogError(retCode, (char*)(String("GFxIME Error:Obtaining IME Filename failed for ") + achKey).ToCStr());

				memset(subKeyIfVal, 0, MAX_KEY_LENGTH*2);
			}

			// Close Registry Key
			RegCloseKey(subKey);

			// Convert to UTF8, since we save the IME table with UTF8 encoding
			UTF8Util::EncodeString(utf8BufLt, subKeyLtVal);

			// Convert to UTF8, since we save the IME table with UTF8 encoding
			UTF8Util::EncodeString(utf8BufIf, subKeyIfVal);

			HKlLayoutName props; 

#if defined(GFC_MSVC_SAFESTRING)
			sscanf_s(achKey, "%x", &props.hkl);
#else
			sscanf_s(achKey, "%x", &props.hkl);
#endif

			UInt32 size = (UInt32)SFstrlen(utf8BufLt)+1;
			char* pLayoutName = (char*)SF_ALLOC(size, StatIME_Mem);
			if (pLayoutName)
				SFstrcpy(pLayoutName, size, utf8BufLt);
			props.LayoutName = pLayoutName;

			size = (UInt32)SFstrlen(utf8BufIf)+1;
			char* pImeFileName = (char*)SF_ALLOC(size, StatIME_Mem);
			if (pImeFileName)
				SFstrcpy(pImeFileName, size, utf8BufIf);
			props.ImeFileName = pImeFileName;

			// Check if this ime is installed. Since we are only doing this at initialization time,
			// we can get away with this kind of dumb search. A better way would be to do a binary search
			bool installed = false;
			for (UInt32 installedIdx = 0; installedIdx < numInstalledIMEs; installedIdx++)
			{
				if (hkls[installedIdx] == Un.hkl)
				{
					installed = true;
					break;
				}
			}

			if (!installed)
			{
				props.IsInstalled = 0;
			}
			else
				props.IsInstalled = 1;

			HklLayoutTextMap.PushBack(props);
		}
	}

	RegCloseKey(hKey);
	return 0;
}

int GImeNamesManager::CheckForSupportedIME(const char* layoutTextName, const char* imeFileName)
{
	String			name;
	bool foundMatch		= false;
	UInt32 j				= 0;
	int index			= -1;
	GFxIMETag tag		= GFxIME_NotSet;
	UInt32 hasBeenFilled	= 0;

	// First match against LayoutName field, if not match is found, try ImeFileName. 
	for(j = 0; j < NumIme; j++)
	{
		// Its important to match ItemTag (which indicates the input language in use) since on Vista, different input languages
		// can have IME's that have the same name. For example, both Japanese and Korean languages have "Microsoft IME" as the name
		// of their IME. 
		if (layoutTextName && !SFstricmp(layoutTextName, SupportedIMEs[j].ItemNameOnSystem) /*&& (ImeFileNameOrLayoutText[j] == 0) */
			&& (CurrentInputLangTag ==  GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag)))
		{
			name						= SupportedIMEs[j].ItemNameOnSystem;
			hasBeenFilled				= (ImeFileNameOrLayoutText[j] == 0)? 0:1;
			ImeFileNameOrLayoutText[j]	= 1;
			foundMatch					= true;
			index						= j;
			break;
		}
	}
	
	if (foundMatch != true)
	{
		for(j = 0; j < NumIme; j++)
		{
			if (imeFileName && !SFstricmp(imeFileName, SupportedIMEs[j].ItemNameOnSystem) &&
				(CurrentInputLangTag ==  GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag)))
			{
				name						= SupportedIMEs[j].ItemNameCommon;
				hasBeenFilled				= (ImeFileNameOrLayoutText[j] == 0)? 0:1;
				ImeFileNameOrLayoutText[j]	= 2;
				foundMatch					= true;
				index						= j;
				tag = SupportedIMEs[j].ItemTag;
				break;
			}			
		}

		for(UInt32 jj = 0; jj < NumIme; jj++)
		{
			if ((ImeFileNameOrLayoutText[jj] == 1) && (SupportedIMEs[jj].ItemTag == tag))
			{
				foundMatch = false;
				break;
			}
		}
	}

	if ((foundMatch == true) && (!hasBeenFilled))
	{
		GFxIMETag inputLangTag = GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag);

		if (inputLangTag & (UInt32)GFxIME_Jp_Flag)
		{ 
			JapaneseIMEs.PushBack(name);
		}  

		if (inputLangTag & (UInt32)GFxIME_Kr_Flag)
		{ 
			KoreanIMEs.PushBack(name);
		} 

		if (inputLangTag & (UInt32)GFxIME_Ch_Simp_Flag)
		{ 
			ChineseSimpIMEs.PushBack(name);
		} 

		if (inputLangTag & (UInt32)GFxIME_Ch_Trad_Flag)
		{ 
			ChineseTradIMEs.PushBack(name);
		} 
	}

		return index;
}

void	GImeNamesManager::LogError(DWORD err, const char* context)
{
	LPSTR errorString;
	SF_UNUSED2(err, context); // to avoid warnings in release build since GFC_DEBUG_WARNING doesn't do anything for release blds.

	if (::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		0,
		(LPSTR)&errorString,
		0,
		NULL) == 0)
	{
		// Error in error reporting itself..
		// Print error message to the log.
		if(pImeMgr && pImeMgr->pLog)
		{
		//	pImeMgr->pLog->LogError("GFxIME Error: Error in error logging- FormatMessage failed!");
			SF_DEBUG_WARNING(1, "GFxIME Error: Error in error logging- FormatMessage failed!");
		}
	}
	if	(pImeMgr && pImeMgr->pLog)
	{
	//	pImeMgr->pLog->LogError(String(context) + ":" + (char*)errorString);
		SF_DEBUG_WARNING(1, String(context) + ":" + (char*)errorString + "\n");
	}
	::LocalFree(errorString);
}

void	GImeNamesManager::LogError(const char* error, const char* context)
{
	SF_UNUSED2(error, context); // to avoid warnings in release build since GFC_DEBUG_WARNING doesn't do anything for release blds.
	if	(pImeMgr && pImeMgr->pLog)
	{
	//	pImeMgr->pLog->LogError(String(context) + ":" + (char*)errorString);
		SF_DEBUG_WARNING(1, String(context) + ":" + error + "\n");
	}
}

void    GImeStringNode::Release()
{
	if(pImeNamesManager)
		pImeNamesManager->StringSet.Remove(this);

	SF_FREE((void*)data.ItemNameOnSystem);
};
}}}
#endif //#ifndef SF_NO_IME_SUPPORT
