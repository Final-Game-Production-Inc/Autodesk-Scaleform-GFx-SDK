/**************************************************************************

Filename    :   GFx_IMEIdMap.cpp
Content     :   Implementation of the IME names/id mapping classes for
                Win32 (registry based enumeration of installed IMEs and
                input languages).
Created     :   OCt 4, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include <stdint.h>

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_Allocator.h"
#include "Kernel/SF_String.h"

#include <windows.h>
#include <imm.h>
#include <msctf.h>

#include "GFx/IME/GFx_IMEManager.h"
#include "GFx_IMEIdMap.h"
#include "GFx_IMENamesManagerXP.h"
#include "GFx_IMENamesManagerVista.h"
#include "GFx_IMEManagerWin32.h"

//--------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------

namespace Scaleform {
namespace GFx {
namespace IME {

// Converts a wide string into a UTF-8 char* buffer allocated with SF_ALLOC.
static char* IMEWideToChar(const wchar_t* pstr)
{
    if (!pstr)
        return NULL;
    int len = WideCharToMultiByte(CP_UTF8, 0, pstr, -1, NULL, 0, NULL, NULL);
    if (len <= 0)
        return NULL;
    char* pbuf = (char*)SF_ALLOC(len, StatIME_Mem);
    WideCharToMultiByte(CP_UTF8, 0, pstr, -1, pbuf, len, NULL, NULL);
    return pbuf;
}

//--------------------------------------------------------------------
// Hash function used to build the GImeStringNode hash entries.
//--------------------------------------------------------------------

UInt32 IMEHashFunction(const char* pchar, size_t length)
{
    UInt32 hash = 2166136261u;
    for (size_t i = 0; i < length; i++)
    {
        hash ^= (UByte)pchar[i];
        hash *= 16777619u;
    }
    return hash & 0x00FFFFFF;
}

//--------------------------------------------------------------------
// GImeStringNode support
//--------------------------------------------------------------------

void GImeStringNode::Release()
{
    if (data.ItemNameOnSystem)
        SF_FREE((void*)data.ItemNameOnSystem);
    data.ItemNameOnSystem = NULL;
    data.ItemNameCommon   = NULL;
    delete this;
}

bool operator == (GImeStringNode* pnode, const GImeStringKey& key)
{
    if (!pnode)
        return key.str.IsEmpty();
    if (!pnode->data.ItemNameOnSystem)
        return key.str.IsEmpty();
    return SFstrcmp(pnode->data.ItemNameOnSystem, key.str.ToCStr()) == 0;
}

//--------------------------------------------------------------------
// Master table of supported IMEs. This is the compiled-in fallback
// list used to recognize IMEs by their registry "Layout Text" name or
// by their IME file name. The same data is copied into the
// GImeNamesManager::SupportedIMEs array by PopulateSupportedIMEs().
//--------------------------------------------------------------------

InputLangProps SupportedIMEs[] =
{
    {"Chinese (Traditional) - Phonetic",                     "Phonetic",                GFxIME_Ch_Trad_Phonetic,          0},
    {"phon.ime",                                             "Phonetic",                GFxIME_Ch_Trad_Phonetic,          0},
    {"TINTLGNT.IME",                                         "New Phonetic",            GFxIME_Ch_Trad_NewPhonetic,       0},
    {"MSTCIPHA.IME",                                         "New Phonetic",            GFxIME_Ch_Trad_NewPhonetic,       0},
    {"Chinese (Traditional) - New Phonetic",                 "New Phonetic",            GFxIME_Ch_Trad_NewPhonetic,       0},
    {"Chinese (Traditional) - New Phonetic 2010",            "New Phonetic 2010",       GFxIME_Ch_Trad_NewPhonetic,       0},
    {"Chinese (Traditional) - ChangJie",                     "ChangJie",                GFxIME_Ch_Trad_ChangJie,          0},
    {"chajei.ime",                                           "ChangJie",                GFxIME_Ch_Trad_ChangJie,          0},
    {"Chinese (Traditional) - New ChangJie",                 "New ChangJie",            GFxIME_Ch_Trad_NewChangJie,       0},
    {"Chinese (Traditional) - New ChangJie 2010",            "New ChangJie 2010",       GFxIME_Ch_Trad_NewChangJie,       0},
    {"CINTLGNT.IME",                                         "New ChangJie",            GFxIME_Ch_Trad_NewChangJie,       0},
    {"IMTCC14.IME",                                          "New ChangJie 2010",       GFxIME_Ch_Trad_NewChangJie2010,   0},
    {"IMTCP14.IME",                                          "New Phonetic 2010",       GFxIME_Ch_Trad_NewPhonetic2010,   0},
    {"IMTCQ14.IME",                                          "New Quick 2010",          GFxIME_Ch_Trad_NewQuick2010,      0},
    {"MSTCICJA.IME",                                         "New ChangJie",            GFxIME_Ch_Trad_NewChangJie,       0},
    {"Chinese (Traditional) - Big5 Code",                    "Big5",                    GFxIME_NotSupported,              0},
    {"Chinese (Traditional) - Array",                        "Array",                   GFxIME_Ch_Trad_Array,             0},
    {"Chinese Traditional Array (version 6.0)",              "Array",                   GFxIME_Ch_Trad_Array,             0},
    {"winar30.ime",                                          "Array",                   GFxIME_Ch_Trad_Array,             0},
    {"Chinese (Traditional) - Quick",                        "Quick",                   GFxIME_Ch_Trad_Quick,             0},
    {"quick.ime",                                            "Quick",                   GFxIME_Ch_Trad_Quick,             0},
    {"Chinese (Traditional) - DaYi",                         "DaYi",                    GFxIME_Ch_Trad_DaYi,              0},
    {"Chinese Traditional DaYi (version 6.0)",               "DaYi",                    GFxIME_Ch_Trad_DaYi,              0},
    {"dayi.ime",                                             "DaYi",                    GFxIME_Ch_Trad_DaYi,              0},
    {"Chinese (Traditional) - Unicode",                      "Unicode",                 GFxIME_NotSupported,              0},
    {"Chinese (Traditional) - US Keyboard",                  "Unknown",                 GFxIME_Ch_Trad,                   0},
    {"Chinese (Traditional) - Alphanumeric",                 "Alphanumeric",            GFxIME_NotSupported,              0},

    {"Microsoft IME",                                        "Japanese Microsoft IME",  GFxIME_Jp,                        0},
    {"imjp81.ime",                                           "Japanese Microsoft IME",  GFxIME_Jp,                        0},
    {"Japanese Input System (MS-IME2002)",                  "MS-IME 2002",             GFxIME_Jp_2002,                   0},
    {"imjp9.ime",                                            "Microsoft IME Standard 2003", GFxIME_Jp_2003,               0},
    {"imjp8.ime",                                            "Microsoft IME Standard 2002", GFxIME_Jp_2002,               0},
    {"imjp12.ime",                                           "MS-IME 2007",             GFxIME_Jp_2007,                   0},
    {"imjp14.ime",                                           "Japanese IME 2010",       GFxIME_Jp_2010,                   0},
    {"ATOK 2008",                                            "ATOK 2008",               GFxIME_Jp_ATOK2008,               0},
    {"ATOK 2009",                                            "ATOK 2009",               GFxIME_Jp_ATOK2009,               0},
    {"ATOK22W.IME",                                          "ATOK 2009",               GFxIME_Jp_ATOK2009,               0},
    {"Google Japanese Input",                               "Google Japanese Input",   GFxIME_Jp_GOOG2010,               0},

    {"Korean Input System (IME 2000)",                      "Korean 2000",             GFxIME_Kr,                        0},
    {"Korean",                                               "Korean",                  GFxIME_Kr,                        0},
    {"imekr61.ime",                                          "Korean IME 2000",         GFxIME_Kr_2000,                   0},
    {"imekr70.ime",                                          "Korean IME 2003",         GFxIME_Kr_2003,                   0},
    {"imkr12.ime",                                           "Korean IME 2007",         GFxIME_Kr_2007,                   0},
    {"imkr14.ime",                                           "Korean IME 2010",         GFxIME_Kr_2010,                   0},
    {"imekr.ime",                                            "Korean IME 2002",         GFxIME_Kr_2002,                   0},
    {"Microsoft IME",                                        "Korean Microsoft IME",    GFxIME_Kr,                        0},
    {"Microsoft \xed\x95\x9c\xea\xb8\x80 \xec\x9e\x85\xeb\xa0\xa5\xea\xb8\xb0", "Korean Microsoft IME", GFxIME_Kr_8_6001, 0},

    {"Chinese (Simplified) - US Keyboard",                  "Chinese (Simplified)",    GFxIME_Ch_Simp,                   0},
    {"Microsoft Pinyin",                                     "",                        GFxIME_Ch_Simp,                   0},
    {"Chinese (Simplified) - QuanPin",                      "Quan Pin",                GFxIME_Ch_Simp_QuanPin,           0},
    {"Chinese Simplified QuanPin (version 6.0)",            "Quan Pin",                GFxIME_Ch_Simp_QuanPin,           0},
    {"winpy.ime",                                            "Quan Pin",                GFxIME_Ch_Simp_QuanPin,           0},
    {"Chinese (Simplified) - ShuangPin",                    "Shuang Pin",              GFxIME_Ch_Simp_ShuangPin,         0},
    {"Chinese Simplified ShuangPin (version 6.0)",          "Shuang Pin",              GFxIME_Ch_Simp_ShuangPin,         0},
    {"winsp.ime",                                            "Shuang Pin",              GFxIME_Ch_Simp_ShuangPin,         0},
    {"Chinese (Simplified) - ZhengMa",                      "ZhengMa",                 GFxIME_Ch_Simp_ZhengMa,           0},
    {"Chinese Simplified ZhengMa (version 6.0)",            "Zheng Ma",                GFxIME_Ch_Simp_ZhengMa,           0},
    {"winzm.ime",                                            "ZhengMa",                 GFxIME_Ch_Simp_ZhengMa,           0},
    {"Chinese (Simplified) - NeiMa",                        "NeiMa",                   GFxIME_NotSupported,              0},
    {"Chinese (Simplified) - Microsoft Pinyin IME 3.0",     "MSPinyin IME 3.0",        GFxIME_Ch_Simp_MSPinyin_3_0,      0},
    {"Chinese (Simplified) - Microsoft Pinyin IME",         "MSPinyin IME",            GFxIME_Ch_Simp_MSPinyin,          0},
    {"Chinese (Simplified) - Microsoft Pinyin New Experience Input Style", "MSPinyin IME", GFxIME_Ch_Simp_MSPinyin, 0},
    {"pintlgnt.ime",                                         "Microsoft Pinyin IME 3.0", GFxIME_Ch_Simp_MSPinyin_3_0,    0},
    {"IMSC40A.IME",                                          "Microsoft Pinyin IME 3.0", GFxIME_Ch_Simp_MSPinyin_3_0,    0},
    {"IMSC12li.IME",                                         "Microsoft Pinyin IME 2007", GFxIME_Ch_Simp_MSPinyin_2007,  0},
    {"IMSC14.IME",                                           "Microsoft Pinyin IME 2010", GFxIME_Ch_Simp_MSPinyin1_2010,  0},
    {"\xe4\xb8\xad\xe6\x96\x87(\xe7\xae\x80\xe4\xbd\x93) - \xe5\xbe\xae\xe8\xbd\xaf\xe6\x8b\xbc\xe9\x9f\xb3\xe8\xbe\x93\xe5\x85\xa5\xe6\xb3\x95", "Microsoft Pinyin IME 3.0", GFxIME_Ch_Simp_MSPinyin_3_0, 0},
    {"winabc.ime",                                           "ABC IME",                GFxIME_Ch_Simp_ABC,               0},
    {"winabcx.ime",                                          "itABC IME v5.30",        GFxIME_Ch_Simp_ABC,               0},
    {"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE7\xBE\x8E\xE5\xBC\x8F\xE9\x94\xAE\xE7\x9B\x98", "MSPinyin IME 3.0", GFxIME_Ch_Simp_MSPinyin_3_0, 0},
    {"Chinese (Simplified) - Wubi86",                       "Wubi86",                  GFxIME_NotSupported,              0},
    {"Chinese (Simplified) - Wubi98",                       "Wubi98",                  GFxIME_NotSupported,              0},
    {"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE8\xB0\xB7\xE6\xAD\x8C\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "Google Pinyin", GFxIME_GooglePinyin, 0},
    {"\xE8\xB0\xB7\xE6\xAD\x8C\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "Google Pinyin", GFxIME_GooglePinyin, 0},
    {"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE6\x90\x9C\xE7\x8B\x97\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "Sogou Pinyin", GFxIME_SogouPinyin, 0},
    {"\xE8\xB0\xB7\xE6\xAD\x8C\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95 2.0", "Google Pinyin", GFxIME_GooglePinyin, 0},
    {"GOOGLEPINYIN2.IME",                                   "Google Pinyin",           GFxIME_GooglePinyin,              0},
    {"\xE8\xB0\xB7\xE6\xAD\x8C\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95 2", "Google Pinyin", GFxIME_GooglePinyin, 0},
    {"\xE6\x90\x9C\xE7\x8B\x97\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "Sogou Pinyin", GFxIME_SogouPinyin, 0},
    {"sogoupy.ime",                                          "Sogou Pinyin IME",        GFxIME_SogouPinyin,              0},
    {"BAIDUCN.IME",                                          "Baidu Pinyin IME",         GFxIME_Ch_Simp,              0},
    {"\xE4\xB8\xAD\xE6\x96\x87(\xE7\xAE\x80\xE4\xBD\x93) - \xE7\x99\xBE\xE5\xBA\xA6\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "Baidu Pinyin IME", GFxIME_Ch_Simp, 0},
    {"\xE4\xB8\xAD\xE6\x96\x87(\xE7\xAE\x80\xE4\xBD\x93) - \xE6\x90\x9C\xE7\x8B\x97\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "Sogou Pinyin IME", GFxIME_SogouPinyin, 0},
    {"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE5\x8F\x8C\xE6\x8B\xBC", "Shuang Pin", GFxIME_Ch_Simp_ShuangPin, 0},
    {"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE9\x83\x91\xE7\xA0\x81", "Zheng Ma", GFxIME_Ch_Simp_ZhengMa, 0},
    {"\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "Unknown", GFxIME_Ch_Simp_Pinyin03, 0},
    {"\xE4\xB8\xAD\xE6\x96\x87 - QQ\xE6\x8B\xBC\xE9\x9F\xB3\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "QQPinyin", GFxIME_Ch_Simp_QQPinyin, 0},
    {"\xE4\xB8\xAD\xE6\x96\x87 - QQ\xE4\xBA\x94\xE7\xAC\x94\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95", "QQPinyin", GFxIME_Ch_Simp_QQPinyin, 0},
    {"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE5\xBF\xB5\xE9\x9D\x92\xE7\xB9\x81\xE9\xAB\x94\xE4\xBA\x94\xE7\xAD\x86 2.03", "NianQing", GFxIME_Ch_Simp_NianQing, 0},
    {"\xE5\xBE\xAE\xE8\xBD\xAF\xE7\x8E\x8B\xE7\xA0\x81\xE4\xBA\x94\xE7\xAC\x94\x38\x36\xE7\x89\x88", "WuBi86", GFxIME_Ch_Simp_WuBi86, 0},
    {"WINWB86.IME",                                          "WuBi86",                  GFxIME_Ch_Simp_WuBi86,            0},
    {"\xE5\xBE\xAE\xE8\xBD\xAF\xE7\x8E\x8B\xE7\xA0\x81\xE4\xBA\x94\xE7\xAC\x94\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95\x20\x39\x38\xE7\x89\x88", "WuBi98", GFxIME_Ch_Simp_WuBi98, 0},
    {"WINWB98.IME",                                          "WuBi98",                  GFxIME_Ch_Simp_WuBi98,            0},
    {"\xE4\xB8\xAD\xE6\x96\x87 (\xE7\xAE\x80\xE4\xBD\x93) - \xE5\x8A\xA0\xE5\x8A\xA0\xE8\xBE\x93\xE5\x85\xA5\xE6\xB3\x95\x35\x2E\x30", "JJ Pinyin", GFxIME_Ch_Simp_JJ, 0},
    {"JJ.IME",                                               "JJ Pinyin",               GFxIME_Ch_Simp_JJ,                0},
    {"LIUNT.IME",                                            "Wu Xia",                  GFxIME_Ch_Trad_WuXia,             0},
    {"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe4\xbb\x93\xe9\xa2\x89", "ChangJie", GFxIME_Ch_Trad_ChangJie, 0},
    {"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe6\xb3\xa8\xe9\x9f\xb3", "Phonetic", GFxIME_Ch_Trad_Phonetic, 0},
    {"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe6\x96\xb0\xe6\xb3\xa8\xe9\x9f\xb3", "New Phonetic", GFxIME_Ch_Trad_NewPhonetic, 0},
    {"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe9\x80\x9f\xe6\x88\x90", "Quick", GFxIME_Ch_Trad_Quick, 0},
    {"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe6\x96\xb0\xe9\x80\x9f\xe6\x88\x90", "New Quick", GFxIME_Ch_Trad_NewQuick, 0},
    {"\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe9\x83\x91\xe7\xa0\x81(\xe7\x89\x88\xe6\x9c\xac 6.0", "Zheng Ma 6.0", GFxIME_Ch_Simp_ZhengMa, 0},
    {"\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe5\x85\xa8\xe6\x8b\xbc(\xe7\x89\x88\xe6\x9c\xac 6.0)", "Shuang Pin 6.0", GFxIME_Ch_Simp_ShuangPin, 0},
    {"\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe5\x8f\x8c\xe6\x8b\xbc(\xe7\x89\x88\xe6\x9c\xac 6.0)", "Quan Pin 6.0", GFxIME_Ch_Simp_QuanPin, 0},
    {"\xe5\xbe\xae\xe8\xbd\xaf\xe6\x8b\xbc\xe9\x9f\xb3\xe8\xbe\x93\xe5\x85\xa5\xe6\xb3\x95 2007", "Microsoft Pinyin IME 2007", GFxIME_Ch_Simp_MSPinyin_2007, 0},
    {"\xe4\xb8\xad\xe6\x96\x87(\xe7\xb9\x81\xe4\xbd\x93) - \xe6\x96\xb0\xe4\xbb\x93\xe9\xa2\x89", "New ChangJie", GFxIME_Ch_Trad_NewChangJie, 0},
    {"\xe7\xb9\x81\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe5\xa4\xa7\xe6\x98\x93(\xe7\x89\x88\xe6\x9c\xac 6.0)", "DaYi 6.0", GFxIME_Ch_Trad_DaYi, 0},
    {"\xe7\xb9\x81\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87\xe8\xa1\x8c\xe5\x88\x97(\xe7\x89\x88\xe6\x9c\xac 6.0)", "Array 6.0", GFxIME_Ch_Trad_Array, 0},
    {"\xec\xa4\x91\xea\xb5\xad\xec\x96\xb4 (\xeb\xb2\x88\xec\xb2\xb4) - ChangJie", "ChangJie", GFxIME_Ch_Trad_ChangJie, 0},
    {"\x4D\x69\x63\x72\x6f\x73\x6f\x66\x74 \xEC\x9E\x85\xeb\xa0\xa5\xea\xb8\xb0", "Korean IME 2007", GFxIME_Kr_2007, 0},
    {"\xec\xa4\x91\xea\xb5\xad\xec\x96\xb4 (\xeb\xb2\x88\xec\xb2\xb4) - Phonetic", "Phonetic", GFxIME_Ch_Trad_Phonetic, 0},
    {"\xec\xa4\x91\xea\xb5\xad\xec\x96\xb4 (\xeb\xb2\x88\xec\xb2\xb4) - New Phonetic", "New Phonetic", GFxIME_Ch_Trad_NewPhonetic, 0},
    {"\xec\xa4\x91\xea\xb5\xad\xec\x96\xb4 (\xeb\xb2\x88\xec\xb2\xb4) - New ChangJie", "New ChangJie", GFxIME_Ch_Trad_NewChangJie, 0},
    {"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe5\x80\x89\xe9\xa0\xa1", "ChangJie", GFxIME_Ch_Trad_ChangJie, 0},
    {"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe6\x96\xb0\xe5\x80\x89\xe9\xa0\xa1", "New ChangJie", GFxIME_Ch_Trad_NewChangJie, 0},
    {"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe6\xb3\xa8\xe9\x9f\xb3", "Phonetic", GFxIME_Ch_Trad_Phonetic, 0},
    {"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe6\x96\xb0\xe6\xb3\xa8\xe9\x9f\xb3", "New Phonetic", GFxIME_Ch_Trad_NewPhonetic, 0},
    {"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe9\x80\x9f\xe6\x88\x90", "Quick", GFxIME_Ch_Trad_Quick, 0},
    {"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb9\x81\xe9\xab\x94) - \xe6\x96\xb0\xe9\x80\x9f\xe6\x88\x90", "New Quick", GFxIME_Ch_Trad_NewQuick, 0},
    {"\xe4\xb8\xad\xe6\x96\x87\xe7\xb9\x81\xe9\xab\x94\xe5\xa4\xa7\xe6\x98\x93 (version 6.0)", "DaYi", GFxIME_Ch_Trad_DaYi, 0},
    {"\xe4\xb8\xad\xe6\x96\x87\xe7\xb9\x81\xe9\xab\x94\xe8\xa1\x8c\xe5\x88\x97 (version 6.0)", "Array", GFxIME_Ch_Trad_Array, 0},
    {"\xe4\xb8\xad\xe6\x96\x87 (\xe7\xb0\xa1\xe9\xab\x94) - \xe5\xbe\xae\xe8\xbb\x9f\xe6\x8b\xbc\xe9\x9f\xb3\xe8\xbc\xb8\xe5\x85\xa5\xe6\xb3\x95", "Microsoft Pinyin IME 3.0", GFxIME_Ch_Simp_MSPinyin_3_0, 0},
    {"\xe4\xb8\xad\xe6\x96\x87\xe7\xb0\xa1\xe9\xab\x94\xe5\x85\xa8\xe6\x8b\xbc (version 6.0)", "Shuang Pin", GFxIME_Ch_Simp_ShuangPin, 0},
    {"\xe4\xb8\xad\xe6\x96\x87\xe7\xb0\xa1\xe9\xab\x94\xe9\x84\xad\xe7\xa2\xbc (version 6.0)", "Zheng Ma", GFxIME_Ch_Simp_ZhengMa, 0},
    {"\xe4\xb8\xad\xe6\x96\x87\xe7\xb0\xa1\xe9\xab\x94\xe9\x9b\x99\xe6\x8b\xbc (version 6.0)", "Quan Pin", GFxIME_Ch_Simp_QuanPin, 0},
    {"Microsoft Pinyin IME 2007",                            "Microsoft Pinyin IME 2007", GFxIME_Ch_Simp_MSPinyin_2007,   0},
    {"CHEWING.IME",                                          "New Chewing IME",         GFxIME_Ch_Trad_NewChewing,        0},
    {NULL, NULL, GFxIME_NotSet, 0}
};

// Reserved storage for the supported IME list. These exported data symbols
// mirror the original libgfx_ime; evaluation builds may pre-reserve the
// table here before PopulateSupportedIMEs() fills it.
enum { ReserveSupportedIMEsCapacity = 64 };
InputLangProps ReserveSupportedIMEs[ReserveSupportedIMEsCapacity] = { {0, 0, GFxIME_NotSet, 0} };
int            ReserveSupportedIMEsSize = ReserveSupportedIMEsCapacity;

// Tag name <-> GFxIMETag mapping used by the XML (supported IME list) parser.
StringToTagDef g_StringToTagTable[] =
{
    {"GFxIME_English",                   GFxIME_English},
    {"GFxIME_Ch_Trad_Phonetic",         GFxIME_Ch_Trad_Phonetic},
    {"GFxIME_Ch_Trad_NewPhonetic",      GFxIME_Ch_Trad_NewPhonetic},
    {"GFxIME_Ch_Trad_ChangJie",         GFxIME_Ch_Trad_ChangJie},
    {"GFxIME_Ch_Trad_NewChangJie",      GFxIME_Ch_Trad_NewChangJie},
    {"GFxIME_Ch_Trad_NewChangJie2010",  GFxIME_Ch_Trad_NewChangJie2010},
    {"GFxIME_Ch_Trad_NewPhonetic2010",  GFxIME_Ch_Trad_NewPhonetic2010},
    {"GFxIME_Ch_Trad_NewQuick2010",     GFxIME_Ch_Trad_NewQuick2010},
    {"GFxIME_Ch_Trad_DaYi",             GFxIME_Ch_Trad_DaYi},
    {"GFxIME_Ch_Trad_Array",            GFxIME_Ch_Trad_Array},
    {"GFxIME_Ch_Trad_Quick",            GFxIME_Ch_Trad_Quick},
    {"GFxIME_Ch_Trad_NewQuick",         GFxIME_Ch_Trad_NewQuick},
    {"GFxIME_Jp_2002",                  GFxIME_Jp_2002},
    {"GFxIME_Jp_2007",                  GFxIME_Jp_2007},
    {"GFxIME_Jp_2010",                  GFxIME_Jp_2010},
    {"GFxIME_Jp_2003",                  GFxIME_Jp_2003},
    {"GFxIME_Jp",                       GFxIME_Jp},
    {"GFxIME_Jp_ATOK2008",              GFxIME_Jp_ATOK2008},
    {"GFxIME_Jp_GOOG2010",              GFxIME_Jp_GOOG2010},
    {"GFxIME_Kr_2000",                  GFxIME_Kr_2000},
    {"GFxIME_Kr",                       GFxIME_Kr},
    {"GFxIME_Kr_2003",                  GFxIME_Kr_2003},
    {"GFxIME_Kr_8_6001",                GFxIME_Kr_8_6001},
    {"GFxIME_Kr_2007",                  GFxIME_Kr_2007},
    {"GFxIME_Kr_2010",                  GFxIME_Kr_2010},
    {"GFxIME_Kr_2002",                  GFxIME_Kr_2002},
    {"GFxIME_Ch_Simp_MSPinyin_3_0",     GFxIME_Ch_Simp_MSPinyin_3_0},
    {"GFxIME_Ch_Simp_QuanPin",          GFxIME_Ch_Simp_QuanPin},
    {"GFxIME_Ch_Simp_ShuangPin",        GFxIME_Ch_Simp_ShuangPin},
    {"GFxIME_Ch_Simp_ZhengMa",          GFxIME_Ch_Simp_ZhengMa},
    {"GFxIME_SogouPinyin",              GFxIME_SogouPinyin},
    {"GFxIME_GooglePinyin",             GFxIME_GooglePinyin},
    {"GFxIME_Ch_Simp_MSPinyin",         GFxIME_Ch_Simp_MSPinyin},
    {"GFxIME_Ch_Simp_ABC",              GFxIME_Ch_Simp_ABC},
    {"GFxIME_Ch_Simp",                  GFxIME_Ch_Simp},
    {"GFxIME_Ch_Simp_MSPinyin_2007",    GFxIME_Ch_Simp_MSPinyin_2007},
    {"GFxIME_Ch_Simp_MSPinyin1_2010",   GFxIME_Ch_Simp_MSPinyin1_2010},
    {"GFxIME_Ch_Simp_MSPinyin2_2010",   GFxIME_Ch_Simp_MSPinyin2_2010},
    {"GFxIME_Ch_Simp_WuBi86",           GFxIME_Ch_Simp_WuBi86},
    {"GFxIME_Ch_Simp_WuBi98",           GFxIME_Ch_Simp_WuBi98},
    {"GFxIME_Ch_Simp_JJ",               GFxIME_Ch_Simp_JJ},
    {"GFxIME_Ch_Trad_WuXia",            GFxIME_Ch_Trad_WuXia},
    {"GFxIME_BaiduPinyin",              GFxIME_Ch_Simp},
    {"GFxIME_Ch_Simp_QQPinyin",         GFxIME_Ch_Simp_QQPinyin},
    {"GFxIME_Ch_Trad_NewChewing",       GFxIME_Ch_Trad_NewChewing},
    {"",                                GFxIME_NotSet}
};

//--------------------------------------------------------------------
// GImeNamesManager - base implementation
//--------------------------------------------------------------------

GImeNamesManager::GImeNamesManager(GFxIMEManagerWin32* pimeManager)
{
    pImeMgr             = pimeManager;
    NumLanguages        = 0;
    NumIme              = 0;
    CurrentInputLangTag = GFxIME_NotSet;
    CurrentIMETag       = GFxIME_NotSet;
    fdwConversion       = 0xffffffff;
    hKey                = NULL;
    ImeVersionId        = 0;
    ImeFileNameOrLayoutText = NULL;
    JapaneseIMEState    = false;
    mark                = false;
    mark_ime            = false;
    mark_displayname    = false;
    mark_tag            = false;
    label               = NONE;
    Current             = &Root;
    Root.HashFlags      = 0;
    Root.Size           = 0;
    Root.pNextAlloc     = NULL;
    Root.data.ItemTag   = GFxIME_NotSet;
    Root.data.ItemNameOnSystem = NULL;
}

bool GImeNamesManager::HashIMENames(void)
{
    // Release previously allocated nodes, then build the HKL list from the
    // registry and create a hash node for every layout so that GetImeTag()
    // can map a layout text name back to a GFxIMETag.
    CleanUp();

    if (HklLayoutTextMap.GetSize() == 0)
        MakeKeyboardLayoutListFromRegistry(NULL, 0);

    GImeStringNode* pNode = &Root;
    for (UPInt i = 0; i < HklLayoutTextMap.GetSize(); i++)
    {
        const char* pLayoutName = HklLayoutTextMap[i].LayoutName;
        GImeStringNode* pNewNode = new GImeStringNode;
        if (pLayoutName)
        {
            size_t len = SFstrlen(pLayoutName);
            char* pNameBuf = (char*)SF_ALLOC(len + 1, StatIME_Mem);
            SFstrcpy(pNameBuf, (UPInt)(len + 1), pLayoutName);
            pNewNode->data.ItemNameOnSystem = pNameBuf;
            pNewNode->HashFlags = IMEHashFunction(pLayoutName, len);
            pNewNode->Size      = (UInt32)len;
        }
        pNewNode->pImeNamesManager = this;
        pNode->pNextAlloc = pNewNode;
        pNode = pNewNode;
    }
    Current = pNode;
    return true;
}

void GImeNamesManager::CleanUp()
{
    GImeStringNode* pNode = Root.pNextAlloc;
    while (pNode)
    {
        GImeStringNode* pNext = pNode->pNextAlloc;
        pNode->Release();
        pNode = pNext;
    }
    Root.pNextAlloc = NULL;
    Current = &Root;

    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }
    HklLayoutTextMap.Clear();
    SupportedInputLanguages.Clear();
    SupportedIMEs.Clear();
    ChineseTradIMEs.Clear();
    ChineseSimpIMEs.Clear();
    JapaneseIMEs.Clear();
    KoreanIMEs.Clear();
    InstalledInputLangs = "";
}

GFxIMETag GImeNamesManager::GetImeTag(const char* name)
{
    if (name)
    {
        UInt32 hash = IMEHashFunction(name, SFstrlen(name));
        for (GImeStringNode* pNode = Root.pNextAlloc; pNode; pNode = pNode->pNextAlloc)
        {
            if (pNode->GetHashCode() == hash)
            {
                GImeStringKey key((char*)name, hash, SFstrlen(name));
                if (pNode == key)
                    return pNode->data.ItemTag;
            }
        }
    }
    return CurrentIMETag;
}

GFxIMETag GImeNamesManager::GetInputLangTagfromImeTag(GFxIMETag imeTag)
{
    if ((UInt32)imeTag & GFxIME_Jp_Flag)
        return GFxIME_Jp;
    if ((UInt32)imeTag & GFxIME_Kr_Flag)
        return GFxIME_Kr;
    if ((UInt32)imeTag & GFxIME_Ch_Simp_Flag)
        return GFxIME_Ch_Simp;
    if ((UInt32)imeTag & GFxIME_Ch_Trad_Flag)
        return GFxIME_Ch_Trad;
    if ((UInt32)imeTag & GFxIME_En_Flag)
        return GFxIME_English;
    return GFxIME_NotSupported;
}

GFxIMETag GImeNamesManager::GetLangNamefromLangId(UInt32 langId)
{
    switch (langId & 0x0000FFFF)
    {
    case 0x0411: return GFxIME_Jp;
    case 0x0412: return GFxIME_Kr;
    case 0x0404: return GFxIME_Ch_Trad;
    case 0x0804: return GFxIME_Ch_Simp;
    case 0x0409: // English US
    case 0x0809: // English UK
    case 0x0c09: // English Aus
    case 0x1009: // English Canada
        return GFxIME_English;
    default:
        return GFxIME_NotSupported;
    }
}

bool GImeNamesManager::MakeKeyboardLayoutListFromRegistry(HKL* pHKLList, UInt32 numHKLs)
{
    HklLayoutTextMap.Clear();

    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    UInt32 nInputLanguages = 0;
    HKL*   pHKLs = NULL;
    bool   bOwnArray = false;

    if (numHKLs != 0 && pHKLList)
    {
        pHKLs = pHKLList;
        nInputLanguages = numHKLs;
    }
    else
    {
        nInputLanguages = GetKeyboardLayoutList(0, NULL);
        if (nInputLanguages == 0)
            return false;
        pHKLs = (HKL*)SF_ALLOC(sizeof(HKL) * nInputLanguages, StatIME_Mem);
        bOwnArray = true;
        nInputLanguages = GetKeyboardLayoutList(nInputLanguages, pHKLs);
        if (nInputLanguages == 0)
        {
            SF_FREE(pHKLs);
            return false;
        }
    }

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts",
                      0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        if (bOwnArray)
            SF_FREE(pHKLs);
        return false;
    }

    char szSubKey[16];
    WCHAR wszSubKey[16];
    WCHAR wszValue[256];
    DWORD dwValueLen;
    DWORD dwType;

    for (UInt32 i = 0; i < nInputLanguages; i++)
    {
        size_t hkl = (size_t)pHKLs[i];

        SFsprintf(szSubKey, sizeof(szSubKey), "%08X", (UInt32)hkl);
        for (int j = 0; j < 8; j++)
        {
            wszSubKey[j] = (WCHAR)szSubKey[j];
        }
        wszSubKey[8] = 0;

        HKlLayoutName layout;
        layout.hkl         = (UInt32)hkl;
        layout.IsInstalled = 1;
        layout.ImeFileName = NULL;
        layout.LayoutName  = NULL;

        // Query "Layout Text".
        dwValueLen = 256;
        dwType = 0;
        if (RegQueryValueExW(hKey, L"Layout Text", NULL, &dwType,
                             (LPBYTE)wszValue, &dwValueLen) == ERROR_SUCCESS &&
            dwValueLen > 0)
        {
            wszValue[dwValueLen / 2] = 0;
            layout.LayoutName = IMEWideToChar(wszValue);
        }

        // Query "IME File".
        dwValueLen = 256;
        dwType = 0;
        if (RegQueryValueExW(hKey, L"IME File", NULL, &dwType,
                             (LPBYTE)wszValue, &dwValueLen) == ERROR_SUCCESS &&
            dwValueLen > 0)
        {
            wszValue[dwValueLen / 2] = 0;
            layout.ImeFileName = IMEWideToChar(wszValue);
        }

        HklLayoutTextMap.PushBack(layout);
    }

    if (bOwnArray)
        SF_FREE(pHKLs);

    return HklLayoutTextMap.GetSize() > 0;
}

void GImeNamesManager::OnInputLangChange(DWORD langId)
{
    CurrentInputLangTag = GetLangNamefromLangId(langId);
}

void GImeNamesManager::SetLastIMEName(String imeName)
{
    // Records the last matched IME name; kept for the test framework.
    SF_UNUSED(imeName);
}

void GImeNamesManager::HideReadingWindow()
{
    // No-op in the base implementation. The Win32 impl classes take care
    // of removing the reading window through the candidate list UI.
}

int GImeNamesManager::GetImeId()
{
    return ImeVersionId;
}

void GImeNamesManager::SetConversionMode(UInt32 conversionParams)
{
    if (conversionParams != 0xffffffff)
        fdwConversion = conversionParams;
}

void GImeNamesManager::OnLangBarLoaded()
{
    // Nothing to do in the base implementation.
}

void GImeNamesManager::PopulateSupportedIMEs()
{
    // Note: inside a member function, "SupportedIMEs" resolves to the member
    // array; the compiled-in table is the namespace global of the same name.
    InputLangProps* pGlobalTable = ::Scaleform::GFx::IME::SupportedIMEs;
    SupportedIMEs.Clear();
    for (int i = 0; pGlobalTable[i].ItemNameOnSystem != NULL || pGlobalTable[i].ItemTag != GFxIME_NotSet; i++)
    {
        InputLangProps2 entry;
        entry.ItemNameOnSystem = pGlobalTable[i].ItemNameOnSystem ? pGlobalTable[i].ItemNameOnSystem : "";
        entry.ItemNameCommon   = pGlobalTable[i].ItemNameCommon   ? pGlobalTable[i].ItemNameCommon   : "";
        entry.ItemTag          = pGlobalTable[i].ItemTag;
        entry.Id               = pGlobalTable[i].Id;
        SupportedIMEs.PushBack(entry);
    }
    NumIme = (UInt32)SupportedIMEs.GetSize();
}

void GImeNamesManager::AllocateImeNameOrLayoutTextArray()
{
    // Allocated on demand by the XML parser.
    if (!ImeFileNameOrLayoutText)
        ImeFileNameOrLayoutText = (int*)SF_ALLOC(sizeof(int) * 4, StatIME_Mem);
}

void GImeNamesManager::LogError(DWORD err, char* pContext)
{
    if (pImeMgr && pImeMgr->pLog)
        pImeMgr->pLog->LogMessage("GFxIME: Error 0x%08X in %s\n", err, pContext ? pContext : "");
}

void GImeNamesManager::LogError(const char* error, const char* context)
{
    if (pImeMgr && pImeMgr->pLog)
        pImeMgr->pLog->LogMessage("GFxIME: %s (context: %s)\n", error ? error : "", context ? context : "");
}

//--------------------------------------------------------------------
// GImeNamesManagerXP - IMM based implementation for Windows XP
//--------------------------------------------------------------------

int GImeNamesManager::CheckForSupportedIME(const char* layoutTextName, const char* imeFileName)
{
    if (!layoutTextName && !imeFileName)
        return -1;

    for (UPInt i = 0; i < SupportedIMEs.GetSize(); i++)
    {
        const char* pSysName = SupportedIMEs[i].ItemNameOnSystem.ToCStr();
        if (layoutTextName && pSysName && SFstrcmp(layoutTextName, pSysName) == 0)
            return (int)i;
        if (imeFileName && pSysName && SFstrcmp(imeFileName, pSysName) == 0)
            return (int)i;
    }
    return -1;
}

bool GImeNamesManagerXP::HashIMENames(void)
{
    if (!GImeNamesManager::HashIMENames())
        return false;

    // Go through each HKL and figure out which IME it corresponds to.
    for (UPInt i = 0; i < HklLayoutTextMap.GetSize(); i++)
    {
        int idx = CheckForSupportedIME(HklLayoutTextMap[i].LayoutName,
                                       HklLayoutTextMap[i].ImeFileName);
        if (idx >= 0)
        {
            CurrentIMETag = SupportedIMEs[idx].ItemTag;
            break;
        }
    }
    return true;
}

GImeStringKey GImeNamesManagerXP::GetKey()
{
    const char* pName = Current->data.ItemNameOnSystem;
    if (!pName)
        return GImeStringKey();
    size_t len = SFstrlen(pName);
    return GImeStringKey(pName, IMEHashFunction(pName, len), len);
}

void GImeNamesManagerXP::ActivateIME(const char* imeName)
{
    if (!imeName)
        return;
    // Find the HKL whose layout text (or IME file name) matches imeName.
    for (UPInt i = 0; i < HklLayoutTextMap.GetSize(); i++)
    {
        const char* pLayout = HklLayoutTextMap[i].LayoutName;
        const char* pFile   = HklLayoutTextMap[i].ImeFileName;
        if ((pLayout && SFstrcmp(pLayout, imeName) == 0) ||
            (pFile   && SFstrcmp(pFile,   imeName) == 0))
        {
            ActivateKeyboardLayout((HKL)(size_t)HklLayoutTextMap[i].hkl, KLF_REORDER);
            return;
        }
    }
}

void GImeNamesManagerXP::ActivateInputLanguage(const char* inputLangName)
{
    if (!inputLangName)
        return;
    for (UPInt i = 0; i < SupportedInputLanguages.GetSize(); i++)
    {
        if (SupportedInputLanguages[i].ItemNameCommon &&
            SFstrcmp(SupportedInputLanguages[i].ItemNameCommon, inputLangName) == 0)
        {
            HKL hkl = (HKL)(size_t)SupportedInputLanguages[i].Id;
            ActivateKeyboardLayout(hkl, KLF_REORDER);
            return;
        }
    }
}

void GImeNamesManagerXP::HandleStatusWindowNotifications(const char* pcommand, const char* parg)
{
    if (!pcommand)
        return;
    if (SFstrcmp(pcommand, "setconversionmode") == 0 && parg)
        SetConversionMode((UInt32)atoi(parg));
}

void GImeNamesManagerXP::OnLangBarLoaded()
{
    if (pImeMgr && pImeMgr->pLog)
        pImeMgr->pLog->LogMessage(
            "GFxIME: Language bar loaded. %d IME(s) installed.\n", NumIme);
}

bool GImeNamesManagerXP::SwitchIME(String imeName)
{
    ActivateIME(imeName.ToCStr());
    return true;
}

void GImeNamesManagerXP::CheckForSupportedInputLang(TF_LANGUAGEPROFILE& langProfile,
                                                    const wchar_t* inputLangName)
{
    InputLangProps langProps;
    langProps.ItemNameOnSystem = inputLangName ? IMEWideToChar(inputLangName) : NULL;
    langProps.ItemNameCommon   = langProps.ItemNameOnSystem;
    langProps.ItemTag          = GetLangNamefromLangId(langProfile.langid);
    langProps.Id               = (SPInt)langProfile.langid;
    SupportedInputLanguages.PushBack(langProps);
    NumLanguages++;
}

int GImeNamesManagerXP::CheckForSupportedIME(const char* layoutTextName, const char* imeFileName)
{
    if (!layoutTextName && !imeFileName)
        return -1;

    for (UPInt i = 0; i < SupportedIMEs.GetSize(); i++)
    {
        const char* pSysName = SupportedIMEs[i].ItemNameOnSystem.ToCStr();
        if (layoutTextName && pSysName && SFstrcmp(layoutTextName, pSysName) == 0)
            return (int)i;
        if (imeFileName && pSysName && SFstrcmp(imeFileName, pSysName) == 0)
            return (int)i;
    }
    return -1;
}

GFxIMETag GImeNamesManagerXP::GetImeTag(const char* name)
{
    if (!name)
        return CurrentIMETag;
    for (UPInt i = 0; i < SupportedIMEs.GetSize(); i++)
    {
        if (SFstrcmp(SupportedIMEs[i].ItemNameOnSystem.ToCStr(), name) == 0)
            return SupportedIMEs[i].ItemTag;
    }
    return CurrentIMETag;
}

//--------------------------------------------------------------------
// GImeNamesManagerVista - TSF based implementation for Vista and up
//--------------------------------------------------------------------

GImeNamesManagerVista::GImeNamesManagerVista(GFxIMEManagerWin32* pimemanager)
    : GImeNamesManager(pimemanager)
{
    dwInputProcessorProfileEventSinkCookie = TF_INVALID_COOKIE;
    dwCompartmentSinkCookie                = TF_INVALID_COOKIE;
    cRef                                   = 1;
    pCompartment                           = NULL;
    pThreadMgr                             = NULL;
    pClientId                              = 0;
}

STDMETHODIMP_(HRESULT) GImeNamesManagerVista::OnActivated(DWORD dwProfileType,
                                                          LANGID langid,
                                                          REFCLSID clsid,
                                                          REFGUID catid,
                                                          REFGUID guidProfile,
                                                          HKL hkl,
                                                          DWORD dwFlags)
{
    SF_UNUSED3(clsid, catid, dwFlags);

    if (dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR)
    {
        // A TIP got activated - query its display name and check whether
        // it is one of our supported IMEs.
        ITfInputProcessorProfileMgr* pProfileMgr = NULL;
        if (pThreadMgr && SUCCEEDED(pThreadMgr->QueryInterface(
                IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr)))
        {
            BSTR bstrProfile = NULL;
            ITfInputProcessorProfiles* pProfiles = NULL;
            if (SUCCEEDED(pThreadMgr->QueryInterface(
                    IID_ITfInputProcessorProfiles, (void**)&pProfiles)))
            {
                pProfiles->GetLanguageProfileDescription(clsid, langid, guidProfile, &bstrProfile);
                pProfiles->Release();
            }
            if (bstrProfile)
            {
                TF_INPUTPROCESSORPROFILE profile;
                memset(&profile, 0, sizeof(profile));
                profile.dwProfileType = TF_PROFILETYPE_INPUTPROCESSOR;
                profile.langid        = langid;
                profile.clsid         = clsid;
                profile.guidProfile   = guidProfile;
                profile.hkl           = hkl;
                CheckForSupportedIME(profile, (const char*)NULL);
                SysFreeString(bstrProfile);
            }
            pProfileMgr->Release();
        }
    }
    else if (dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT)
    {
        InputLangProps langProps;
        langProps.ItemNameOnSystem = NULL;
        langProps.ItemNameCommon   = NULL;
        langProps.ItemTag          = GetLangNamefromLangId(langid);
        langProps.Id               = (SPInt)hkl;
        SupportedInputLanguages.PushBack(langProps);
        NumLanguages++;
    }
    return S_OK;
}

STDMETHODIMP GImeNamesManagerVista::OnChange(REFGUID refguid)
{
    SF_UNUSED(refguid);
    return S_OK;
}

STDMETHODIMP GImeNamesManagerVista::QueryInterface(REFIID riid, void** ppvObj)
{
    if (!ppvObj)
        return E_INVALIDARG;
    *ppvObj = NULL;
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfInputProcessorProfileActivationSink_GFx))
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) GImeNamesManagerVista::AddRef()
{
    return InterlockedIncrement(&cRef);
}

STDMETHODIMP_(ULONG) GImeNamesManagerVista::Release()
{
    LONG nRef = InterlockedDecrement(&cRef);
    if (nRef == 0)
        delete this;
    return nRef;
}

bool GImeNamesManagerVista::HashIMENames(void)
{
    if (!GImeNamesManager::HashIMENames())
        return false;

    HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ITfThreadMgr, (void**)&pThreadMgr);
    if (FAILED(hr) || !pThreadMgr)
        return false;

    // Enumerate the currently installed TIPs.
    ITfInputProcessorProfiles* pProfiles = NULL;
    hr = pThreadMgr->QueryInterface(IID_ITfInputProcessorProfiles, (void**)&pProfiles);
    if (SUCCEEDED(hr) && pProfiles)
    {
        LANGID* pLangIds = NULL;
        ULONG   langCount = 0;
        if (SUCCEEDED(pProfiles->GetLanguageList(&pLangIds, &langCount)))
        {
            ITfInputProcessorProfileMgr* pProfileMgr = NULL;
            if (SUCCEEDED(pProfiles->QueryInterface(
                    IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr)) && pProfileMgr)
            {
                for (ULONG li = 0; li < langCount; li++)
                {
                    IEnumTfInputProcessorProfiles* pEnum = NULL;
                    if (FAILED(pProfileMgr->EnumProfiles(pLangIds[li], &pEnum)) || !pEnum)
                        continue;
                    TF_INPUTPROCESSORPROFILE profile;
                    ULONG fetched = 0;
                    while (pEnum->Next(1, &profile, &fetched) == S_OK && fetched > 0)
                    {
                        if (profile.dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR)
                        {
                            BSTR bstrDesc = NULL;
                            pProfiles->GetLanguageProfileDescription(profile.clsid, profile.langid,
                                                                     profile.guidProfile, &bstrDesc);
                            if (bstrDesc)
                            {
                                char* pDesc = IMEWideToChar(bstrDesc);
                                CheckForSupportedIME(profile, pDesc);
                                if (pDesc) SF_FREE(pDesc);
                                SysFreeString(bstrDesc);
                            }
                        }
                        fetched = 0;
                    }
                    pEnum->Release();
                }
                pProfileMgr->Release();
            }
            CoTaskMemFree(pLangIds);
        }
        pProfiles->Release();
    }

    InitCompartmentSink();
    return true;
}

void GImeNamesManagerVista::CheckForSupportedIME(TF_INPUTPROCESSORPROFILE& profile,
                                                 const char* imeName)
{
    SF_UNUSED(profile);
    if (!imeName)
        return;
    for (UPInt i = 0; i < SupportedIMEs.GetSize(); i++)
    {
        if (SFstrcmp(SupportedIMEs[i].ItemNameOnSystem.ToCStr(), imeName) == 0 ||
            SFstrcmp(SupportedIMEs[i].ItemNameCommon.ToCStr(), imeName) == 0)
        {
            CurrentIMETag = SupportedIMEs[i].ItemTag;
            return;
        }
    }
}

void GImeNamesManagerVista::CheckForSupportedInputLang(TF_INPUTPROCESSORPROFILE& langProfile,
                                                       const wchar_t* inputLangName)
{
    InputLangProps langProps;
    langProps.ItemNameOnSystem = inputLangName ? IMEWideToChar(inputLangName) : NULL;
    langProps.ItemNameCommon   = langProps.ItemNameOnSystem;
    langProps.ItemTag          = GetLangNamefromLangId(langProfile.langid);
    langProps.Id               = (SPInt)langProfile.hkl;
    SupportedInputLanguages.PushBack(langProps);
    NumLanguages++;
}

void GImeNamesManagerVista::ActivateIME(const char* imeName)
{
    if (!imeName)
        return;
    ITfInputProcessorProfiles* pProfiles = NULL;
    if (pThreadMgr && SUCCEEDED(pThreadMgr->QueryInterface(
            IID_ITfInputProcessorProfiles, (void**)&pProfiles)) && pProfiles)
    {
        LANGID* pLangIds = NULL;
        ULONG   langCount = 0;
        if (SUCCEEDED(pProfiles->GetLanguageList(&pLangIds, &langCount)))
        {
            ITfInputProcessorProfileMgr* pProfileMgr = NULL;
            if (SUCCEEDED(pProfiles->QueryInterface(
                    IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr)) && pProfileMgr)
            {
                bool bActivated = false;
                for (ULONG li = 0; li < langCount && !bActivated; li++)
                {
                    IEnumTfInputProcessorProfiles* pEnum = NULL;
                    if (FAILED(pProfileMgr->EnumProfiles(pLangIds[li], &pEnum)) || !pEnum)
                        continue;
                    TF_INPUTPROCESSORPROFILE profile;
                    ULONG fetched = 0;
                    while (pEnum->Next(1, &profile, &fetched) == S_OK && fetched > 0)
                    {
                        BSTR bstrDesc = NULL;
                        pProfiles->GetLanguageProfileDescription(profile.clsid, profile.langid,
                                                                 profile.guidProfile, &bstrDesc);
                        if (bstrDesc)
                        {
                            char* pDesc = IMEWideToChar(bstrDesc);
                            if (pDesc && SFstrcmp(pDesc, imeName) == 0)
                            {
                                pProfiles->ActivateLanguageProfile(profile.clsid, profile.langid,
                                                                   profile.guidProfile);
                                if (pDesc) SF_FREE(pDesc);
                                SysFreeString(bstrDesc);
                                bActivated = true;
                                break;
                            }
                            if (pDesc) SF_FREE(pDesc);
                            SysFreeString(bstrDesc);
                        }
                        fetched = 0;
                    }
                    pEnum->Release();
                }
                pProfileMgr->Release();
            }
            CoTaskMemFree(pLangIds);
        }
        pProfiles->Release();
    }
}

void GImeNamesManagerVista::ActivateInputLanguage(const char* inputLangName)
{
    if (!inputLangName)
        return;
    for (UPInt i = 0; i < SupportedInputLanguages.GetSize(); i++)
    {
        if (SupportedInputLanguages[i].ItemNameCommon &&
            SFstrcmp(SupportedInputLanguages[i].ItemNameCommon, inputLangName) == 0)
        {
            HKL hkl = (HKL)(size_t)SupportedInputLanguages[i].Id;
            if (hkl)
                ActivateKeyboardLayout(hkl, KLF_REORDER);
            return;
        }
    }
}

void GImeNamesManagerVista::SetConversionMode(UInt32 conversionParams)
{
    // Store the requested mode; it is applied through the TSF compartment.
    GImeNamesManager::SetConversionMode(conversionParams);
    if (pCompartment)
    {
        VARIANT var;
        var.vt = VT_I4;
        var.lVal = (LONG)(conversionParams == 0xffffffff ? 0 : conversionParams);
        pCompartment->SetValue(pClientId, &var);
    }
}

void GImeNamesManagerVista::HandleStatusWindowNotifications(const char* pcommand, const char* parg)
{
    if (!pcommand)
        return;
    if (SFstrcmp(pcommand, "setconversionmode") == 0 && parg)
        SetConversionMode((UInt32)atoi(parg));
}

void GImeNamesManagerVista::OnLangBarLoaded()
{
    if (pImeMgr && pImeMgr->pLog)
        pImeMgr->pLog->LogMessage(
            "GFxIME: Language bar loaded. %d IME(s) installed.\n", NumIme);
}

GFxIMETag GImeNamesManagerVista::GetImeTag(const char* name)
{
    if (!name)
        return CurrentIMETag;
    for (UPInt i = 0; i < SupportedIMEs.GetSize(); i++)
    {
        if (SFstrcmp(SupportedIMEs[i].ItemNameOnSystem.ToCStr(), name) == 0)
            return SupportedIMEs[i].ItemTag;
    }
    return CurrentIMETag;
}

void GImeNamesManagerVista::OnInputLangChange(DWORD langId)
{
    GImeNamesManager::OnInputLangChange(langId);
    if (pImeMgr)
        pImeMgr->BroadcastSetCurrentInputLanguage("");
}

void GImeNamesManagerVista::UnInstallProfileActivationSink()
{
    ITfSource* pSource = NULL;
    if (dwInputProcessorProfileEventSinkCookie != TF_INVALID_COOKIE && pThreadMgr)
    {
        if (SUCCEEDED(pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource)) && pSource)
        {
            pSource->UnadviseSink(dwInputProcessorProfileEventSinkCookie);
            pSource->Release();
        }
        dwInputProcessorProfileEventSinkCookie = TF_INVALID_COOKIE;
    }
}

bool GImeNamesManagerVista::SwitchIME(String imeName)
{
    ActivateIME(imeName.ToCStr());
    return true;
}

BOOL GImeNamesManagerVista::InitCompartmentSink()
{
    if (!pThreadMgr)
        return FALSE;
    ITfCompartmentMgr* pCompMgr = NULL;
    HRESULT hr = pThreadMgr->GetGlobalCompartment(&pCompMgr);
    if (FAILED(hr) || !pCompMgr)
        return FALSE;
    hr = pCompMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
                                  &pCompartment);
    pCompMgr->Release();
    if (FAILED(hr) || !pCompartment)
        return FALSE;

    ITfSource* pSource = NULL;
    if (FAILED(pCompartment->QueryInterface(IID_ITfSource, (void**)&pSource)))
        return FALSE;

    hr = pSource->AdviseSink(IID_ITfCompartmentEventSink,
                             (ITfCompartmentEventSink*)this,
                             &dwCompartmentSinkCookie);
    if (FAILED(hr))
        dwCompartmentSinkCookie = TF_INVALID_COOKIE;
    if (pSource)
        pSource->Release();
    return SUCCEEDED(hr) ? TRUE : FALSE;
}

void GImeNamesManagerVista::UnInitCompartmentSink()
{
    ITfSource* pSource = NULL;
    if (dwCompartmentSinkCookie != TF_INVALID_COOKIE && pCompartment)
    {
        if (SUCCEEDED(pCompartment->QueryInterface(IID_ITfSource, (void**)&pSource)) && pSource)
        {
            pSource->UnadviseSink(dwCompartmentSinkCookie);
            pSource->Release();
        }
        dwCompartmentSinkCookie = TF_INVALID_COOKIE;
    }
    if (pCompartment)
    {
        pCompartment->Release();
        pCompartment = NULL;
    }
}

}}}
