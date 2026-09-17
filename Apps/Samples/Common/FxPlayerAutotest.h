/**********************************************************************

PublicHeader:   Render
Filename    :   FxPlayerAutotest.h
Content     :   
Created     :   2012/07/11
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_FxPlayerAutotest_H
#define INC_SF_FxPlayerAutotest_H

#include "GFxConfig.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_KeyCodes.h"
#include "GFx/GFx_Types.h"

#include "Platform/Platform.h"

#ifdef GFX_ENABLE_XML
#include "GFx/XML/XML_DOM.h"
#endif

class FxPlayerAppBase;
class FxPlayerCommand;

namespace Scaleform {

class ScopedAutoTestTimerReplacement
{
public:
    ScopedAutoTestTimerReplacement(FxPlayerAppBase* app);
    ~ScopedAutoTestTimerReplacement();
private:
    bool ReplacedTimer;
    FxPlayerAppBase* App;
};

class FxPlayerAutoTest : public NewOverrideBase<Stat_Debug_Mem>
{
public:
    FxPlayerAutoTest(FxPlayerAppBase* app, bool playback, const char* autoFile, const char* autoOutDir );
    ~FxPlayerAutoTest();

    bool OnInit(Platform::ViewConfig& config);
    void AddCommand(FxPlayerCommand* cmd);
    void PerformPlayback(bool untilAdvanceOnly = false);

    // App events, which require recording
    void OnAdvanceMovie(float delta);
    void OnKeyPress(unsigned controllerIndex, Key::Code keyCode, unsigned wcharCode, bool downFlag, KeyModifiers mods);
    void OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, const Render::PointF& mousePos, KeyModifiers mods);
    void OnMouseWheel(unsigned mouseIndex, float zdelta, const Render::PointF& mousePos, KeyModifiers mods);
    void OnMouseMove(unsigned mouseIndex, const Render::PointF& mousePos, KeyModifiers mods);
    void OnFocus(bool set, KeyModifiers mods);

    // General accessors.
    bool IsPlayback() const { return Playback; }
#ifdef GFX_ENABLE_XML
    GFx::XML::ObjectManager*        GetObjManager() const { return AutoObjManager; }
    GFx::XML::Document*             GetInstructions() const { return AutoInstructions; }
#endif
    const StringHash< Ptr<FxPlayerCommand> >& GetAutoCommandMap() const { return AutoCommandMap; }
    const StringHash< Key::Code >&            GetKeyCodeMap() const { return KeyCodeMap; }
    const StringHash< unsigned >&             GetKeyModifierMap() const { return KeyModifierMap; }

    void GetScreenshotFilename(String& filename, unsigned index) const;

protected:
    FxPlayerAppBase*                App;                    // Pointer to the application.
    bool                            Playback;               // True if playing back a test, false if recording.
    String                          AutoFilename;           // Filename which is being recorded or played back.
    String                          AutoOutputDir;          // The directory for automated output to go into (playback only).

    StringHash< Ptr<FxPlayerCommand> > AutoCommandMap;      // String map, mapping autotest instructions to names. General convention for the class name is FxPlayerCommand<String>.
    StringHash< Key::Code >            KeyCodeMap;          // String mapping keycode values to strings (used in autotest keypress events)
    StringHash< unsigned >             KeyModifierMap;      // String mapping modifier values to modifier flags (used in autotest events)

#ifdef GFX_ENABLE_XML
    Ptr<GFx::XML::ObjectManager>    AutoObjManager;         // XML ObjectManager for allocation of node, etc.
    Ptr<GFx::XML::Document>         AutoInstructions;       // Pointer to the rootNode of the XML instructions for testing (playback & record).
    Ptr<GFx::XML::Node>             NextAutoInstruction;    // Pointer to the next instruction (playback only).
#endif
};

}; // Scaleform

#endif // INC_SF_FxPlayerAutotest_H
