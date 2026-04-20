#ifndef INC_GFXIMEIDMAP_H
#define INC_GFXIMEIDMAP_H

/**********************************************************************

Filename    :   GFxIMEIdMap.h
Content     :   Contains declarations for classes needed for Registry Query. 
Created     :   Jun 17, 2008
Authors     :   Ankur Mohan

Notes       :   
History     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
#ifndef SF_NO_IME_SUPPORT
//#include <windows.h>
#include <stdio.h>
//#include <tchar.h>
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_Array.h"
#include "GFx/GFx_PlayerStats.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{

#define MAX_KEY_LENGTH 1024
#define MAX_VALUE_NAME 16383
#define MAX_IME_NAME    100

/*
The piece of code below is borrowed from IMEUI.cpp in DXSDK. It's used to hide some IME UI windows that 
don't follow any other method of hiding IME windows (setting lParam = 0 while processing WM_IMESETCONTEXT,
not passing ime messages to DefWndProc). The key ideas are as follows:

1) IME Windows are hidden by suppressing WM_IMENOTIFY messages when lParam = IMN_PRIVATE and wParam = some 
magic numbers that are totally undocumented.

2) Different IME's use different magic numbers

3) Different versions of the same IME uses different magic numbers

4) The version number of the current ime is obtained in GetIMEId() by first obtaining the IMEFileName
and then using API functions to get the version number. I load the version.dll to avoid having to link with
version.lib in the player

*/
#define LANG_CHT	MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS	MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)

#define MAKEIMEVERSION(major,minor) ( (DWORD)( ( (BYTE)( major ) << 24 ) | ( (BYTE)( minor ) << 16 ) ) )

#define IMEID_CHT_VER42 ( LANG_CHT | MAKEIMEVERSION( 4, 2 ) )	// New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43 ( LANG_CHT | MAKEIMEVERSION( 4, 3 ) )	// New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44 ( LANG_CHT | MAKEIMEVERSION( 4, 4 ) )	// New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50 ( LANG_CHT | MAKEIMEVERSION( 5, 0 ) )	// New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51 ( LANG_CHT | MAKEIMEVERSION( 5, 1 ) )	// New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52 ( LANG_CHT | MAKEIMEVERSION( 5, 2 ) )	// New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/WinXP)
#define IMEID_CHT_VER60 ( LANG_CHT | MAKEIMEVERSION( 6, 0 ) )	// New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // New IME 6.0(web download)
#define IMEID_CHT_VER_VISTA ( LANG_CHT | MAKEIMEVERSION( 7, 0 ) )	// All TSF TIP under Cicero UI-less mode: a hack to make GetImeId() return non-zero value



// We use both Layout Text and IME File fields in KeyBoard Layout Key of the registry
// to identify supported IME's. For some IME's the registry doesn't contain the IME File 
// field. This has to be thoroughly tested. Microsoft Pinyin IME for example leads to 
// different keyboardlayoutname on Chinese XP vs English XP. The registry key corresponding
// to the keyboardlayoutname on ChineseXP doesn't have a IME File field.

enum GFxIMETag
{
    GFxIME_English              = 0x10000000,

    GFxIME_Ch_Trad_Phonetic     = 0x00000001,
    GFxIME_Ch_Trad_NewPhonetic  = 0x00000002,
    GFxIME_Ch_Trad_ChangJie     = 0x00000003,
    GFxIME_Ch_Trad_NewChangJie  = 0x00000004,
    GFxIME_Ch_Trad_DaYi         = 0x00000005,
    GFxIME_Ch_Trad_Array        = 0x00000006,
	GFxIME_Ch_Trad_Quick        = 0x00000007,
	GFxIME_Ch_Trad_NewQuick     = 0x00000008,
    GFxIME_Ch_Trad              = 0x00000009,
	GFxIME_Ch_Trad_NewChangJie2010	= 0x0000000A,
	GFxIME_Ch_Trad_NewPhonetic2010	= 0x0000000B,
	GFxIME_Ch_Trad_NewQuick2010		= 0x0000000C,

    GFxIME_Jp_2002              = 0x00000100,
    GFxIME_Jp_2007              = 0x00000200,
    GFxIME_Jp_2003              = 0x00000300,
    GFxIME_Jp                   = 0x00000400,
	GFxIME_Jp_2010              = 0x00000500,
    GFxIME_Jp_ATOK2008          = 0x00D00800,
	GFxIME_Jp_ATOK2009          = 0x00D00900,
	GFxIME_Jp_GOOG2010			= 0x00D00A00,

    GFxIME_Kr_2000              = 0x00001000,
    GFxIME_Kr                   = 0x00002000,
	GFxIME_Kr_2003				= 0x00003000,
	GFxIME_Kr_8_6001			= 0x00004000,
	GFxIME_Kr_2007				= 0x00005000,
	GFxIME_Kr_2002				= 0x00006000,
	GFxIME_Kr_2010				= 0x00007000,

    GFxIME_Ch_Simp_QuanPin      = 0x00010000,
    GFxIME_Ch_Simp_ShuangPin    = 0x00020000,
    GFxIME_Ch_Simp_ZhengMa      = 0x00030000,
    GFxIME_Ch_Simp_MSPinyin_3_0 = 0x00040000,
    GFxIME_Ch_Simp_MSPinyin     = 0x00050000,
    GFxIME_Ch_Simp_ABC          = 0x00060000,
    GFxIME_Ch_Simp              = 0x00080000,
    GFxIME_Ch_Simp_MSPinyin_2007= 0x00070000,
	GFxIME_Ch_Simp_MSPinyin1_2010=0x00090000,
	GFxIME_Ch_Simp_MSPinyin2_2010=0x000A0000,

    GFxIME_GooglePinyin         = 0x00110000, // Both third party and simplified.
    GFxIME_SogouPinyin          = 0x00210000,
    GFxIME_Ch_Simp_Pinyin03     = 0x00410000,
    GFxIME_Ch_Simp_QQPinyin     = 0x00810000,
    GFxIME_Ch_Simp_NianQing     = 0x00910000,
    GFxIME_Ch_Simp_WuBi86       = 0x00A10000,
    GFxIME_Ch_Simp_WuBi98       = 0x00B10000,
    GFxIME_Ch_Simp_JJ           = 0x00C10000,
    // Both third party and traditional.
	GFxIME_Ch_Trad_NewChewing	= 0x00D00002,
	GFxIME_Ch_Trad_WuXia        = 0x00D00001,


    GFxIME_NotSupported         = 0x01000000,
    GFxIME_NotSet               = 0x02000000,
	GFxIME_DoesntExist			= 0x03000000 // For languages that don't have IMEs.
};

struct StringToTagDef
{
    char*   String;
    GFxIMETag  Tag;
};

struct ProfileInfo
{
	void*			profile;
	ProfileInfo*	next;
};

// This structure defines the properties of Installed Input languages and IME's.
struct InputLangProps
{
    char*			ItemNameOnSystem; // The name of the entry (input language/ime) on the system
    char*			ItemNameCommon;       // Common name of the entry     
    GFxIMETag		ItemTag;
    SPInt			Id;                 // can be HKL or Pointer to profileInfo struct.
};


struct InputLangProps2: public ListNode<InputLangProps2>
{
	InputLangProps2():ItemNameOnSystem(), ItemNameCommon(), ItemTag(GFxIME_NotSet), Id(0){};
	String			ItemNameOnSystem; // The name of the entry (input language/ime) on the system
	String			ItemNameCommon;       // Common name of the entry     
	GFxIMETag		ItemTag;
	SPInt			Id;                 // can be HKL or Pointer to profileInfo struct.
};

// This structure defines the properties of Installed Input languages and IME's.
struct HKlLayoutName
{
    char*   ImeFileName;
    char*   LayoutName; 
    UInt32    hkl;
    UInt32    IsInstalled;
};

// This structure is used to store a mapping between locale/lang id's and input language names.
struct InputLangHKLMap
{
    //    SupportedIMETags(String& data, GFxIMETag imeTag): SupportedIME(data), IMETag(imeTag){};
    GFxIMETag   IMETag;
    size_t      LangId; // Initially set to zero
};

/*
static InputLangHKLMap SupportedInputLanguages[] = 
{
    {GFxIME_English             ,0x00000409 },  // English US
    {GFxIME_English             ,0x00000809 },  // English UK
    {GFxIME_English             ,0x00000c09 },  // English Aus
    {GFxIME_English             ,0x00001009 },  // English Canada
    {GFxIME_Kr                  ,0x00000412 },  // Korean
    {GFxIME_Ch_Trad             ,0x00000404 },  // Chinese Traditional
    {GFxIME_Ch_Simp             ,0x00000804 },  // Chinese Simplified
    {GFxIME_Jp                  ,0x00000411 },  // Japanese
    {GFxIME_NotSupported        ,0x00000419 },  // Russian
    {GFxIME_NotSupported        ,0x00000000 }   // Unknown
};

*/
// String node - stored in the manager table.

struct GImeStringNode;
class GImeNamesManager;
class GFxIMEWin32Impl;

enum GFxIMETagFlags
{
    GFxIME_En_Flag              = 0x10000000,
    GFxIME_Ch_Trad_Flag         = 0x0000000F,
    GFxIME_Jp_Flag              = 0x00000F00,
    GFxIME_Kr_Flag              = 0x0000F000,
    GFxIME_Ch_Simp_Flag         = 0x000F0000,
    GFxIME_Ch_ThirdParty_Flag   = 0x00F00000,
    GFxIME_NotSupported_Flag    = 0x0F000000
};

struct GImeStringKey
{
    String           str;
    size_t              HashValue;
    size_t              Length;

    GImeStringKey(){};

    GImeStringKey(const GImeStringKey &src)
        : str(src.str), HashValue(src.HashValue), Length(src.Length)
    { }
    GImeStringKey(char* _pstr, size_t hashValue, size_t length)
        : str(_pstr), HashValue(hashValue), Length(length)
    { }
};

// Static hash function calculation routine
UInt32  IMEHashFunction(const char *pchar, size_t length);

template<class C>
class GImeStringNodeHashFunc
{
public:
    typedef C value_type;

    // Hash code is stored right in the node
    size_t  operator() (const C& data) const
    {
        return data->HashFlags;
    }

    size_t  operator() (const GImeStringKey &str) const
    {
        return str.HashValue;
    }

    // Hash update - unused.
    static size_t  get_cached_hash(const C& data)               { return data->HashFlags; }
    static void    set_cached_hash(C& data, size_t hashValue)   { SF_UNUSED2(data, hashValue); }
    // Value access.
    static C&      get_value(C& data)                           { return data; }
    static const C& get_value(const C& data)                    { return data; }

};

// String node hash set - keeps track of all strings currently existing in the manager.
typedef HashSetUncachedLH<GImeStringNode*, GImeStringNodeHashFunc<GImeStringNode*>,
GImeStringNodeHashFunc<GImeStringNode*>, StatIME_Mem > GImeStringNodeHash;

struct GImeStringNode: public NewOverrideBase<StatIME_Mem>
{        
    GImeStringNode():HashFlags(0), Size(0), pNextAlloc(0), pImeNamesManager(0)
    {
        data.ItemTag            = GFxIME_NotSupported;
        data.ItemNameOnSystem   = NULL;
    };

    InputLangProps          data;
    UInt32                  HashFlags;
    UInt32                    Size;
    GImeStringNode*         pNextAlloc;
    GImeNamesManager*       pImeNamesManager;

    inline UInt32  GetHashCode()
    {
        return HashFlags & 0x00FFFFFF;
    };

    void Release();

};

// Must define an operator that can compare GFxIMEKey to GImeStringNode:

bool operator == (GImeStringNode* pnode, const GImeStringKey &key);

class GFxIMEManagerWin32;

class GImeNamesManager: public NewOverrideBase<StatIME_Mem>
{      
public:

    GImeNamesManager(GFxIMEManagerWin32* pimeManager); 

    virtual ~GImeNamesManager()
    {
        CleanUp();
    }
    // We try to distinguish the functions used for querying IME names from the registry against those used to support the 
    // language bar/stauts window functionality. This is so that the language bar functions can be easily ifdef'd out.

    virtual bool            HashIMENames(void);
    void                    CleanUp();
    UInt32                    GetNumSupportedIMEs() {return NumIme;};
    virtual GFxIMETag       GetImeTag(const char* name = NULL);
    virtual GFxIMETag       GetInputLangTagfromImeTag(GFxIMETag imeTag);
    virtual GFxIMETag       GetLangNamefromLangId(UInt32 langId);
    virtual GImeStringKey   GetKey() {return GImeStringKey();};
    virtual bool            MakeKeyboardLayoutListFromRegistry(HKL*, UInt32);

    // Called when WM_INPUTLANGCHANGE is received. This can happen if the user clicks on our language bar
    // or the windows language bar. 
    virtual void            OnInputLangChange(DWORD langId);

    // When we switch to a new input language, a default IME is automatically selected by the system.
    // this function sets the text in the IMEName field of the language bar to this default IME.
    // It also displays the status window if it needs to be displayed.
    void                    SetLastIMEName(String imeName);

    void                    HideReadingWindow();

    int                     GetImeId();

#ifndef GFC_NO_LANGBAR_SUPPORT

    virtual void            ActivateIME(const char* imeName) { SF_UNUSED(imeName); };
    // Instructs the system to switch to the requested input language
    virtual void            ActivateInputLanguage(const char* inputLangName) { SF_UNUSED(inputLangName); };
    
    // Set's the conversion mode
    virtual void            SetConversionMode(UInt32 conversionParams = -1);
    // Handles user input to the status window. Instructs the system to change the conversion mode
    // of the IME under use accordingly.
    virtual void            HandleStatusWindowNotifications(const char* pcommand, const char* parg) 
                            {SF_UNUSED2(pcommand, parg);} ;
    
    // Called when "LangBar_OnInit" is received from the Langbar movie 
    virtual void            OnLangBarLoaded();

	int						CheckForSupportedIME(const char* layoutTextName, const char* imeFileName);

	virtual bool			SwitchIME(String imeName) {return false;};
	
	/* This function is the fallback option for filling in supported IME list in case the user doesn't specify
	the xml file for the supported IME's or there is some problem opening the xml file */
	void PopulateSupportedIMEs();
	void AllocateImeNameOrLayoutTextArray();

#endif

    virtual void            LogError(DWORD err, char*);
	virtual void			LogError(const char* error, const char* context);

    friend struct GImeStringNode;
    GImeStringNodeHash      StringSet;
    Array<String>			JapaneseIMEs;
    Array<String>			KoreanIMEs;
    Array<String>			ChineseSimpIMEs;
    Array<String>			ChineseTradIMEs;
    String					InstalledInputLangs; 
    GFxIMEManagerWin32*		pImeMgr;
    UInt32                  NumLanguages;
    GFxIMETag               CurrentInputLangTag;
    GFxIMETag               CurrentIMETag;
    Array<InputLangProps>	SupportedInputLanguages;
    // The description "below" is not applicable. It's not used. We use the technique of associating a null 
    // context to disable IME. If we do this, the previous IME state is automatically maintained and no book keeping is 
    // needed.
    // Below:
    // This variable is used to record if the japanese IME is in direct input mode or not. The problem is
    // that we use the ImmSetOpenStatus function to turn IME on/off when the user clicks from a text field 
    // to a non-text element such as a button. We use the same function to also switch between direct input mode
    // and other IME enables modes (Hiragana, Katakana etc) for the Japanese IME. So consider the following scenario:
    // User is in a text field and switches to DirectInput mode. Now the user clicks on the background or a button. This
    // will cause IMEEnabled(false) to be called which will in turn do a ImmSetOpenStatus(false). Now if the user clicks
    // back to the textfield, IMEEnabled(true) is called which in turn calls ImmSetOpenStatus(true) which not only activates
    // the IME, it also sets the input mode to Hiragana. This is bad since the user had earlier set it to DirectInput and
    // expects the input mode to be the same. To alleviate this problem, we'll use an extra variable that will record
    // the state of the Japanese input mode.
    bool                    JapaneseIMEState;
	bool	mark;
	bool	mark_ime;
	bool	mark_displayname;
	bool	mark_tag;
	enum labels
	{
		IMENAME,
		DISPLAYNAME,
		IMETAG,
		NONE
	} label;
	Array<InputLangProps2>	SupportedIMEs;
	UInt32                  NumIme;
protected:

    UInt32                  fdwConversion;
    GImeStringNode          EmptyStringNode;
    GImeStringNode          *Current;
    GImeStringNode          Root;
    HKEY                    hKey;
    Array<HKlLayoutName>	HklLayoutTextMap;
    int                     ImeVersionId;
	int*					ImeFileNameOrLayoutText;
    union HKL_Size_t_Union
    {
        HKL     hkl;
        size_t  val;
    } Un;
};

}}}
#endif //INC_GFXIMEIDMAP_H
#endif //#ifdef SF_NO_IME_SUPPORT
