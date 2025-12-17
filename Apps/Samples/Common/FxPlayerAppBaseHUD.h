/**********************************************************************

PublicHeader:   Render
Filename    :   FxPlayerAppBaseHUD.h
Content     :   Builds on the base player application functionality, by
                adding HUD, and associated functions.
Created     :   2013/02/01
Authors     :   Bart Muzzin

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_FxPlayerAppBaseHUD_H
#define INC_SF_FxPlayerAppBaseHUD_H

#include "FxPlayerAppBase.h"
#include "../FxPlayer/FxHUD.h"
#include "Platform/Platform.h"

namespace Scaleform {

class FxPlayerAppBaseHUD : public FxPlayerAppBase, public FxHUDInfoProvider
{
public:
    FxPlayerAppBaseHUD();

    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            OnShutdown();
    virtual bool            LoadMovie(const String& filename);

    // Adds the 'NoHUD' parameter (-nh).
    virtual void            InitArgDescriptions(Platform::Args* args);

    // Used by HUD to display statistics
    virtual void            GetExtraInfo(FxHUDExtraInfo* pinfo);
    virtual void            GetHelpStr(String* phelpStr);
    // Invoked when HUD becomes active/inactive
    virtual void            OnHudStatusChange(bool active) { SF_UNUSED(active); }   

    virtual void            UpdateStatReports();
    virtual void            UpdateFpsDisplay();
    virtual void            UpdateViewSize();
    virtual void            UpdateHUDViewport();
    virtual FxPlayerCommand* PadKeyToCommand(PadKeyCode key);

    bool                    RedirectEventsToHud;
    Ptr<FxHUDDataProvider>  pHudDataProvider;
    Ptr<FxHUD>              pHud;
    MemoryHeap*             pHudHeap;

    Hash<UInt32, Ptr<FxPlayerCommand> >     HudKeyCommandMap;
    Hash<PadKeyCode, Ptr<FxPlayerCommand> > HudPadKeyCommandMap;

};

}; // Scaleform

#endif // INC_SF_FxPlayerAppBaseHUD_H
