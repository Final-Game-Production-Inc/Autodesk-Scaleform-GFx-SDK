
/**************************************************************************

Filename    :   GFx_IMENamesManagerXP.h
Content     :   Overrides the functions used to obtain names of installed ime's and 
                activating ime's according to platform specific implementation.
Created     :   Oct 01, 2008
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_GFXIMENAMESMANAGERXP_H
#define INC_GFXIMENAMESMANAGERXP_H

#include "GFx_IMEIdMap.h"
#include <msctf.h>
#include "GFx_TSFUIElementDef.h"


namespace Scaleform {
	namespace GFx {
		namespace IME{

class GImeNamesManagerXP: public GImeNamesManager
{      
public:
    GImeNamesManagerXP(GFxIMEManagerWin32* pimeMgr):GImeNamesManager(pimeMgr)
    {
        SF_UNUSED(pimeMgr);
    };
  
    ~GImeNamesManagerXP() { CleanUp(); };

    virtual bool        HashIMENames(void);
    
    virtual GImeStringKey   GetKey();

    void                    CleanUp()
    {
        for (UInt32 i = 0; i < HklLayoutTextMap.GetSize(); i++)
        {
            SF_FREE((char*)HklLayoutTextMap[i].LayoutName);
            SF_FREE((char*)HklLayoutTextMap[i].ImeFileName);
        }

        for (UInt32 i = 0; i < NumLanguages; i++)
        {
            if (SupportedInputLanguages[i].ItemNameOnSystem != NULL)
                SF_FREE((char*)SupportedInputLanguages[i].ItemNameOnSystem);
        }
    }
    
#ifndef GFC_NO_LANGBAR_SUPPORT

    virtual void        ActivateIME(const char* imeName);

    void                ActivateInputLanguage(const char* inputLangName);

    virtual void        HandleStatusWindowNotifications(const char* pcommand, const char* parg);

    virtual void        OnLangBarLoaded();

#endif

	virtual bool		SwitchIME(String imeName);

private:

    void                CheckForSupportedInputLang(TF_LANGUAGEPROFILE& langProfile, const wchar_t* inputLangName);
	virtual int			CheckForSupportedIME(const char* layoutTextName, const char* imeFileName);
    virtual GFxIMETag   GetImeTag(const char* name = NULL);
};
}}}
#endif //INC_GFXIMENAMESMANAGERXP_H
