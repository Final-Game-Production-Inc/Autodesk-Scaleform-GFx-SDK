/**************************************************************************

Filename    :   FxPlayerAppBase.cpp
Content     :   FxPlayerAppBase and supporting code implementation.

Created     :   November, 2010
Authors     :   Michael Antonov, Dmitry Polenur, Maxim Didenko, 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerAppBase.h"
#include "GFx/AMP/Amp_Server.h"
//#include "Render/Render_Amp.h"
#include "FxPlayerLog.h"
#include "FxPlayerAutotest.h"
#include "GFx/AMP/Amp_Message.h"

#ifdef GFX_GESTURE_RECOGNIZE
    #include "GFx/GFx_Gesture.h"
#endif

#ifdef SF_OS_WIN32
    #include "GFx_FontProvider_Win32.h"
    //#include "FileFindWin32.h"

    #ifdef __cplusplus_cli
    #using <mscorlib.dll>
    using namespace System;
    #endif
#endif

#ifdef GFX_ENABLE_XML
    #include "GFx_XML.h"
    #include "GFx/XML/XML_DOM.h"
#endif

#include "GFx/IME/GFx_IMEManager.h"

#if defined(SF_ENABLE_IME) && defined(SF_ENABLE_IME_WIN32)
#include "GFxIME/GFx_IMEManagerWin32.h"
#endif

#ifdef SF_ENABLE_THREADS
#include "GFx/GFx_TaskManager.h"    
#endif

#ifdef GFX_AS2_ENABLE_SHAREDOBJECT
#include "FxSharedObjectManager.h"
#endif  // GFX_AS2_ENABLE_SHAREDOBJECT

#ifdef GFX_ENABLE_SOUND
#include "Sound/Sound_SoundRenderer.h"
#include "Sound/Sound_SoundEvent.h"
#include "Sound/Sound_SoundEventFMOD.h"
#include "Sound/Sound_SoundEventWwise.h"
#include "GFx/GFx_Audio.h"
#include "FxSoundFMOD.h"
#include "FxSoundWwise.h"
#endif

#include "GFx/GFx_ImageCreator.h"

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"
#include "Render/ImageFiles/SIF_ImageFile.h"
//#include "Render/ImageFiles/KTX_ImageFile.h"
#if defined (SF_OS_WIIU)
#include "Render/ImageFiles/GTX_ImageFile.h"
#endif
#if defined (SF_OS_PSVITA)
#include "Render/ImageFiles/GXT_ImageFile.h"
#endif
#include "Render/Render_GlyphCacheConfig.h"

#ifdef GFX_ENABLE_VIDEO
// Core video support
#include "Video/Video_Video.h"
#if   defined(SF_OS_WIN32) && !defined(_DURANGO)
#include "Video/Video_VideoPC.h"
#elif defined(SF_OS_XBOX360)
#include "Video/Video_VideoXbox360.h"
#elif defined(SF_OS_PS3)
//#define CRIMV_SPU_TRHEADS
#include "Video/Video_VideoPS3.h"
#elif defined(_DURANGO)
#include "Video/Video_VideoXboxOne.h"
#elif defined(SF_OS_ORBIS)
#include "Video/Video_VideoPS4.h"
#elif defined(SF_OS_WIIU)
#include "Video/Video_VideoWiiU.h"
#endif
// Video system sound interfaces
#ifdef GFX_VIDEO_USE_SYSTEM_SOUND
#if   defined(SF_OS_WIN32) && !defined(_DURANGO)
#include "Video/Video_VideoSoundSystemDX8.h"
#elif defined(SF_OS_MAC)
#include "Video/Video_VideoSoundSystemMac.h"
#elif defined(SF_OS_XBOX360) || defined(_DURANGO)
#include "Video/Video_VideoSoundSystemXA2.h"
#elif defined(SF_OS_PS3)
#include "Video/Video_VideoSoundSystemPS3.h"
#elif defined(SF_OS_WII)
#include "Video/Video_VideoSoundSystemWii.h"
#elif defined(SF_OS_WIIU)
#include "Video/Video_VideoSoundSystemWiiU.h"
#endif
// FMOD and Wwise system sound interfaces
#elif defined(GFX_VIDEO_USE_FMOD)
#include "Video/Video_VideoSoundSystemFMOD.h"
#elif defined(GFX_VIDEO_USE_WWISE)
#include "Video/Video_VideoSoundSystemWwise.h"
#endif
#endif // GFX_ENABLE_VIDEO

//#include "GFx_FontProvider_NGP.h"
//#include "GFx_FontProvider_FT2.h"

#if 0
//  Sample translator implementation that can be used for testing.
class TranslatorImpl : public GFx::Translator
{
public:
    // wwMode can be GFx::Translator::WWT_Korean,
    // GFx::Translator::WWT_Japanese or
    // GFx::Translator::WWT_Chinese. Also GFx::Translator::WWT_Hyphenation
    // might be used for very simple hyphenation. OnWordWrapping virtual 
    // method might be overridden to implement custom word-wrapping rules
    // (WWT_Custom should be used for wwMode then).
    TranslatorImpl(unsigned wwMode = GFx::Translator::WWT_Default) : GFx::Translator(wwMode) 
    {}

    virtual unsigned GetCaps() const         
    { 
        return Cap_StripTrailingNewLines | Cap_BidirectionalText; 
    }
    virtual void     Translate(TranslateInfo* ptranslateInfo)
    {
        // Translated strings start with $$sign
        const wchar_t* pkey = ptranslateInfo->GetKey();
        if (pkey[0] == '$')
        {
            WStringBuffer buffer;
            size_t l = SFwcslen(pkey);
            buffer.Resize(l + 100);
            wchar_t* pb = buffer.GetBuffer();

            //SFwcscpy(pb, buffer.GetLength(), pkey+1);
            //ptranslateInfo->SetResult(buffer.ToWStr());

            SFwcscpy(pb, buffer.GetLength(), L"<font color='#00FF33'>*TR[<font color='#FF3300'>");
            SFwcscat(pb, buffer.GetLength(), pkey+1);
            SFwcscat(pb, buffer.GetLength(), L"</font>]</font>");
            ptranslateInfo->SetResultHtml(buffer.ToWStr());
        }
        // example of using GetInstanceName()
        else if (SFstrcmp(ptranslateInfo->GetInstanceName(), "plainText") == 0)
        {
            ptranslateInfo->SetResult("PLAIN");
        }
    }

    // an example of custom word wrapping
    bool OnWordWrapping(LineFormatDesc* pdesc)
    {
        // check if we deal with Asian languages; if so, use
        // the default implementation.
        if (WWMode != WWT_Custom)
            return Translator::OnWordWrapping(pdesc);
        if (pdesc->NumCharsInLine == 0)
            return false; // skip empty lines
        // implementing word wrapping at spaces, dots, commas and dashes.
        // start from NumCharsInLine.
        // search for desired word breaker backward
        UPInt linePos = pdesc->NumCharsInLine - 1;
        UPInt textPos = pdesc->LineStartPos + linePos;
        for (SPInt tpos = SPInt(textPos), lpos = SPInt(linePos); 
             tpos >= 0 && lpos >= 0; 
             --tpos, --lpos)
        {
            int isSpace = SFiswspace(pdesc->pParaText[tpos]) ? 1 : 0;
            if (isSpace ||
                pdesc->pParaText[tpos] == '.' ||
                pdesc->pParaText[tpos] == ',' ||
                pdesc->pParaText[tpos] == '-')
            {
                // found our spacer;
                // check if width fits our textfield.
                // we will want to exclude the whitespace from the
                // widths, but include other spacers (so, they fit
                // the line).
                if (pdesc->pWidths[lpos + (1 - isSpace)] <= pdesc->VisibleRectWidth)
                {
                    // it fits, modify the ProposedWordWrapPoint and return true;
                    // otherwise, continue.
                    pdesc->ProposedWordWrapPoint = lpos + (1 - isSpace);
                    return true;
                }
            }
        }
        // nothing was found, return false.
        return false;
    }
};
#endif

//------------------------------------------------------------------------
/** 
A sample sound event callback processor.  This class can be used as a function object
to override _global.gfxProcessSound for sound playback. Overridng the _global.gfxProcessSound
method will, by default, cause most CLIK Components to send sound events to this class. It can
also be used to trigger sound playback from ActionScript using _global.gfxProcessSound().

Note that the implementation of this function is at the discretion of the developer.
*/

#if defined(GFX_ENABLE_SOUND)
class GFxSoundEventCallback : public FunctionHandler
{
public:
    GFxSoundEventCallback (const Movie* pmovie) : pMovie(pmovie)
    {   
        pApp = (FxPlayerAppBase*)pMovie->GetUserData();  

#if defined(GFX_SOUND_FMOD) && !defined(GFX_SOUND_WWISE)
        if (pApp->pSoundFMODSystem)
        {   // FMOD Ex (implementation based on GSoundRender)
            pSoundEvent = *new SoundEventFMOD;    
            pSoundEvent->pUserData = pApp->pSoundFMODSystem->GetSoundRenderer();
            // FMOD Designer
            //pSoundEvent = *new SoundEventFMODDesigner("examples.fev", "examples/FeatureDemonstration");
            //pSoundEvent->pUserData = pApp->pSoundFMODSystem->pSoundRenderer->GetFMODEventSystem();
        }
#elif defined(GFX_SOUND_WWISE)
        // AK Wwise SoundFrame
        pSoundEvent = *new SoundEventWwiseSF; 
#endif
        bCLIKSound = pApp->bCLIKSound; // Is CLIK soundEvent processing enabled ("-cksd" command line parameter)
        pLog = pMovie->GetLog();   // Used for logging debug messages
    }

    /** Called when _global.gfxProcessSound() is called in ActionScript */
    void Call(const Params& params)
    {
        bool bCallProcessed = ProcessSoundEvent(params);
        if (!bCallProcessed && bCLIKSound) 
        {
            if (params.ArgCount >= 1)
            {
                if (params.pArgs[0].IsDisplayObject() && params.ArgCount == 3)
                    ProcessCLIKSoundEvent(params);
            }            
        } 
    }

    /** FMOD-based sample for standard sound processing in FxPlayer. */     
    bool ProcessSoundEvent(const Params& params)
    {
        if (params.pArgs[0].GetType() != Value::VT_String)
            return false;

        const String methodName = params.pArgs[0].GetString();        

        // Usage:   _global.gfxProcessSound(Method:String, Command:String, Filename:String)
        // Example: _global.gfxProcessSound("GFxSound_PostEvent", "Play", "electric.wav");
        if (SFstricmp(methodName, "GFxSound_PostEvent") == 0)
        {
            if (params.ArgCount >= 2 && params.pArgs[1].GetType() == Value::VT_String)
            {
                const String command = params.pArgs[1].GetString();
                String filename;
                if (params.ArgCount >= 3 && params.pArgs[2].GetType() == Value::VT_String)
                    filename = params.pArgs[2].GetString();  

                // This log message can be commented or uncommented at the user's discretion.
                pLog->LogMessage("gfxProcessSound Callback - \n"
                    "\tCommand: %s\n"
                    "\tFilename: %s\n\n", 
                    command.ToCStr(), filename.ToCStr());

                pSoundEvent->PostEvent(command, filename);
                return true;
            } 
        }
        // Usage:   _global.gfxProcessSound(Method:String, Parameter:String, Value:Number, Filename:String)
        // Example: _global.gfxProcessSound("GFxSound_SetParam", "Pan", -1.0, "electric.wav");
        else if (SFstricmp(methodName, "GFxSound_SetParam") == 0)
        {
            if (params.ArgCount >= 2 &&
                params.pArgs[1].GetType() == Value::VT_String &&
                params.pArgs[2].GetType() == Value::VT_Number)
            {
                const String parameter = params.pArgs[1].GetString();
                const GFx::Double paramvalue = params.pArgs[2].GetNumber();
                String filename;
                if (params.ArgCount >= 2 && params.pArgs[3].GetType() == Value::VT_String)
                    filename = params.pArgs[3].GetString();  

                // This log message can be commented or uncommented at the user's discretion.
                pLog->LogMessage("gfxProcessSound Callback -\n"
                    "\tParameter: %s\n" 
                    "\tParameter Value: %f\n"
                    "\tFilename: %s\n\n",
                    parameter.ToCStr(), static_cast<double>(paramvalue), filename.ToCStr());

                pSoundEvent->SetParam(parameter, (float)paramvalue, filename);
                return true;
            }
        }

        // If we're here, the call could not be processed.
        return false;
    }

    /** 
    Sample internal sound processing for CLIK components. CLIK Component events are identified by their
    first parameter, a Value reference to themselves from which extra data can be retrieved. This event is
    fired by all CLIK components who inherit from UIComponent (see UIComponent::dispatchEventAndSound()).

    This method is disabled by default. It can be enabled using the -cksd command line option.

    By default, this method will search for SoundTheme+SoundName.wav (eg. defaultRollOver.wav). It is designed
    to hook into to a more advanced engine sound tool where a theme and name can easily be mapped to a particular
    sound file.

    Usage: _global.gfxProcessSound(CLIK Component:MovieClip, SoundTheme:String, SoundName:String);
    */
    bool ProcessCLIKSoundEvent(const Params& params)
    {
        // Retrieve arguments from the function parameters
        const String componentName = params.pArgs[0].ToString();
        const String soundThemeName = params.pArgs[1].GetString();
        const String soundEventName = params.pArgs[2].GetString(); 
        String soundName = String(soundThemeName.ToCStr(), soundEventName.ToCStr(), ".wav");

        // This log message can be commented or uncommented at the user's discretion.
        if (pLog)
            pLog->LogMessage("gfxProcessSound Callback - \n"
            "\tComponent: %s\n"
            "\tSound Theme: %s\n"
            "\tSoundEvent: %s\n"
            "\tAttempting to play file: %s\n\n",
            componentName.ToCStr(), soundThemeName.ToCStr(), 
            soundEventName.ToCStr(), soundName.ToCStr());     

        if (pSoundEvent)
        {
            // Play the sound with the default parameters.
            pSoundEvent->SetParam("Volume", 0.4f, soundName);                   
            pSoundEvent->SetParam("Pan", 0.f, soundName);
            pSoundEvent->PostEvent("Play", soundName);
        }
        return true;
    }

private:
    const Movie* pMovie;
    FxPlayerAppBase* pApp;
    Ptr<GFx::Log> pLog;

    bool bCLIKSound;
    Ptr<SoundEvent> pSoundEvent;
};
#endif // GFX_ENABLE_SOUND

class FxPlayerCommandChangeRes : public FxPlayerCommand
{
    static const unsigned ResList[5][2];
    static Size<unsigned> GetRes(unsigned i) { return Size<unsigned>(ResList[i][0], ResList[i][1]); }
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->ResIndex = (app->ResIndex + 1) % 5;
        Size<unsigned> newSize = GetRes(app->ResIndex);
        if (newSize == app->GetViewSize())
        {
            app->ResIndex = (app->ResIndex + 1) % 5;
            newSize = GetRes(app->ResIndex);
        }
        app->OnSize(newSize);
    }
    virtual String GetHelpString() const
    {
        return "Toggle display resolution";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(ChangeRes);
};
const unsigned FxPlayerCommandChangeRes::ResList[5][2] = {{640,480}, {1024, 768}, {1280, 720}, {1280, 1024}, {1920, 1080}};

class FxPlayerCommandScaledDisplay : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->ScaleEnable = !app->ScaleEnable;
        app->UpdateViewSize();
    }
    virtual String GetHelpString() const
    {
        return "Toggle scaled display";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(ScaledDisplay);
};

class FxPlayerCommandWireframe : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->Wireframe = !app->Wireframe;
        app->GetRenderThread()->SetWireframe(app->Wireframe);
        app->GetRenderThread()->DrawFrame();
        app->AmpDirty = true;

    }
    virtual String GetHelpString() const
    {
        return "Toggle wire frame";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Wireframe);
};

class FxPlayerCommandAntialiasingMode : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->ToggleAaMode();
    }
    virtual String GetHelpString() const
    {
        return "Toggle anti-aliasing mode";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(AntialiasingMode);
};

class FxPlayerCommandFullscreen : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);

        Platform::ViewConfig config;
        app->GetViewConfig(&config);        
        // Use default position for mode switching.
        config.ClearSize();
        config.ClearPos();
        config.SetFlag(Platform::View_FullScreen, !config.HasFlag(Platform::View_FullScreen));
        
        app->ReconfigureGraphics(config);
        app->UpdateViewSize();
        app->SetWindowTitle(app->GetAppTitle());
    }
    virtual String GetHelpString() const
    {
        return "Toggle full-screen";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Fullscreen);
};

class FxPlayerCommandFastMode : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->MeasurePerformance = !app->MeasurePerformance;
        //app->pRenderStats->GetTessStatistics(); // Clear stats
        app->AdjustFrameTime();
        app->LastFPS = 0;
        if (!app->MeasurePerformance)
        {
            app->SetWindowTitle(app->GetAppTitle());
        }
    }
    virtual String GetHelpString() const
    {
        return "Toggle fast mode (FPS)";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(FastMode);
};

class FxPlayerCommandFastForward : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->FastForward = !app->FastForward;
        app->AdjustFrameTime();
        app->AmpDirty = true;    
    }
    virtual String GetHelpString() const
    {
        return "Toggle fast forward";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(FastForward);
};

class FxPlayerPadPassThrough : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->PadPassthrough = !app->PadPassthrough;
    }
    virtual String GetHelpString() const
    {
        return "Toggle Pad Passthrough mode";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(PadPassThrough);
};


class FxPlayerCommandVSync : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        // MA TBD:

        Platform::ViewConfig config;
        app->GetViewConfig(&config);
        config.SetFlag(Platform::View_VSync, !config.HasFlag(Platform::View_VSync));
        config.ViewFlags &= ~Platform::View_Pos;

        app->ReconfigureGraphics(config);
        app->AmpDirty = true;
    }
    virtual String GetHelpString() const
    {
        return "Toggle VSync";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(VSync);
};

class FxPlayerCommandPause : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->Paused = !app->Paused;
        // Note that toggling play state through pMovie->SetPlayState would only pause root clip,
        // not children. So instead we just set the global variable above which prevents Advance.
        // However, we may still need to save/restore state in frame.
        app->OnPause(app->Paused);
        app->AdjustFrameTime();
        app->AmpDirty = true;
    }
    virtual String GetHelpString() const
    {
        return "Toggle pause";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Pause);
};

class FxPlayerCommandObjectsReport : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (app->pMovie)
        {
            app->OnObjectsReport();
        }
    }
    virtual String GetHelpString() const
    {
        return "Print objects report";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(ObjectsReport);
};

class FxPlayerCommandFontConfig : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->CycleFontConfig();
    }
    virtual String GetHelpString() const
    {
        return "Next font config";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(FontConfig);
};

class FxPlayerCommandRestart : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (!app->pMovie) return;

        app->SafeMovieRestart();

        app->AmpDirty = true;
    }
    virtual String GetHelpString() const
    {
        return "Restart the movie";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Restart);
};

class FxPlayerCommandStageClipping : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->ClippingEnable = !app->ClippingEnable;
        app->UpdateViewSize();
    }
    virtual String GetHelpString() const
    {
        return "Toggle stage clipping";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(StageClipping);
};

class FxPlayerCommandStageCulling : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (app->pMovie)
        {
//@TODO
//             UInt32 rendererFlags = app->pRenderConfig->GetRenderFlags();
//             app->pRenderConfig->SetRenderFlags(rendererFlags ^ RenderConfig::RF_NoViewCull);
        }
    }
    virtual String GetHelpString() const
    {
        return "Toggle stage culling";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(StageCulling);
};

class FxPlayerCommandResetUserMatrix : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->ResetUserMatrix();
    }
    virtual String GetHelpString() const
    {
        return "Reset Zoom";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(ResetUserMatrix);
};

class FxPlayerCommandTriangleOptimization : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (app->pMovie)
        {
            //@TODO

//             UInt32 rendererFlags = app->pRenderConfig->GetRenderFlags();
//             app->pRenderConfig->SetRenderFlags(rendererFlags ^ RenderConfig::RF_OptimizeTriangles);
        }
    }
    virtual String GetHelpString() const
    {
        return "Toggle triangle optimization";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(TriangleOptimization);
};

class FxPlayerCommandStrokeMode : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->ToggleStrokeType();
    }
    virtual String GetHelpString() const
    {
        return "Switch stroke type";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(StrokeMode);
};

template< Render::ProfilerModes ProfileMode >
class FxPlayerCommandToggleProfileMode : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (app->ProfileMode != ProfileMode)
        {
            app->ProfileMode = ProfileMode;
        }
        else
        {
            app->ProfileMode = Render::Profile_None;
        }
        app->GetRenderThread()->SetProfileMode(app->ProfileMode);
        app->AmpDirty = true;
    }
    virtual String GetHelpString() const
    {
        static const char* ProfileModeHelpStrings[Render::Profile_Count] =
        {
            "No Profiling (shouldn't be a command!)",
            "Overdraw Profile",
            "Batch Mode Profile",
            "Blending Profile",
            "Texture density Profile"
        };
        return ProfileModeHelpStrings[ProfileMode];
    }
    virtual const char* GetCommandName() const
    {
        static const char* ProfileModeCommandName[Render::Profile_Count] =
        {
            "ThisShouldNotBeACommand",
            "ToggleOverdrawProfileMode",
            "ToggleBatchProfileMode",
            "ToggleBlendingProfileMode"
            "ToggleTextureDensityProfileMode"
        };
        return ProfileModeCommandName[ProfileMode];
    }
};

template< int IndexChange >
class FxPlayerCommandChangeHighlightBatch : public FxPlayerCommand
{
public:
    FxPlayerCommandChangeHighlightBatch() : FxPlayerCommand()
    {
        Format(HelpString, "Changes the highlighted batch index by {0}", IndexChange);
        Format(CommandName, "ChangeHighlightBatch{0}", IndexChange);
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->BatchHighlight += IndexChange;
        if (app->BatchHighlight < 0)
            app->BatchHighlight = -1;
        app->GetRenderThread()->SetBatchHighlight(app->BatchHighlight);
        app->AmpDirty = true;
    }    
    virtual String GetHelpString() const
    {
        return HelpString;
    }
    virtual const char* GetCommandName() const
    {
        return CommandName.ToCStr();
    }
    String HelpString;
    String CommandName;
};

class FxPlayerCommandCycleProfile : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        unsigned profileMode = app->ProfileMode;
        profileMode++;
        if (profileMode >= Render::Profile_Count)
            profileMode = 0;
        app->ProfileMode = (Render::ProfilerModes)profileMode;
        app->GetRenderThread()->SetProfileMode(app->ProfileMode);
        app->AmpDirty = true;
    }
    virtual String GetHelpString() const
    {
        return "Cycle Profile Mode";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(CycleProfile);
};

template< Render::ProfilerFlags Flag>
class FxPlayerCommandToggleProfilerFlag : public FxPlayerCommand
{
public:
    FxPlayerCommandToggleProfilerFlag() : FxPlayerCommand()
    {
        switch(Flag)
        {
        case Render::ProfileFlag_NoFilterCaching:
            HelpString  = "Toggles Filter Caching";
            CommandName = "ToggleProfileFlag_FilterCaching";
            ResetFile   = false;
            break;
        case Render::ProfileFlag_NoBatching:
            HelpString  = "Toggles Batching";
            CommandName = "ToggleProfileFlag_NoBatching";
            ResetFile   = true;
            break;
        case Render::ProfileFlag_NoInstancing:
            HelpString  = "Toggles Instancing";
            CommandName = "ToggleProfileFlag_NoInstancing";
            ResetFile   = true;
            break;
        case Render::ProfileFlag_NoBlendCaching:
            HelpString  = "Toggles Blend Caching";
            CommandName = "ToggleProfileFlag_BlendCaching";
            ResetFile   = false;
        }
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED3(app, controllerIdx, keyDown);
        bool set = app->GetRenderThread()->GetProfileFlag(Flag) ? true : false;
        app->GetRenderThread()->SetProfileFlag(Flag, !set);

        if (ResetFile)
            app->SafeMovieRestart();
    }
    virtual String GetHelpString() const
    {
        return HelpString;
    }
    virtual const char* GetCommandName() const
    {
        return CommandName.ToCStr();
    }
    String HelpString;
    String CommandName;
    bool   ResetFile;   // If true, this command resets the file when it is toggled (for batching/instancing toggles).
};

class FxPlayerCommandMemReport : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED3(app, controllerIdx, keyDown);
        StringBuffer buffer;
        Memory::pGlobalHeap->MemReport(buffer, MemoryHeap::MemReportFull);
        printf("\n%s\n", buffer.ToCStr());
    }
    virtual String GetHelpString() const
    {
        return "Print full memory report to console";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(MemReport);
};

class FxPlayerCommandFunctionTiming : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED3(app, controllerIdx, keyDown);
#ifdef SF_AMP_SERVER
        AmpServer::GetInstance().SetProfileLevel(Amp_Profile_Level_High, false);
        AmpServer::GetInstance().ToggleInternalStatRecording();
#endif
    }
    virtual String GetHelpString() const
    {
        return "Toggle function timing";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(FunctionTiming);
};

class FxPlayerCommandMute : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->IsMute = !app->IsMute;
#ifdef GFX_ENABLE_SOUND
        Sound::SoundRenderer* psoundRenderer = app->GetSoundRenderer();
        if (psoundRenderer)
        {
            psoundRenderer->Mute(app->IsMute);    
        }
#endif
    }
    virtual String GetHelpString() const
    {
        return "Toggle Mute";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Mute);
};

class FxPlayerCommandVolumeUp : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
#ifdef GFX_ENABLE_SOUND
        Sound::SoundRenderer* psoundRenderer = app->GetSoundRenderer();
        if (psoundRenderer)
        {
            if (app->IsMute)
            {
                psoundRenderer->Mute(false);
                app->IsMute = false;
            } 
            else
            {
                app->SoundVolume += 10;
                if (app->SoundVolume > 100)
                {
                    app->SoundVolume = 100;
                }
                psoundRenderer->SetMasterVolume(app->SoundVolume);
            }
        }
#endif
    }
    virtual String GetHelpString() const
    {
        return "Increase Sound Volume";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(VolumeUp);
};

class FxPlayerCommandVolumeDown : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
#ifdef GFX_ENABLE_SOUND
        Sound::SoundRenderer* psoundRenderer = app->GetSoundRenderer();
        if (psoundRenderer)
        {
            if (app->IsMute)
            {
                psoundRenderer->Mute(false);
                app->IsMute = false;
            } 
            else
            {
                app->SoundVolume-=10;
                if (app->SoundVolume < 0)
                {
                    app->SoundVolume = 0;
                }
                psoundRenderer->SetMasterVolume(app->SoundVolume);
            }
        }
#endif
    }
    virtual String GetHelpString() const
    {
        return "Decrease Sound Volume";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(VolumeDown);
};

class FxPlayerCommandStepForward_1 : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (!app->pMovie) return;
        app->pMovie->GotoFrame(app->pMovie->GetCurrentFrame() + 1);
        app->OnKeyFinishSeek();
    }
    virtual String GetHelpString() const
    {
        return "Step forward one frame";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(StepForward_1);
};

class FxPlayerCommandStepForward_10 : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (!app->pMovie) return;
        app->pMovie->GotoFrame(Alg::Min<unsigned>(app->pMovie->GetCurrentFrame() + 10, app->mMovieInfo.FrameCount - 1));
        app->OnKeyFinishSeek();
    }
    virtual String GetHelpString() const
    {
        return "Step forward 10 frames";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(StepForward_10);
};

class FxPlayerCommandStepBack_1 : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (!app->pMovie) return;
        app->pMovie->GotoFrame(app->pMovie->GetCurrentFrame() - 1);
        app->OnKeyFinishSeek();
    }
    virtual String GetHelpString() const
    {
        return "Step back one frame";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(StepBack_1);
};

class FxPlayerCommandStepBack_10 : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (!app->pMovie) return;
        app->pMovie->GotoFrame(Alg::Max<unsigned>(0, app->pMovie->GetCurrentFrame()- 10));
        app->OnKeyFinishSeek();
    }
    virtual String GetHelpString() const
    {
        return "Step back 10 frames";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(StepBack_10);
};

class FxPlayerCommandBackground : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (!app->pMovie) return;
        app->Background = !app->Background;
        app->pMovie->SetBackgroundAlpha(app->Background ? 1.0f : 0.0f);
    }
    virtual String GetHelpString() const
    {
        return "Toggle swf background";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Background);
};

class FxPlayerCommandBackgroundColor : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        switch(app->BackgroundColor)
        {
        case 0x555555: //Gray 
            app->BackgroundColor = 0x0000FF; 
            break;
        case 0x0000FF: //Blue
            app->BackgroundColor = 0x000000; 
            break;
        case 0x000000: //Black
            app->BackgroundColor = 0xFFFFFF; 
            break;
        case 0xFFFFFF: //White
            app->BackgroundColor = 0x555555; 
            break;
        }    
        app->GetRenderThread()->SetBackgroundColor(app->BackgroundColor);
    }

    virtual String GetHelpString() const
    {
        return "Cycle background color";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(BackgroundColor);
};

class FxPlayerCommandCurveToleranceUp : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->CurvePixelError = Alg::Min(10.0f, app->CurvePixelError + 0.5f);
        Render::ToleranceParams params;
        app->GetRenderThread()->GetToleranceParams(&params);
        params.CurveTolerance = app->CurvePixelError;
        app->GetRenderThread()->SetToleranceParams(params);
        app->AmpDirty = true;
    }
    virtual String GetHelpString() const
    {
        return "Curve tolerance up";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(CurveToleranceUp);
};

class FxPlayerCommandCurveToleranceDown : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->CurvePixelError = Alg::Max(0.5f, app->CurvePixelError - 0.5f);
        Render::ToleranceParams params;
        app->GetRenderThread()->GetToleranceParams(&params);
        params.CurveTolerance = app->CurvePixelError; 
        app->GetRenderThread()->SetToleranceParams(params);
        app->AmpDirty = true;
    }
    virtual String GetHelpString() const
    {
        return "Curve tolerance down";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(CurveToleranceDown);
};

class FxPlayerCommandLoadPrevFile : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (app->ContentDir.GetNextFile(&app->FileName, true))
        {
            app->LoadMovie(app->FileName);    
        }
    }
    virtual String GetHelpString() const
    {
        return "Load previous file";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(LoadPrevFile);
};

class FxPlayerCommandLoadNextFile : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        if (app->ContentDir.GetNextFile(&app->FileName, false))
        {
            app->LoadMovie(app->FileName);    
        }
    }
    virtual String GetHelpString() const
    {
        return "Load next file";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(LoadNextFile);
};

class FxPlayerCommandQuit : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->Shutdown();

    }
    virtual String GetHelpString() const
    {
        return "Quit";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Quit);
};

class FxPlayerCommandCtrlKey : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->NoCtrlKey = !app->NoCtrlKey;
    }
    virtual String GetHelpString() const
    {
        return "Toggle Ctrl key handling between Player and movie";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(CtrlKey);
};

class FxPlayerCommandEnter : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {        
        app->OnKeyEvent(controllerIdx, Key::Return, 0, keyDown);  
    }
    virtual String GetHelpString() const
    {
        return "Enter or Mouse button";
    }
    virtual bool HandleKeyUp() const { return true; }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Enter);
};

class FxPlayerCommandEscape : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED(controllerIdx);
        app->OnKeyEvent(controllerIdx, Key::Escape, 0, keyDown);
    }
    virtual String GetHelpString() const
    {
        return "Escape";
    }
    virtual bool HandleKeyUp() const { return true; }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Escape);
};


class FxPlayerCommandShowMouseCursor : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->EnableCursor(!app->IsCursorEnabled());        
        if (!app->pMovie) return;
        unsigned cursorCount = app->IsCursorEnabled() ? app->GetMouseCount() : 0;
        app->pMovie->SetMouseCursorCount(cursorCount);
    }
    virtual String GetHelpString() const
    {
        return "Enable mouse; Show/Hide cursor";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(ShowMouseCursor);
};

class FxPlayerCommandTakeScreenshot : public FxPlayerCommand
{
public:
    static int ScreenshotIndex;
    virtual bool HandleKeyUp() const { return true; }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);

        String screenshotFilename;
        
        if ( app->pMovieDef )
        {
            if ( !app->AutoTest || !app->AutoTest->IsPlayback())
            {
                screenshotFilename = (app->pMovieDef->GetFileURL());
                Format(screenshotFilename, "{0}_{1}", screenshotFilename, ScreenshotIndex++);
            }
            else
            {
                app->AutoTest->GetScreenshotFilename(screenshotFilename, ScreenshotIndex++);
            }
        }
        else
            screenshotFilename = "GFxPlayer_Screenshot";

        if ( !app->TakeScreenShot(screenshotFilename) )
            fprintf(stderr, "Failed to capture screenshot: %s\n", screenshotFilename.ToCStr());
    }

    virtual String GetHelpString() const
    {
        return "Takes a screenshot.";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(TakeScreenshot);
};
int FxPlayerCommandTakeScreenshot::ScreenshotIndex = 0;

//------------------------------------------------------------------------

void    FxPlayerAppBase::LoadFontConfigs(ConfigParser *parser)
{
    FontConfigs.Parse(parser);
    if (FontConfigs.GetSize() > 0)
    {
        FontConfigIndex = 0;
    }
    else
    {
        FontConfigIndex = -1;
    }
}

int     FxPlayerAppBase::GetFontConfigIndexByName(const char* pname)
{
    for (unsigned i = 0; i < FontConfigs.GetSize(); i++)
    {
        if (SFstricmp(pname, FontConfigs[i]->ConfigName) == 0)
            return i;
    }   
    return -1;
}

FontConfig*  FxPlayerAppBase::GetCurrentFontConfig()
{
    // we are skipping over invalid fontconfigs until one is found.
    // else return NULL.

    if (FontConfigIndex == -1)
        return NULL;

    FontConfig* fc = NULL;
    int  sIdx = FontConfigIndex;
    bool ok = false;

    while (!ok)
    {
        ok = true;
        fc = FontConfigs[FontConfigIndex];
        // check if all fontlib files exist
        for (unsigned i=0; i < fc->FontLibFiles.GetSize(); i++)
        {
            // check if file exists
            SysFile file(fc->FontLibFiles[i]);
            if (!file.IsValid())
            {
                ok = false;
                fprintf(stderr, "Fontlib file '%s' cannot be found. Skipping config '%s'..\n",
                        fc->FontLibFiles[i].ToCStr(), fc->ConfigName.ToCStr());
                break;
            }
        }

        if (!ok)
        {
            FontConfigIndex++;
            FontConfigIndex %= (int)FontConfigs.GetSize();
            if (FontConfigIndex == sIdx)
                return NULL;
        }
    }

    return FontConfigs[FontConfigIndex];
}

bool FxPlayerAppBase::LoadDefaultFontConfigFromPath( const String& path )
{
    if (!GetArgs()["NoFontConfig"]->GetBool())
    {
        // load fontconfig.txt if it exists in the movie's path
        String fontConfigFilePath = GetArgs()["FontConfigFile"]->GetString();

        if (fontConfigFilePath.IsEmpty())
        {
            // [PPS] The absolute check is unneccessary
            // ExtractFilePath will return the correct parent path:
            // C:/folder/filename.ext - C:/folder/
            // folder/filename.ext - folder/
            // filename.ext - empty string
            //if (URLBuilder::IsPathAbsolute(path))
            //{
            fontConfigFilePath.AppendString(path);
            if ( !URLBuilder::ExtractFilePath(&fontConfigFilePath) )
            {
                fontConfigFilePath = "";
            }
            //}
            fontConfigFilePath += "fontconfig.txt";
        }
        
        bool maintainIndex = false;

        // store font config file related info
        if (FontConfigFilePath.GetLength() == 0)   // if no file was previously loaded
        {
            FileStat fileStats;
            if ( SysFile::GetFileStat(&fileStats, fontConfigFilePath.ToCStr()) )
            {
                FontConfigFilePath = fontConfigFilePath;
                FontConfigFileStats = fileStats;
            }
        }
        else // if the file was previously loaded and is modified
        {
            if (fontConfigFilePath == FontConfigFilePath)
            {
                // if modified time is not the same, then reload config file
                FileStat fileStats;
                if ( SysFile::GetFileStat(&fileStats, fontConfigFilePath.ToCStr()) )
                {
                    if ( !(fileStats == FontConfigFileStats) )
                    {
                        FontConfigFileStats = fileStats;
                        maintainIndex = true;
                    }
                }
            }
        }

        // parse the config file
        ConfigParser parser(fontConfigFilePath.ToCStr());
        int          oldIdx = FontConfigIndex;
        LoadFontConfigs(&parser);

        // try to maintain previous font config index
        if ( maintainIndex &&
            (FontConfigIndex == 0) &&
            (oldIdx != -1) )
        {
            FontConfigIndex = oldIdx;
            FontConfigIndex %= (int)FontConfigs.GetSize();
        }

        return true;
    }
    return false;
}




// ***** FxPlayerAppBase Implementation

FxPlayerAppBase::FxPlayerAppBase()
{
    Wireframe       = false;
    AmpDirty        = true;
    // Scale toggle, on by default
    ScaleEnable     = true;
    ClippingEnable  = true;
    Paused          = false;
    PausedState     = GFx::State_Playing;

    StickMode       = PSM_StickAndPad;

    ArgResolution.Clear();
    ScaleX = ScaleY     = 1.0f;
    TexLodBias          = -0.5f;
    AAMode              = AAMode_EdgeAA;
    Background          = 1;

    BackgroundColor     = 0x555555;

    MeasurePerformance = IsConsole();

    // No font config used by default.
    FontConfigIndex     = -1;

    AutoTest            = 0;

    FastForward         = false;
    ExitTimeout         = 0.0f;
    
    // *** Clear timing
    AccumClockTicks = 0;
    PrevTicks       = 0;
    LastFPSUpdatedTicks  = 0;
    LastStatUpdatedTicks = 0;
        
    LastFPS         = 0.0f;
    AccumFPS        = 0.0f;
    FrameCounter    = 0;
    AccumFPSSecondCount = 0;
    LastAdvanceTicks= 0;

    PlayOnce        = false;
    Rendering       = true;
    DoSound         = 0;
    NoControlKeys   = 0;
    NoCtrlKey       = 0;

    SoundVolume     = 1.0f;
    IsMute          = 0;

    bCLIKSound      = false;

    MViewSize.SetSize(0, 0);
    MinSize.SetSize(0, 0);
    MovieCursorType = Platform::Cursor_Arrow;

    Zoom = 1.0;
    Move = PointF(0.0);

    MouseTracking = None;

    PadPassthrough = false;

    SizingEntered = 0;

    CurvePixelError = 1.0f;
    FontLibChanged = false;

#ifdef GFX_ENABLE_SOUND
#ifdef GFX_SOUND_FMOD
    pSoundFMODSystem  = NULL;
#endif
#ifdef GFX_SOUND_WWISE
    pSoundWwiseSystem = NULL;
#endif
#endif

    ProfileMode     = Render::Profile_None;
    BatchHighlight  = -1; // Disabled

    KeyboardIndex = 0;
    ResIndex  = 1;
    SkipLoadMovie = false;

    mMovieInfo.Clear();

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360) || defined(FXPLAYER_X11) || defined(SF_OS_MAC)
    UInt32 ctrlMask = (KeyModifiers::Key_CtrlPressed << 16);
    UInt32 altMask = (KeyModifiers::Key_AltPressed << 16);
    
    KeyCommandMap.Set(Key::S | ctrlMask, *SF_NEW FxPlayerCommandScaledDisplay());
    KeyCommandMap.Set(Key::W | ctrlMask, *SF_NEW FxPlayerCommandWireframe());
    KeyCommandMap.Set(Key::A | ctrlMask, *SF_NEW FxPlayerCommandAntialiasingMode());
    #if !defined(SF_OS_XBOX360)
        KeyCommandMap.Set(Key::U | ctrlMask, *SF_NEW FxPlayerCommandFullscreen());
    #endif
    KeyCommandMap.Set(Key::F | ctrlMask, *SF_NEW FxPlayerCommandFastMode());
    KeyCommandMap.Set(Key::G | ctrlMask, *SF_NEW FxPlayerCommandFastForward());
    KeyCommandMap.Set(Key::Y | ctrlMask, *SF_NEW FxPlayerCommandVSync());
    KeyCommandMap.Set(Key::P | ctrlMask, *SF_NEW FxPlayerCommandPause());
    KeyCommandMap.Set(Key::I | ctrlMask, *SF_NEW FxPlayerCommandObjectsReport());
    KeyCommandMap.Set(Key::N | ctrlMask, *SF_NEW FxPlayerCommandFontConfig());
    KeyCommandMap.Set(Key::R | ctrlMask, *SF_NEW FxPlayerCommandRestart());
    KeyCommandMap.Set(Key::D | ctrlMask, *SF_NEW FxPlayerCommandStageClipping());
    KeyCommandMap.Set(Key::C | ctrlMask, *SF_NEW FxPlayerCommandStageCulling());
    KeyCommandMap.Set(Key::Z | ctrlMask, *SF_NEW FxPlayerCommandResetUserMatrix());
    KeyCommandMap.Set(Key::O | ctrlMask, *SF_NEW FxPlayerCommandTriangleOptimization());
    KeyCommandMap.Set(Key::T | ctrlMask, *SF_NEW FxPlayerCommandStrokeMode());
    KeyCommandMap.Set(Key::E | ctrlMask, *SF_NEW FxPlayerCommandToggleProfileMode<Render::Profile_Overdraw>());
    KeyCommandMap.Set(Key::J | ctrlMask, *SF_NEW FxPlayerCommandToggleProfileMode<Render::Profile_Batch>());
    KeyCommandMap.Set(Key::E | altMask|ctrlMask, *SF_NEW FxPlayerCommandToggleProfileMode<Render::Profile_TextureDensity>());
    KeyCommandMap.Set(Key::J | altMask|ctrlMask, *SF_NEW FxPlayerCommandToggleProfileMode<Render::Profile_Blending>());
    KeyCommandMap.Set(Key::Num1 | ctrlMask, *SF_NEW FxPlayerCommandChangeHighlightBatch<-1>()); // decrement highlight batch
    KeyCommandMap.Set(Key::Num2 | ctrlMask, *SF_NEW FxPlayerCommandChangeHighlightBatch<+1>()); // increment highlight batch
    KeyCommandMap.Set(Key::F3 | ctrlMask, *SF_NEW FxPlayerCommandToggleProfilerFlag<Render::ProfileFlag_NoFilterCaching>());
    KeyCommandMap.Set(Key::F3 | ctrlMask | altMask, *SF_NEW FxPlayerCommandToggleProfilerFlag<Render::ProfileFlag_NoBlendCaching>());
    KeyCommandMap.Set(Key::F4 | ctrlMask, *SF_NEW FxPlayerCommandToggleProfilerFlag<Render::ProfileFlag_NoBatching>());
    KeyCommandMap.Set(Key::F5 | ctrlMask, *SF_NEW FxPlayerCommandToggleProfilerFlag<Render::ProfileFlag_NoInstancing>());
    KeyCommandMap.Set(Key::F6 | ctrlMask, *SF_NEW FxPlayerCommandMemReport());
    KeyCommandMap.Set(Key::F7 | ctrlMask, *SF_NEW FxPlayerCommandFunctionTiming());
    #ifdef GFX_ENABLE_SOUND
        KeyCommandMap.Set(Key::M | ctrlMask, *SF_NEW FxPlayerCommandMute());
        KeyCommandMap.Set(Key::Up | ctrlMask, *SF_NEW FxPlayerCommandVolumeUp());
        KeyCommandMap.Set(Key::Down | ctrlMask, *SF_NEW FxPlayerCommandVolumeDown());
    #endif
    KeyCommandMap.Set(Key::Right | ctrlMask, *SF_NEW FxPlayerCommandStepForward_1());
    KeyCommandMap.Set(Key::PageUp | ctrlMask, *SF_NEW FxPlayerCommandStepForward_10());
    KeyCommandMap.Set(Key::Left | ctrlMask, *SF_NEW FxPlayerCommandStepBack_1());
    KeyCommandMap.Set(Key::PageDown | ctrlMask, *SF_NEW FxPlayerCommandStepBack_10());
    KeyCommandMap.Set(Key::B | ctrlMask, *SF_NEW FxPlayerCommandBackground());
    KeyCommandMap.Set(Key::F8 | ctrlMask, *SF_NEW FxPlayerCommandBackgroundColor());
    KeyCommandMap.Set(Key::Equal | ctrlMask, *SF_NEW FxPlayerCommandCurveToleranceUp());
    KeyCommandMap.Set(Key::Minus | ctrlMask, *SF_NEW FxPlayerCommandCurveToleranceDown());
    KeyCommandMap.Set(Key::BracketLeft | ctrlMask, *SF_NEW FxPlayerCommandLoadPrevFile());
    KeyCommandMap.Set(Key::BracketRight | ctrlMask, *SF_NEW FxPlayerCommandLoadNextFile());
    KeyCommandMap.Set(Key::Q | ctrlMask, *SF_NEW FxPlayerCommandQuit());
    KeyCommandMap.Set(Key::F9, *SF_NEW FxPlayerCommandCtrlKey());
    KeyCommandMap.Set(Key::F11, *SF_NEW FxPlayerCommandChangeRes());
#ifdef GFX_ENABLE_XML
    KeyCommandMap.Set(Key::PrintScreen, *SF_NEW FxPlayerCommandTakeScreenshot());
#endif
#endif

#if defined(SF_OS_XBOX360) || defined(SF_OS_WIN32) || defined(_DURANGO)
    PadKeyCommandMap.Set(SF::Pad_A, *SF_NEW FxPlayerCommandEnter());
    PadKeyCommandMap.Set(SF::Pad_B, *SF_NEW FxPlayerCommandEscape());
    PadKeyCommandMap.Set(SF::Pad_Y, *SF_NEW FxPlayerCommandRestart());
    PadKeyCommandMap.Set(SF::Pad_Start, *SF_NEW FxPlayerCommandPause());
    //PadKeyCommandMap.Set(SF::Pad_Back, *SF_NEW FxPlayerCommandCycleProfile());
    PadKeyCommandMap.Set(SF::Pad_Back, *SF_NEW FxPlayerPadPassThrough());
    PadKeyCommandMap.Set(SF::Pad_L1, *SF_NEW FxPlayerCommandAntialiasingMode());
    PadKeyCommandMap.Set(SF::Pad_R1, *SF_NEW FxPlayerCommandWireframe());
    PadKeyCommandMap.Set(SF::Pad_L2, *SF_NEW FxPlayerCommandLoadPrevFile());
    PadKeyCommandMap.Set(SF::Pad_R2, *SF_NEW FxPlayerCommandLoadNextFile());
    PadKeyCommandMap.Set(SF::Pad_LT, *SF_NEW FxPlayerCommandFastForward());
#if defined(SF_OS_XBOX360)  || defined(_DURANGO)
    PadKeyCommandMap.Set(SF::Pad_RT, *SF_NEW FxPlayerCommandShowMouseCursor());
#endif
    PadPassthroughMap.Set(SF::Pad_A, Key::Return);
    PadPassthroughMap.Set(SF::Pad_B, Key::Escape);
    PadPassthroughMap.Set(SF::Pad_X, Key::Minus);
    PadPassthroughMap.Set(SF::Pad_Y, Key::Equal);
    PadPassthroughMap.Set(SF::Pad_Start, Key::Bar);
    PadPassthroughMap.Set(SF::Pad_L1, Key::PageUp);
    PadPassthroughMap.Set(SF::Pad_R1, Key::PageDown);
    PadPassthroughMap.Set(SF::Pad_L2, Key::Home);
    PadPassthroughMap.Set(SF::Pad_R2, Key::End);
    PadPassthroughMap.Set(SF::Pad_LT, Key::Insert);
    PadPassthroughMap.Set(SF::Pad_RT, Key::Delete);
#elif defined(SF_OS_PS3) || defined(SF_OS_ORBIS)
    PadKeyCommandMap.Set(SF::Pad_X, *SF_NEW FxPlayerCommandEnter());
    PadKeyCommandMap.Set(SF::Pad_O, *SF_NEW FxPlayerCommandEscape());
    PadKeyCommandMap.Set(SF::Pad_T, *SF_NEW FxPlayerCommandRestart());
    PadKeyCommandMap.Set(SF::Pad_Start, *SF_NEW FxPlayerCommandPause());
    PadKeyCommandMap.Set(SF::Pad_Select, *SF_NEW FxPlayerPadPassThrough());
    PadKeyCommandMap.Set(SF::Pad_L1, *SF_NEW FxPlayerCommandAntialiasingMode());
    PadKeyCommandMap.Set(SF::Pad_R1, *SF_NEW FxPlayerCommandWireframe());
    PadKeyCommandMap.Set(SF::Pad_L2, *SF_NEW FxPlayerCommandLoadPrevFile());
    PadKeyCommandMap.Set(SF::Pad_R2, *SF_NEW FxPlayerCommandLoadNextFile());
    PadKeyCommandMap.Set(SF::Pad_LT, *SF_NEW FxPlayerCommandFastForward());
    PadKeyCommandMap.Set(SF::Pad_RT, *SF_NEW FxPlayerCommandShowMouseCursor());

    PadPassthroughMap.Set(SF::Pad_X, Key::Return);
    PadPassthroughMap.Set(SF::Pad_O, Key::Escape);
    PadPassthroughMap.Set(SF::Pad_T, Key::Minus);
    PadPassthroughMap.Set(SF::Pad_S, Key::Equal);
    PadPassthroughMap.Set(SF::Pad_Start, Key::Bar);
    PadPassthroughMap.Set(SF::Pad_L1, Key::PageUp);
    PadPassthroughMap.Set(SF::Pad_R1, Key::PageDown);
    PadPassthroughMap.Set(SF::Pad_L2, Key::Home);
    PadPassthroughMap.Set(SF::Pad_R2, Key::End);
    PadPassthroughMap.Set(SF::Pad_LT, Key::Insert);
    PadPassthroughMap.Set(SF::Pad_RT, Key::Delete);
#elif defined(SF_OS_WII)
    PadKeyCommandMap.Set(SF::Pad_A, *SF_NEW FxPlayerCommandEnter());
    PadKeyCommandMap.Set(SF::Pad_B, *SF_NEW FxPlayerCommandEscape());
    PadKeyCommandMap.Set(SF::Pad_Home, *SF_NEW FxPlayerCommandPause());
    PadKeyCommandMap.Set(SF::Pad_Z,  *SF_NEW FxPlayerPadPassThrough());
    PadKeyCommandMap.Set(SF::Pad_C, *SF_NEW FxPlayerCommandFastForward());
    PadKeyCommandMap.Set(SF::Pad_Plus, *SF_NEW FxPlayerCommandLoadNextFile());
    PadKeyCommandMap.Set(SF::Pad_Minus, *SF_NEW FxPlayerCommandLoadPrevFile());

    PadPassthroughMap.Set(SF::Pad_A, Key::Return);
    PadPassthroughMap.Set(SF::Pad_B, Key::Escape);
    PadPassthroughMap.Set(SF::Pad_Home, Key::Home);
    PadPassthroughMap.Set(SF::Pad_C, Key::PageDown);
    PadPassthroughMap.Set(SF::Pad_Plus, Key::Equal);
    PadPassthroughMap.Set(SF::Pad_Minus, Key::Minus);


#elif defined(SF_OS_WIIU) //Some functions may be assigned to two buttons for use with Wii controller
    PadKeyCommandMap.Set(SF::Pad_A, *SF_NEW FxPlayerCommandEnter());
    PadKeyCommandMap.Set(SF::Pad_B, *SF_NEW FxPlayerCommandEscape());
    PadKeyCommandMap.Set(SF::Pad_Y, *SF_NEW FxPlayerCommandRestart());
    PadKeyCommandMap.Set(SF::Pad_Plus, *SF_NEW FxPlayerCommandPause());
    PadKeyCommandMap.Set(SF::Pad_Home, *SF_NEW FxPlayerCommandPause());
    PadKeyCommandMap.Set(SF::Pad_Minus, *SF_NEW FxPlayerPadPassThrough());
    PadKeyCommandMap.Set(SF::Pad_Z, *SF_NEW FxPlayerCommandAntialiasingMode());
    PadKeyCommandMap.Set(SF::Pad_L1, *SF_NEW FxPlayerCommandAntialiasingMode());
    PadKeyCommandMap.Set(SF::Pad_R1, *SF_NEW FxPlayerCommandFastForward());
    PadKeyCommandMap.Set(SF::Pad_L2, *SF_NEW FxPlayerCommandLoadPrevFile());
    PadKeyCommandMap.Set(SF::Pad_R2, *SF_NEW FxPlayerCommandLoadNextFile());
    PadKeyCommandMap.Set(SF::Pad_C, *SF_NEW FxPlayerCommandFastForward());

    PadPassthroughMap.Set(SF::Pad_A, Key::Return);
    PadPassthroughMap.Set(SF::Pad_B, Key::Escape);
    PadPassthroughMap.Set(SF::Pad_X, Key::Minus);
    PadPassthroughMap.Set(SF::Pad_Y, Key::Equal);
    PadPassthroughMap.Set(SF::Pad_Plus, Key::Bar);
    PadPassthroughMap.Set(SF::Pad_L1, Key::PageUp);
    PadPassthroughMap.Set(SF::Pad_R1, Key::PageDown);
    PadPassthroughMap.Set(SF::Pad_L2, Key::Home);
    PadPassthroughMap.Set(SF::Pad_R2, Key::End);
    PadPassthroughMap.Set(SF::Pad_LT, Key::Insert);
    PadPassthroughMap.Set(SF::Pad_RT, Key::Delete);
#else
    PadKeyCommandMap.Set(SF::Pad_X, *SF_NEW FxPlayerCommandEnter());
    PadKeyCommandMap.Set(SF::Pad_O, *SF_NEW FxPlayerCommandEscape());
    PadKeyCommandMap.Set(SF::Pad_T, *SF_NEW FxPlayerCommandRestart());
    PadKeyCommandMap.Set(SF::Pad_Start, *SF_NEW FxPlayerCommandPause());
    PadKeyCommandMap.Set(SF::Pad_Select, *SF_NEW FxPlayerCommandCycleProfile());
    PadKeyCommandMap.Set(SF::Pad_L1, *SF_NEW FxPlayerCommandLoadPrevFile());
    PadKeyCommandMap.Set(SF::Pad_R1, *SF_NEW FxPlayerCommandLoadNextFile());
    PadKeyCommandMap.Set(SF::Pad_LT, *SF_NEW FxPlayerCommandFastForward());
    PadKeyCommandMap.Set(SF::Pad_Y, *SF_NEW FxPlayerCommandFunctionTiming());

#endif
}

FxPlayerAppBase::~FxPlayerAppBase()      
{
#ifdef GFX_ENABLE_SOUND
    xShutdownSound();
#endif
}

#ifdef GFX_ENABLE_SOUND
void FxPlayerAppBase::InitializeSound()
{
#ifdef GFX_SOUND_FMOD
    if (!pSoundFMODSystem)
        pSoundFMODSystem = new FxSoundFMOD;
    if (!pSoundFMODSystem->IsInitialized())
    {
        bool bIsInitFMOD = pSoundFMODSystem->Initialize(
#ifdef SF_OS_PS3        
            GetSpurs()
#endif
            );
        if (!bIsInitFMOD)
            xShutdownSound();
    }
#endif // GFX_SOUND_FMOD

#ifdef GFX_SOUND_WWISE
    if (!pSoundWwiseSystem)
        pSoundWwiseSystem = new FxSoundWwise;
    if (!pSoundWwiseSystem->IsInitialized())
    {
        bool bIsInitWwise = pSoundWwiseSystem->Initialize(
#ifdef SF_OS_PS3
            GetSpurs()
#endif          
            );
        if (!bIsInitWwise)
            xShutdownSound();
    }
#endif // GFX_SOUND_WWISE
}

void FxPlayerAppBase::xShutdownSound()
{
#ifdef GFX_SOUND_FMOD
    if (pSoundFMODSystem && pSoundFMODSystem->IsInitialized())
        pSoundFMODSystem->Finalize();
    delete pSoundFMODSystem;
    pSoundFMODSystem = NULL;
#endif

#ifdef GFX_SOUND_WWISE
    if (pSoundWwiseSystem && pSoundWwiseSystem->IsInitialized())
        pSoundWwiseSystem->Finalize();
    delete pSoundWwiseSystem;
    pSoundWwiseSystem = NULL;
#endif
}

Sound::SoundRenderer* FxPlayerAppBase::GetSoundRenderer()
{
#ifdef GFX_SOUND_FMOD
    return pSoundFMODSystem && pSoundFMODSystem->IsInitialized() ? pSoundFMODSystem->GetSoundRenderer() : NULL;
#else
    return NULL;
#endif
}
#endif // GFX_ENABLE_SOUND

#if defined(SF_OS_WIN32) && !defined(_DURANGO)

// Install system-specific clipboard implementation on Win32. If this is not done
// the clipboard will still work in FxPlayer, but it will be impossible to paste
// text to external applications.
class FxPlayerTextClipboard : public Clipboard
{
public:
    void OnTextStore(const wchar_t* ptext, UPInt len)
    {
        // store the text in a system clipboard.
        if (OpenClipboard(NULL))
        {
            // Empty the Clipboard. This also has the effect
            // of allowing Windows to free the memory associated
            // with any data that is in the Clipboard
            EmptyClipboard();

            HGLOBAL hClipboardData;
            hClipboardData = GlobalAlloc(GMEM_DDESHARE, (len+1)*sizeof(wchar_t));
            SF_ASSERT(sizeof(wchar_t) == 2);

            // Calling GlobalLock returns a pointer to the 
            // data associated with the handle returned from 
            // GlobalAlloc
            wchar_t * pchData;
            pchData = (wchar_t*)GlobalLock(hClipboardData);
            SFwcscpy(pchData, len+1, ptext);

            // Once done, unlock the memory. 
            // don't call GlobalFree because Windows will free the 
            // memory automatically when EmptyClipboard is next 
            // called. 
            GlobalUnlock(hClipboardData);

            // Now, set the Clipboard data by specifying that 
            // Unicode text is being used and passing the handle to
            // the global memory.
            SetClipboardData(CF_UNICODETEXT, hClipboardData);

            // Finally, close the Clipboard which has the effect of 
            // unlocking it so that other  apps can examine or modify its contents.
            CloseClipboard();
        }
    }

    UInt32 GetSupportedFormats() 
    {
        return Format_Text | Format_RichText | Format_Bitmap;
    }

    UInt32 GetAvailableFormats() 
    {
        UInt32 result = 0;
        if (IsClipboardFormatAvailable(CF_TEXT)) 
            result|= Format_Text;        
        return result;
    }
};


class FxPlayerMultitouchInterface : public MultitouchInterface
{
public:
    // Should return maximum number of touch points supported by hardware
    virtual unsigned GetMaxTouchPoints() const { return 2; }

    // Should return a bit mask of supported gestures (see MTG_*)
    virtual UInt32   GetSupportedGesturesMask() const { return MTG_Pan | MTG_Zoom | MTG_Rotate; }

    // Sets multitouch input mode. If the input mode is not supported it should
    // return 'false'; otherwise 'true'.
    virtual bool     SetMultitouchInputMode(MultitouchInputMode) { return true; }
};

#endif

void FxPlayerAppBase::InitArgDescriptions(Platform::Args* args)
{
    using namespace Platform;
    BaseClass::InitArgDescriptions(args);

    ArgDesc options []=
    {
        //      {"","--------------spacer example------------------\n","",FxCmdOption::Spacer,""},
        {"",    "FileName",            Args::StringOption | Args::Positional, NULL, //"gfx_cri.swf", 
        "GFX or SWF file to load at startup"},
        {"arg", "Arg",                 Args::StringOption, NULL, //"gfx_cri.swf", 
        "Value for _global.gfxArg variable in ActionScript."},
        {"s",   "ScaleFactor",         Args::FloatOption, "1.0", 
        "<factor> Scale the movie window size by the specified factor."},

        {"vol", "SoundVolume",         Args::IntOption, "80", "Sound Volume. 0-100 (default 80)"},
        {"cksd", "CLIKSound",          Args::Flag, "", "Enable default CLIK sound events."},
        {"mute", "SoundMute",          Args::Flag, "", "Mute sound"},
        {"noclip", "NoClipping",      Args::Flag, "", "Disable stage clipping on startup"},
        {"na",  "noAA",                Args::Flag, "", "Use no anti-aliasing."},
        {"fsa", "FSAA",                Args::Flag, "", "Use use fullscreen HW AA."}, 
        {"nsf", "NoSystemFont",        Args::Flag, "", "No system font - disables FontProviderWin32."},
        {"nfc", "NoFontConfig",        Args::Flag, "", "Disable autoloading of font config file in movie's path."},
        {"fc",  "FontConfigFile",      Args::StringOption, "", "<fname> Load a font config file."},
        {"fl",  "FontLibFile",         Args::StringOption, NULL, 
                                       "<fname>  Specifies a SWF/GFX file to load into FontLib."},
        {"ipk", "UseImagePacker",      Args::Flag, "", "Use ImagePacker (for debug/test only)"},
        {"vp",  "VerboseParse",        Args::Flag, "", "Verbose parse - print SWF parse log."},
        {"vps", "VerboseParseShape",   Args::Flag, "" ,"Verbose parse shape - print SWF shape parse log."},
        {"vpa", "VerboseParseAction",  Args::Flag, "", "Verbose parse action - print SWF actions during parse."},
        {"vfq", "NoLogChildFilnames",  Args::Flag, "", "Do not print SWF filename for ActionScript errors in child movies."},
        {"vfr", "LogRootFilenames",    Args::Flag, "","Print SWF filename for ActionScript errors in root movies."},
        {"vfp", "LogFilePath",         Args::Flag, "", "Print full file path for ActionScript errors."},
        {"va",  "VerboseActions",      Args::Flag, "", "Verbose actions - display ActionScript execution log."},
        {"q",   "Quiet",               Args::Flag, "", "Quiet. Do not display errors or trace statements."},
        {"xmllog",   "XmlLog",         Args::Flag, "", "XML Log. Output errors and trace statement in XML format"},
        {"qae", "SuppressASErrors",    Args::Flag, "", "Quiet. Suppress ActionScript errors."},
        {"ml",  "LodBias",             Args::FloatOption, "-0.5", 
        "<bias>  Specify the texture LOD bias (float, default -0.5)"},
        {"rs",  "RecordStatistics",    Args::StringOption, NULL,
                                       "<filename> Save current performance/memory statistics at exit."},
        {"ff",  "FastForward",         Args::Flag, "", "Fast forward - run one frame per update."},
        {"fps", "LogFPS",              Args::Flag, "", "Log FPS to console every second."},
        {"1",   "PlayOnce",            Args::Flag, "", "Play once; exit when/if movie reaches the last frame."},
        {"nr",  "NoRendering",         Args::Flag, "", "Disable rendering (for batch tests)"},
        {"psh", "PrecompileShaders",   Args::Flag, "", "Compile all shaders on startup"},
        {"ns",  "NoSound",             Args::Flag, "", "Disable sound (for batch tests)"},
        {"nck", "NoControlKeys",       Args::Flag, "", "Disable all player related control keys."},
        {"t",   "ExitTimeout",         Args::FloatOption, "0.0",
        "<msec>    Timeout and exit after the specified number of milliseconds."},
        {"mtl", "ProgressiveLoading", Args::Flag, "", "Enable progressive loading."},
        {"res", "Resolution",          Args::StringOption, "", "Override GFxPlayer resolution with custom values\n"
        "              of width and height using -res W:H."},
#ifdef SF_AMP_SERVER
        {"amp", "AmpPort",             Args::IntOption, "7534", "Port for AMP socket connection. (default 7534)"},
        {"wait","AmpWait",             Args::Flag, "0", "Wait until an AMP connection has been established"},
        {"pl","AmpProfileLevel",       Args::IntOption, "-1", "AMP function profiling level: 0=Low (default), 1=Medium, 2=High."},
        {"mem","AmpMemDetail",         Args::Flag, "0", "AMP detailed memory reports."},
#endif
        {"autotest","AutoPlayback",    Args::StringOption, NULL, "Use the given filename as instructions for an automated test playback." },
        {"autodir", "AutoOutput",      Args::StringOption, "AutoOutput", "Destination directory (prefix) for autotest output." },
        {"record",  "AutoRecord",      Args::StringOption, NULL, "The filename which will record a test." },

        {"","",Args::ArgEnd,"",""},
    };
    args->AddDesriptions(options);
}

String FxPlayerAppBase::GetAppTitle() const
{
    String title("Scaleform GFxPlayer ");

    // Platform
    title += AppBase::GetPlatformName();

    // Version
    title += " v " GFX_VERSION_STRING;

    // Add the word "Debug" to the application
    // title if in debug build mode
#ifdef SF_BUILD_DEBUG
    title += " " SF_BUILD_STRING;
#endif

    return title;
}

String FxPlayerAppBase::GetFilePath() const
{
#if defined(SF_OS_XBOX360)
    return "D:\\Samples\\flash.swf";
#elif defined (SF_OS_PS3)
    return SYS_HOST_ROOT "/usr/local/cell/FxPlayer/flash.swf";
#elif defined (SF_OS_3DS)
    return "rom:/FxPlayer/flash.swf";
#elif defined (SF_OS_WII)
    return "FxPlayer/flash.swf";
#elif defined (SF_OS_PSVITA) || defined(SF_OS_WIIU) || defined (SF_OS_ORBIS)
    return String(GetDefaultFilePath()) + "/flash.swf";
#elif defined(_DURANGO)
	// This can be removed once Directory service is implemented for Durango
	return String(GetDefaultFilePath()) + "flash.swf";
#else
    // return String(GetDefaultFilePath()) + "/flash.swf";
    return "";
#endif
}


#define FXPLAYER_APP_TITLE  "GFx " GFX_VERSION_STRING

void FxPlayerAppBase::UpdateWindowTitle()
{
    String strTitle;
    
    if (pMovie && (MeasurePerformance || FastForward)) 
    {
        // Display frame rate in title
        //Format(strTitle, "{1} (fps:{0:3.1})", LastFPS, GetAppTitle());

        Render::HAL::Stats rstats;
        pRenderThread->GetRenderStats(&rstats);
        UInt64 displayTicks = pRenderThread->GetLastDisplayTicks();
        UInt64 otherTicks   = pRenderThread->GetLastDrawFrameTicks() - displayTicks;

        //@TODO - change sprintf to Format
        char buffer[512]; 
        if (pRenderThread->IsSingleThreaded())
        {
            SF::SFsprintf(buffer, sizeof(buffer),
                FXPLAYER_APP_TITLE " - FPS:%d (M=%d, P=%d, T=%d) A=%.3f, D=%.3f, O=%.3f",
                (int)LastFPS,
                rstats.Meshes, rstats.Primitives, rstats.Triangles,
                double(LastAdvanceTicks)/1000.0,
                double(displayTicks)/1000.0,
                double(otherTicks)/1000.0);
        }
        else
        {
            // TBD: Add Render thread FPS once they are de-synchronized.
            //" - A_FPS:%d, R_FPS:%d  (M=%d, P=%d, T=%d) A=%.3f, D=%.3f, O=%.3f",

            SF::SFsprintf(buffer, sizeof(buffer),
                FXPLAYER_APP_TITLE " MT - FPS:%d (M=%d, P=%d, T=%d) A=%.3f, D=%.3f, O=%.3f",
                (int)LastFPS, // LastDisplayFrames,
                rstats.Meshes, rstats.Primitives,rstats.Triangles,
                double(LastAdvanceTicks)/1000.0,
                double(displayTicks)/1000.0,
                double(otherTicks)/1000.0);
        }
        strTitle = buffer;
    }    

    if (strTitle.IsEmpty())
    {
        strTitle = GetAppTitle();
        if (pRenderThread)
        {
            if (pRenderThread->IsSingleThreaded())
                strTitle += " ST ";
            else
                strTitle +="  MT ";
        }
        else
            strTitle = "  NR ";
    }
    
    if (ViewConfigHasFlag(Platform::View_VSync))
        strTitle += " VSync";

#ifdef SF_AMP_SERVER
    String portString;
    if (AmpServer::GetInstance().IsValidConnection())
    {
        Format(portString, " Connected to AMP ({0})", AmpPort);
    }
    else if (AmpServer::GetInstance().IsEnabled())
    {
        Format(portString, " Listening for AMP ({0})", AmpPort);
    }
    strTitle += portString;
#endif

    SetWindowTitle(strTitle);
}


bool FxPlayerAppBase::OnArgs(const Platform::Args& args, Platform::Args::ParseResult parseResult) 
{
    if (!BaseClass::OnArgs(args, parseResult))
        return false;

    if (args.HasValue("FileName"))
    {
        FileName = args.GetString("FileName");
    }
    String path = FileName.GetPath();
#if defined(SF_OS_WIIU)
    if (path.GetLength() == 0)
        path = "/vol/content/";
    ContentDir.Reread(path, "*.swf");
#elif defined(SF_OS_XBOX360) || defined(SF_OS_WIN32)
    if (path.GetLength() == 0)
        path = ".";
    ContentDir.Reread(path, "*.swf;*.gfx");
#else
    if (path.GetLength() == 0)
        path = ".";
    ContentDir.Reread(path, "*.swf");
#endif

    String customRes = args.GetString("Resolution");
    if (customRes.GetSize() > 0)
    {
        const char* restr = customRes.ToCStr();
        ArgResolution.Width = (unsigned) atoi(restr);
        restr = SFstrchr(restr, ':');
        if (!restr) 
            ArgResolution.Clear();
        else 
            ArgResolution.Height = (unsigned) atoi(restr+1);
    }
    ScaleX = ScaleY = args.GetFloat("ScaleFactor");
    Rendering =       !args.GetBool("NoRendering");
    DoSound =         !args.GetBool("NoSound");
    FastForward =     args.GetBool("FastForward");
    ExitTimeout =     args.GetFloat("ExitTimeout");
    ClippingEnable =  !args.GetBool("NoClipping");

    PlayOnce = args.GetBool("PlayOnce");
    NoControlKeys = args.GetBool("NoControlKeys");
    NoCtrlKey = NoControlKeys;
    SoundVolume = args.GetInt("SoundVolume") / 100.0f;
    if(SoundVolume < 0.0f)
        SoundVolume = 0.0f;
    else if (SoundVolume > 1.0f)
        SoundVolume = 1.0f;

    IsMute     = args.GetBool("SoundMute");
    bCLIKSound = args.GetBool("CLIKSound");

#ifdef SF_AMP_SERVER
    AmpPort = args.GetInt("AmpPort");
#endif


    // Setup automated testing, if desired.
    bool autoPlayback = args.HasValue("AutoPlayback");
    bool autoRecord   = args.HasValue("AutoRecord");
    if (autoPlayback || autoRecord)
    {
        SF_DEBUG_ASSERT(autoPlayback ^ autoRecord, "Error, cannot both playback and record simultaneously. "
            "Figure out what you'd like to do and try again.");
        AutoTest = SF_NEW FxPlayerAutoTest(this, autoPlayback, 
            autoPlayback ? args.GetString("AutoPlayback").ToCStr() : args.GetString("AutoRecord").ToCStr(),
            autoPlayback ? args.GetString("AutoOutput").ToCStr() : 0 );
    }


    return true;
}

void FxPlayerAppBase::SetFileOpener()
{
    // File callback.
    Ptr<FileOpener> pfileOpener = *new FxPlayerFileOpener;  
    mLoader.SetFileOpener(pfileOpener);
}

bool FxPlayerAppBase::OnInit(Platform::ViewConfig& config)
{
    // Set the verbose flags.
    unsigned verboseFlags = 0;
    const Platform::Args& args = GetArgs();

    if (args.GetBool("VerboseParse"))//Settings.VerboseParse
        verboseFlags |= ParseControl::VerboseParse;
    if (args.GetBool("VerboseParseShape")) //Settings.VerboseParseShape
        verboseFlags |= ParseControl::VerboseParseShape;
    if (args.GetBool("VerboseParseAction")) //Settings.VerboseParseAction
        verboseFlags |= ParseControl::VerboseParseAction;

    SetFileOpener();

    SF::Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry(SF::GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    mLoader.SetImageFileHandlerRegistry(pimgReg);

#ifdef GFX_AS2_SUPPORT
    Ptr<ASSupport> pAS2Support = *new AS2Support();
    mLoader.SetAS2Support(pAS2Support);
#endif

#ifdef GFX_AS3_SUPPORT
    Ptr<ASSupport> pAS3Support = *new AS3Support();
    mLoader.SetAS3Support(pAS3Support);
#endif

#ifdef SF_OS_MAC
    /*
    // Keep images for renderer changes
    Ptr<ImageCreator> pImageCreator = *new ImageCreator(1);
    mLoader.SetImageCreator(pImageCreator);
    */
#endif

    //This is for GFxPlayer debug/test only, do not enable in your application.
    if (args.GetBool("UseImagePacker"))
    {
        Ptr<ImagePackParams> ppacker = *new ImagePackParams();
        ImagePackParams::TextureConfig tconfig;
        ppacker->GetTextureConfig(&tconfig);
        tconfig.TextureHeight = tconfig.TextureWidth = 1024;
        tconfig.SizeOptions = ImagePackParamsBase::PackSize_PowerOf2;
        ppacker->SetTextureConfig(tconfig);

        mLoader.SetImagePackParams(ppacker);
    }
    // Task Manager.
#ifdef SF_ENABLE_THREADS
    if (args.GetBool("ProgressiveLoading"))
    {
        pTaskManager = *new ThreadedTaskManager;
        #if defined(SF_OS_XBOX360)
            pTaskManager->AddWorkerThreads(Task::Type_IO, 3, 128 * 1024, 1);
        #endif
        mLoader.SetTaskManager(pTaskManager);
    }
#endif
    //mLoader.SetGradientParams(Ptr<GradientParams>(*new GradientParams(16)));

    // Set log, but only if not quiet
    if (!args.GetBool("Quiet")) //!Settings.Quiet
    {
        if (args.GetBool("XmlLog"))
        {
            mLoader.SetLog(Ptr<GFx::Log>(*new GFxPlayerXmlLog()));
        }
        else
        {
            mLoader.SetLog(Ptr<GFx::Log>(*new GFxPlayerLog()));
        }
    }
#if defined (SF_OS_WIN32) && !defined(_DURANGO)
    mLoader.SetClipboard(Ptr<Clipboard>(*new FxPlayerTextClipboard()));
#endif    
    //mLoader.SetTranslator(Ptr<Translator>(*new TranslatorImpl()));

    // Configuring the glyph packer
    //-----------------------------
    //// Creating the packer
    //mLoader.SetFontPackParams(Ptr<FontPackParams>(*new FontPackParams()));
    //
    //// Optional TextureConfig
    ////-----------------------------
    //FontPackParams::TextureConfig fontPackConfig;
    //fontPackConfig.NominalSize = 32;
    //fontPackConfig.PadPixels   = 2;
    //fontPackConfig.TextureWidth  = 512;
    //fontPackConfig.TextureHeight = 512;
    //mLoader.GetFontPackParams()->SetTextureConfig(fontPackConfig);

    //// Optional restriction: If the font contains more than 1000 glyphs it will not be packed.
    //// Instead, this font will be displayed with the dynamic cache.
    ////-----------------------------
    //mLoader.GetFontPackParams()->SetGlyphCountLimit(1000);


    // Image Packer. Enable in Player only for testing; for production use packing in gfxexport.
    //mLoader.SetImagePackParams(Ptr<ImagePackParams>(*new ImagePackParams));

    // Configure Font compactor
    // FontCompactorParams contains the font compactor configuration options 
    // There are two parameters that can be set on the contractor of this class
    // 1. NominalSize - The nominal glyph size that corresponds with the input 
    //    coordinates. Small nominal size results in smaller total data size, 
    //    but less accurate glyphs. For example, NominalSize=256 can typically 
    //    save about 25% of memory, compared with NominalSize=1024, but really 
    //    big glyphs, like 500 pixels may have visual inaccuracy. For typical game 
    //    UI text NominalSize=256 is still good enough. In SWF it is typically 1024.
    // 2. MergeContours - A Boolean flag that tells, whether or not the 
    //    FontCompactor should merge the same contours and glyphs. When merging 
    //    the data can be more compact and save 10-70% of memory, depending on fonts.
    //    But if the font contains too many glyphs, the hash table may consume 
    //    additional memory, 12 (32-bit) or 16 (64-bit) bytes per each unique path, 
    //    plus 12 (32-bit) or 16 (64-bit) bytes per each unique glyph.
    //mLoader.SetFontCompactorParams(Ptr<FontCompactorParams>(*new FontCompactorParams(256, true)));

    SetFontProvider();

    if (FileName.GetLength() == 0)
    {
        FileName = GetFilePath();
        String path = FileName.GetPath();
        if (path.GetLength() == 0)
#if defined(SF_OS_WIIU)
            path = "/vol/content/";
#else
            path = ".";
#endif
        // Directory implementation for X360 and Win32 support multiple extensions. Others do not.
#if defined(SF_OS_XBOX360) || defined(SF_OS_WIN32)
        ContentDir.Reread(path, "*.swf;*.gfx");
#else
        ContentDir.Reread(path, "*.swf");
#endif
    }

    bool loadMovie = strlen(FileName)>0;
    if (loadMovie)
    {
        // if filename is set, then we try to load the font config
        LoadDefaultFontConfigFromPath(FileName);

        // Tell the directory implementation of the current filename.
        ContentDir.SetCurrentFile(FileName);
    }

    mLoader.SetDefaultFontLibName(DEFAULT_FONT_LIB_NAME);

    // Apply font configuration on a loader
    if (FontConfigIndex != -1) 
    {
        if (FontConfigs.GetSize() && FontConfigs[0]->FontLibFiles.GetSize())
            mLoader.SetDefaultFontLibName(FontConfigs[0]->FontLibFiles[0].ToCStr());
        mLoader.SetDefaultFontLibName(DEFAULT_FONT_LIB_NAME);
        FontConfig* pconfig = GetCurrentFontConfig();
        if (pconfig)
            pconfig->Apply(&mLoader);
    } 
    else 
    {
        // Create and load a file into FontLib if requested.
        if (args.HasValue("FontLibFile")) 
        {
            Ptr<FontLib> fontLib = *new FontLib;
            mLoader.SetFontLib(fontLib);
            mLoader.SetDefaultFontLibName(args.GetString("FontLibFile").ToCStr());
            Ptr<MovieDef> pmovieDef = *mLoader.CreateMovie(args.GetString("FontLibFile").ToCStr());
            fontLib->AddFontsFrom(pmovieDef);
        }
    }

    // Load movie for the stats display
    // Loader::LoadWaitCompletion should be set from threaded loading for HUD movie. 
    // Otherwise HUD will no be shown because is Advance method gets call only once when the
    // movie is not ready yet
 
    Ptr<ParseControl> pparseControl = *new ParseControl(verboseFlags);
    mLoader.SetParseControl(pparseControl);

#ifdef GFX_ENABLE_SOUND
    if (DoSound)
        // Initialize sound system(s)
        InitializeSound();
#endif

#ifdef GFX_ENABLE_VIDEO
    // Core video support
#if defined(SF_OS_WIN32) && !defined(_DURANGO)
    // The video player state for Windows has four initialization parameters
    // 1. ActionScript VM support: AS2, AS3 or all
    // 2. Priority of video decoding threads
    // 3. Number of video deciding threads (maximum is 3)
    // 4. An array with affinity masks for each decoding thread
    UInt32 affinityMasks[] = {
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK,
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK,
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK
    };
    Ptr<Video::Video> pvideo = *new Video::VideoPC(Video::VideoVMSupportAll(),
        Thread::NormalPriority, MAX_VIDEO_DECODING_THREADS, affinityMasks);
#elif defined(SF_OS_XBOX360)
    // The video player state for Xbox360 has four initialization parameters: 
    // 1. ActionScript VM support: AS2, AS3 or all
    // 2. Mask which specifies which hardware threads will be used for video decoding
    // 3. Priority of video decoding threads
    // 4. Hardware thread which will be used for the video reader
    Ptr<Video::Video> pvideo = *new Video::VideoXbox360(Video::VideoVMSupportAll(),
        Video::Xbox360_Proc3|Video::Xbox360_Proc4|Video::Xbox360_Proc5, Thread::NormalPriority,
        Video::Xbox360_Proc2);
#elif defined(SF_OS_PS3)
    #ifndef CRIMV_SPU_TRHEADS
    // The video player state (SPURS based) for PS3 has six initialization parameters:
    // 1. ActionScript VM support: AS2, AS3 or all
    // 2. CellSpurs structure pointer
    // 3. Number of PPU threads which can be used for video decoding
    // 4. Priority of PPU threads
    // 5. Number of SPUs which can be used for video decoding
    // 6. SPURS task priorities
    UInt8 spursPrios[] = {0, 0, 1, 1, 1, 1, 0, 0};
    Ptr<Video::Video> pvideo = *new Video::VideoPS3(Video::VideoVMSupportAll(),
        GetSpurs(), 1, Thread::NormalPriority, 4, spursPrios);
    #else
    // SPU threads based PS3 video player state:
    // 2. Number of PPU threads which can be used for video decoding
    // 3. Priority of PPU threads
    // 4. Number of SPUs which can be used for video decoding
    // 5. SPU threads group priority
    Ptr<Video::Video> pvideo = *new Video::VideoPS3(Video::VideoVMSupportAll(),
        1, Thread::NormalPriority, 4, 128);
    #endif
#elif defined(_DURANGO)
    // The video player state for XboxOne has four initialization parameters:
    // 1. ActionScript VM support: AS2, AS3 or all
    // 2. Priority of video decoding threads
    // 3. Number of video deciding threads (maximum is 5)
    // 4. An array with affinity masks for each decoding thread
    DWORD_PTR affinityMasks[] = {
        Video::XboxOneAffinityMask::CPU2,
        Video::XboxOneAffinityMask::CPU3 | Video::XboxOneAffinityMask::CPU4,
        Video::XboxOneAffinityMask::AllCPUs
    };
    Ptr<Video::Video> pvideo = *new Video::VideoXboxOne(Video::VideoVMSupportAll(),
        Thread::NormalPriority, 3, affinityMasks);
#elif defined(SF_OS_ORBIS)
    // The video player state for PS4 has four initialization parameters:
    // 1. ActionScript VM support: AS2, AS3 or all
    // 2. Priority of video decoding threads
    // 3. Number of video deciding threads (maximum is 5)
    // 4. An array with affinity masks for each decoding thread
    SceKernelCpumask affinityMasks[] = {
        Video::PS4AffinityMask::CPU2,
        Video::PS4AffinityMask::CPU3 | Video::PS4AffinityMask::CPU4,
        Video::PS4AffinityMask::AllCPUs
    };
    Ptr<Video::Video> pvideo = *new Video::VideoPS4(Video::VideoVMSupportAll(),
        Thread::NormalPriority, 3, affinityMasks);
#elif defined(SF_OS_WIIU)
    // The video player state for Wii U has four initialization parameters:
    // 1. ActionScript VM support: AS2, AS3 or all
    // 2. Priority of video decoding threads
    // 3. Number of video deciding threads (maximum is 2)
    // 4. An array with affinity masks for each decoding thread
    u16 affinityMasks[] = {
        OS_THREAD_ATTR_AFFINITY_CORE1,
        OS_THREAD_ATTR_AFFINITY_CORE2
    };
    Ptr<Video::Video> pvideo = *new Video::VideoWiiU(Video::VideoVMSupportAll(),
        Thread::NormalPriority, 2, affinityMasks);
#else
    // 1. ActionScript VM support: AS2, AS3 or all
    // 2. Priority of the video decoding thread
    Ptr<Video::Video> pvideo = *new Video::Video(Video::VideoVMSupportAll(),
        Thread::NormalPriority);
#endif

    // Video system sound interfaces
#ifdef GFX_VIDEO_USE_SYSTEM_SOUND
#if   defined(SF_OS_WIN32)  && !defined(_DURANGO)
    pvideo->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemDX8(0)));
#elif defined(SF_OS_MAC)
    pvideo->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemMac()));
#elif defined(SF_OS_XBOX360) || defined(_DURANGO)
    pvideo->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemXA2(0,0)));
#elif defined(SF_OS_PS3)
    pvideo->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemPS3(true, GetSpurs())));
#elif defined(SF_OS_WII)
    pvideo->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemWii(true)));
#elif defined(SF_OS_WIIU)
    pvideo->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemWiiU(true)));
#endif
    // FMOD & Wwise system sound interfaces
#elif defined(GFX_VIDEO_USE_FMOD)
    pvideo->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemFMOD(pSoundFMODSystem->GetFMOD())));
#elif defined(GFX_VIDEO_USE_WWISE)
    pWwise = Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemWwise());
    pvideo->SetSoundSystem(pWwise);
#endif // GFX_VIDEO_USE_SYSTEM_SOUND

    mLoader.SetVideo(pvideo);
#endif // GFX_ENABLE_VIDEO

#ifdef GFX_ENABLE_SOUND
    Sound::SoundRenderer* psoundRenderer = GetSoundRenderer();
    if (psoundRenderer)
    {
        // Sound renderer and parameters for playing SWF streaming sounds are provided to
        // to the loader by GFx::Audio. Those parameters control when and how the synchronization
        // between SWF frames and sound frames should be done.
        Ptr<Audio> paudio = *new Audio(psoundRenderer);
        mLoader.SetAudio(paudio);
        psoundRenderer->SetMasterVolume(SoundVolume);
        psoundRenderer->Mute(IsMute);

#if defined(GFX_ENABLE_VIDEO) && !defined(GFX_VIDEO_USE_SYSTEM_SOUND) && \
                                 !defined(GFX_VIDEO_USE_FMOD) && !defined(GFX_VIDEO_USE_WWISE)
        // if a video has audio tracks and you want to play them through SoundRenderer 
        // object you need to set this object to Video::Video state
        pvideo->SetSoundSystem(psoundRenderer);
#endif
    }
#endif // GFX_ENABLE_SOUND

    // 
    // SharedObject support
    //
#if defined(GFX_AS2_ENABLE_SHAREDOBJECT) && defined(GFX_ENABLE_XML)

    String SOPath;
    SOPath.AppendString(FileName);
    URLBuilder::ExtractFilePath(&SOPath);
    if (SOPath.CompareNoCase(FileName) == 0)
    {
        SOPath = "";
    } 
    // GFx currently does not support creation of directories
    //SOPath.AppendString(".soCache/");
    Ptr<SharedObjectManagerBase> psomgr = *new FxSharedObjectManager(SOPath);
    mLoader.SetSharedObjectManager(psomgr);

#endif  // GFX_AS2_ENABLE_SHAREDOBJECT


#ifdef GFX_ENABLE_XML

    Ptr<XML::Parser> pexpatXmlParser = *new XML::ParserExpat;
    Ptr<XML::SupportBase> pxmlSupport = *new XML::Support(pexpatXmlParser);
    mLoader.SetXMLSupport(pxmlSupport);

    //
    // Using the default parser for document processing from C++
    // ---------------------------------------------------------
    // (Not available in eval/lib distributions)
    //
    //#include "GFx/XML/XML_DOM.h"
    //
    // -- Create a DOM builder that processes whitespace
    //XML::DOMBuilder domBuilder(pxmlSupport);
    // -- Create a DOM builder that ignores whitespace
    //XML::DOMBuilder domBuilder2(pxmlSupport, true);
    //
    // -- Process the xml file and return the root of the DOM tree
    //Ptr<XML::Document> pdoc = domBuilder.ParseFile("inputfile.xml", mLoader.GetFileOpener());
    // -- Process the xml file and return the root of the DOM tree (use provided object manager)
    //Ptr<XML::ObjectManager> pobjMgr = *new XML::ObjectManager();
    //Ptr<XML::Document> pdoc2 = domBuilder.ParseFile("inputfile.xml", mLoader.GetFileOpener(), pobjMgr);
    // 

#endif

    if (loadMovie)
    {
        // Get info about the width & height of the movie.
#ifdef GFX_ENABLE_VIDEO
        if (IsVideoFile(FileName))
        {
            Video::VideoPlayer::VideoInfo vinfo;
            if (Video::VideoPlayer::LoadVideoInfo(FileName, &vinfo, mLoader.GetFileOpener()))
            {
                MViewSize.Width  = (int)(vinfo.Width  * ScaleX);
                MViewSize.Height = (int)(vinfo.Height * ScaleY);
            }
            else 
            {
                if (args.GetBool("Quiet"))
                    fprintf(stderr, "Error: Failed to get USM file info: '%s'\n", FileName.ToCStr());
                loadMovie = 0;
            }
        }
        else
#endif
        {
        if (!mLoader.GetMovieInfo(FileName, &mMovieInfo)) 
        {
            // mLoader.GetMovieInfo will print error message unless we are in quiet mode
            if (args.GetBool("Quiet"))
                fprintf(stderr, "Error: Failed to get info about '%s'\n", FileName.ToCStr());
            loadMovie = 0;
        } 
        else
        {
            MViewSize.Width  = (int)(mMovieInfo.Width  * ScaleX);
            MViewSize.Height = (int)(mMovieInfo.Height * ScaleY);
        }
        }
    }
    if (!loadMovie)
    {
        MViewSize = Platform::Device::GetDefaultViewSize();
    }

    // Add our custom commands to the autotest object.
    if (AutoTest)
    {
        AutoTest->AddCommand(SF_NEW FxPlayerCommandChangeRes());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandScaledDisplay());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandWireframe());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandAntialiasingMode());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandFullscreen());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandFastMode());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandFastForward());
        AutoTest->AddCommand(SF_NEW FxPlayerPadPassThrough());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandVSync());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandPause());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandFontConfig());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandObjectsReport());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandRestart());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandStageClipping());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandStageCulling());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandResetUserMatrix());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandTriangleOptimization());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandStrokeMode());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandToggleProfileMode<Render::Profile_Overdraw>());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandToggleProfileMode<Render::Profile_Batch>());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandChangeHighlightBatch<-1>);
        AutoTest->AddCommand(SF_NEW FxPlayerCommandChangeHighlightBatch<1>);

        AutoTest->AddCommand(SF_NEW FxPlayerCommandCycleProfile());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandToggleProfilerFlag<Render::ProfileFlag_NoBlendCaching>());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandToggleProfilerFlag<Render::ProfileFlag_NoFilterCaching>());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandToggleProfilerFlag<Render::ProfileFlag_NoBatching>());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandToggleProfilerFlag<Render::ProfileFlag_NoInstancing>());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandMemReport());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandFunctionTiming());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandMute());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandVolumeUp());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandVolumeDown());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandStepForward_1());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandStepForward_10());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandStepBack_1());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandStepBack_10());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandBackground());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandBackgroundColor());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandCurveToleranceUp());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandCurveToleranceDown());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandLoadPrevFile());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandLoadNextFile());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandQuit());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandCtrlKey());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandEnter());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandEscape());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandShowMouseCursor());
        AutoTest->AddCommand(SF_NEW FxPlayerCommandTakeScreenshot());
    }

    if (AutoTest && !AutoTest->OnInit(config))
        return false;

    if (Rendering) 
    {
        bool           fsaa = (AAMode == AAMode_FSAA) ? true : false;
        Size<unsigned> defSize = Platform::Device::GetDefaultViewSize();

        if (!ArgResolution.IsNull()) 
            MViewSize = ArgResolution;

        if (IsConsole())
        {
            config.ViewFlags |= Platform::View_FullScreen;
            ScaleX = ((float)defSize.Width)  / MViewSize.Width;
            ScaleY = ((float)defSize.Height) / MViewSize.Height;
            MViewSize = defSize;
        }
        if (fsaa)
            config.ViewFlags |= Platform::View_FSAA;
        // VSync...

        Size<unsigned> appliedSize = MViewSize;
        appliedSize.Height += FXPLAYER_LOGO_BORDER;
        config.ViewSize = appliedSize;
        config.ViewFlags |= Platform::View_WindowResizable|Platform::View_WindowDropFiles;
        
        config.MinSize = MinSize;
        config.SetFlag(Platform::View_PrecompileShaders, args.GetBool("PrecompileShaders"));      

        // Create the window and RenderThread.
        if (!OnInitHelper(config, GetAppTitle()))
            return false;

        // It is important to initialize these sizes, in case OnSizeEnter gets called.
        SizingSize = GetViewSize();
        // Size might be changed by OnInitHelper call above.
        if (GetViewSize() != appliedSize)
            UpdateViewSize();
        GetRenderThread()->SetBackgroundColor(BackgroundColor);

#ifdef GFX_ENABLE_VIDEO
        Render::TextureManager* ptexMan = GetRenderThread()->GetTextureManager();
        if (pvideo && ptexMan)
            pvideo->SetTextureManager(ptexMan);
#endif
    }


    Ptr<GFx::IMEManagerBase> pimemanager = 0;
    if (IsImeEnabled())
    {
#if defined(SF_ENABLE_IME) && defined(SF_ENABLE_IME_WIN32)
        //!Create IMEManager after the application window has been set up since 
        
    //  pimemanager = InitIME();
        pimemanager = *new GFx::IME::GFxIMEManagerWin32((HWND)GetDeviceWindow()->GetHandle());
        if (pimemanager)
        {
            // For error logging. If error logging not desired, just pass null to Init. User must 
            // call Init though!
            /*  Parameters:
            1- Logger used to log IME messages. If not logging is desired, pass NULL
            2- Fileopener necessary to open the ime.xml file that contains the list of supported IME's. If NULL, a default
                list will be used instead
            3- Path for the ime.xml file that contains a list of supported IMEs
            4- Used to set if while loading the candidate list swf, IMEManager should check if the file exists in 
               current directory (where the parent swf is located) or not. If false, IMEManager will not check
               if the file exists, and if the file is eventually not found, an error message will be printed.
               This is useful if you have you own file management system and you want FileOpener::OpenFile to be
               called regardless. 
            */  
            pimemanager->Init(NULL /*mLoader.GetLog()*/, mLoader.GetFileOpener(), "ime.xml", true);
            pimemanager->SetIMEMoviePath("IME.swf");
            mLoader.SetIMEManager(pimemanager);
            // mLoader keeps the object from this point
        //    pimemanager->Release();
        }
#endif
    }



#ifdef SF_AMP_SERVER
    GFx::AMP::MessageAppControl caps;
	GetAmpServerCapabilities(caps);
    AmpServer::GetInstance().SetAppControlCaps(&caps);
    AmpServer::GetInstance().SetAppControlCallback(this);
    if (args.GetBool("AmpWait"))
    {
        AmpServer::GetInstance().SetConnectionWaitTime(SF_WAIT_INFINITE);
    }
    if (args.GetInt("AmpProfileLevel") >= 0)
    {
        AmpServer::GetInstance().SetProfileLevel(static_cast<AmpProfileLevel>(args.GetInt("AmpProfileLevel")), true);
    }
    if (args.GetBool("AmpMemDetail"))
    {
        AmpServer::GetInstance().SetMemReports(true, true);
    }
    AmpServer::GetInstance().SetListeningPort(AmpPort);
    AmpDirty = true;

    AmpServer::GetInstance().OpenConnection();
#endif

    // Load movie and initialize timing.
    if (!SkipLoadMovie && loadMovie && !LoadMovie(FileName)) 
    {
        //return 1;
    }

    // Animate as fast as possible.
    AdjustFrameTime();    
    AccumClockTicks = 0;
    PrevTicks = Timer::GetTicks();


    if (IsImeEnabled())
    {
#if defined(SF_ENABLE_IME) && defined(SF_ENABLE_IME_WIN32)
        if (pimemanager)
        {
            if(pMovie)
                pMovie->HandleEvent(GFx::SetFocusEvent());
        }
#endif
    }

	
#ifdef GFX_GESTURE_RECOGNIZE
    GestureRecognizer *gestureRecognizer = new GestureRecognizer(*this);
    gestureRecognizer->setup();
	pGestureManager = gestureRecognizer;
	SF_DEBUG_MESSAGE(1, "Gesture recognizer setup\n");
#endif
	

    return true;
}

#ifdef SF_AMP_SERVER
void			FxPlayerAppBase::GetAmpServerCapabilities(GFx::AMP::MessageAppControl &caps) const
{
    caps.SetCurveToleranceDown(true);
    caps.SetCurveToleranceUp(true);
    caps.SetNextFont(true);
    caps.SetRestartMovie(true);
    caps.SetToggleAaMode(true);
    caps.SetToggleAmpRecording(true);
    caps.SetToggleFastForward(true);
    caps.SetToggleInstructionProfile(true);
    caps.SetToggleOverdraw(true);
    caps.SetToggleBatch(true);
    caps.SetToggleBlending(true);
    caps.SetToggleTextureDensity(true);
    caps.SetToggleStrokeType(false);
    caps.SetTogglePause(true);
    caps.SetToggleWireframe(true);
}
#endif

void FxPlayerAppBase::OnRenderThreadCreated()
{
    GlyphCacheParams gcParams;
    //gcParams.NumTextures = 0; // Disable dynamic raster cache.
    GetRenderThread()->SetGlyphCacheParams(gcParams);

    ToleranceParams tolParams;
    //tolParams.FillLowerScale = 0.5f; // Set thresholds for tessellation
    //tolParams.FillUpperScale = 2.0f;
    GetRenderThread()->SetToleranceParams(tolParams);
}

void FxPlayerAppBase::OnShutdown()
{
#ifdef SF_AMP_SERVER
    AmpServer::GetInstance().CloseConnection();
#endif

    pMovie = 0;
    pMovieDef = 0;

    // If we started worker threads in the renderer we must end them here;
    // otherwise FinishAllThreads will lock up waiting indefinitely.
#ifdef SF_ENABLE_THREADS
    if (pTaskManager)
    {
        pTaskManager->RequestShutdown();    
    }
#endif
#ifdef GFX_ENABLE_SOUND
    ShutdownSound();
#endif

    delete AutoTest;
    AutoTest = 0;
    BaseClass::OnShutdown();
}

void FxPlayerAppBase::AdjustFrameTime()
{
    bool zeroFrameTime = false;

    if (Rendering)
    {
        if (MeasurePerformance)
            zeroFrameTime = true;
        if (!Paused &&
            (FastForward || ViewConfigHasFlag(Platform::View_VSync)))
            zeroFrameTime = true;
    }
    else
    {
        zeroFrameTime = true;
    }

    if (zeroFrameTime)
        SetFrameTime(0.0f);
    else
        SetFrameTime(1.0f / (pMovie ? mMovieInfo.FPS : 30.0f));    
}


void FxPlayerAppBase::SetMovieVariables(String& argString)
{
    // This should only be true if this is the Player application
    // Make sure to comment this out or set the value to false in your game
    if(pMovie->GetAVMVersion() == 1)
    {
        pMovie->SetVariable("_global.gfxPlayer", Value(true));

#ifdef GFX_ENABLE_SOUND
        Value _global, soundFunc;
        pMovie->GetVariable(&_global, "_global");
        Ptr<GFxSoundEventCallback> soundEventCallback = *new GFxSoundEventCallback(pMovie);
        pMovie->CreateFunction(&soundFunc, soundEventCallback);            
        _global.SetMember("gfxProcessSound", soundFunc);
#endif // GFX_ENABLE_SOUND


        // Pass command line argument to ActionScript if necessary.
        if (GetArgs().HasValue("Arg") && argString.IsEmpty())
            argString = GetArgs()["Arg"]->GetString();
        if (!argString.IsEmpty())
            pMovie->SetVariable("_global.gfxArg", argString.ToCStr());

        const char* language = (FontConfigIndex == -1) ? "Default"
            : FontConfigs[FontConfigIndex]->ConfigName.ToCStr();
        pMovie->SetVariable("_global.gfxLanguage", Value(language));
    }
    else if (pMovie->GetAVMVersion() == 2)
    {
        GFx::Value trueVal(true);
        GFx::Value arg("scaleform.gfx.Extensions");
        GFx::Value extensionsClass;

        pMovie->Invoke("flash.utils.getDefinitionByName", &extensionsClass, &arg, 1);
        SF_ASSERT(extensionsClass.IsObject());

        extensionsClass.SetMember("isGFxPlayer", trueVal);
    }
}

void FxPlayerAppBase::OnConfigurationChange(const Platform::ViewConfig& config)
{
    SF_UNUSED1(config);
    pRenderThread->UpdateConfiguration();
}


bool FxPlayerAppBase::ValidateDeviceStatus()
{
    if (!Rendering)
        return true;
    
    // Handle D3D9 lost devices.
    bool                   deviceReady = true;
    Platform::DeviceStatus deviceStatus = pRenderThread->GetDeviceStatus();

    if (deviceStatus != Platform::Device_Ready)
    {
        if (deviceStatus == Platform::Device_NeedReconfigure)
        {
            Platform::ViewConfig config;
            GetViewConfig(&config);
            // ReconfigureGraphics is a synchronizing command, so it'll
            // update the status.
            ReconfigureGraphics(config);
        }
        else
        {   
            if (GetFrameTime() != 0.0f)
                Thread::MSleep(10);
            pRenderThread->UpdateDeviceStatus();
            deviceReady = false;
        }
    }

    return deviceReady;
}


bool FxPlayerAppBase::AdvanceMovie(float deltaT)
{
    // Potential out-of bounds range is not a problem here,
    // because it will be adjusted for inside of the player.
    if (pMovie) 
    {
        UInt64 startAdvanceTicks = Timer::GetProfileTicks();

        Size<unsigned> vsz = GetViewSize();

        // It's important to case size difference to int, as otherwise it will yield
        // large numbers instead of negative ones (which are allowed).
        pMovie->SetViewport((int)vsz.Width, (int)vsz.Height,
                            ((int)vsz.Width-(int)MViewSize.Width)/2, 
                            ((int)vsz.Height-(int)MViewSize.Height)/2,
                            (int)MViewSize.Width, (int)MViewSize.Height);
        pMovie->SetBackgroundAlpha(Background ? 1.0f : 0.0f);
    
        float timeTillNextFrame;
        {
            ScopedAutoTestTimerReplacement autoTimer(this);
            timeTillNextFrame = pMovie->Advance(deltaT, 0);
        }
        if (GetFrameTime() != 0.0f ) // Fast forward or Measure Performance modes
            SetFrameTime(Alg::Min( 1.0f / mMovieInfo.FPS, timeTillNextFrame));

        LastAdvanceTicks = Timer::GetProfileTicks() - startAdvanceTicks;

        // If we're reached the end of the movie, exit.
        if (pMovie->IsExitRequested() ||
            (PlayOnce && (pMovie->GetCurrentFrame() + 1 == pMovieDef->GetFrameCount())))
            return false;
    }
    else
    {
        LastAdvanceTicks = 0;
    }
    return true;
}


void FxPlayerAppBase::OnUpdateFrame(bool needRepaint)
{
    // On the first OnUpdateFrame, start autotest playback (if requested).
    if ( AutoTest && AutoTest->IsPlayback() )
    {
        OnAutoUpdateFrame();
        return;
    }

    // TBD: Make use of 'needRepaint' once refresh is conditional on change.
    SF_UNUSED(needRepaint);

    SF_AMP_CODE(HandleAmpAppMessages());
   
    // Update timing.
    UInt64 timer = Timer::GetTicks();
    UInt64 delta = timer - PrevTicks;
    PrevTicks = timer;
    AccumClockTicks += delta;

    if (Paused && Rendering)
        delta = 0;
    else if (FastForward || !Rendering)
        delta = (UInt64)(1000000.0f / mMovieInfo.FPS);

    // Check to see if exit timeout was hit.
    if ((ExitTimeout > 0.0f) && (AccumClockTicks >= (ExitTimeout * 1000.0f)))
    {
        Shutdown();
        return;
    }

    // Update (software) cursors.
    Cursor.UpdateCursor(GetMousePos(), pRenderThread );

#ifdef GFX_VIDEO_USE_WWISE
    pWwise->Update();
#endif

    // Record the advance.
    if (AutoTest)
        AutoTest->OnAdvanceMovie((float)delta / 1000000.f);

    if (!AdvanceMovie((float)delta / 1000000.f))
    {
        // If movie signaled exit, we are done.
        Shutdown();
        return;
    }

    if ((!ValidateDeviceStatus() || !Rendering ) && pMovie )
    {
        // If the device is in an invalid state we still must ensure NextCapture gets 
        // called on display handles. Otherwise capture change data will build up indefinitely.
        MovieDisplayHandle handle = pMovie->GetDisplayHandle();
        handle.NextCapture(pRenderThread ? pRenderThread->GetRenderer2D()->GetContextNotify() : 0 );

        // Need to 'trick' the context into thinking that a frame has ended, so that NextCapture will
        // process the snapshots correctly. Otherwise, snapshot heaps will build up while the device is lost.
        if ( pRenderThread && pRenderThread->GetRenderer2D()->GetContextNotify())
            pRenderThread->GetRenderer2D()->GetContextNotify()->EndFrameContextNotify();

        // Make sure to tick AMP, otherwise the messages will build up.
        SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame();)
        return;
    }

    // Capture is done by Advance.
    pRenderThread->WaitForOutstandingDrawFrame();
    pRenderThread->DrawFrame();

    // Update timing
    FrameCounter++;    
    if ((LastFPSUpdatedTicks + 1000000) <= AccumClockTicks)
    {
        LastFPS = 1000000.f * FrameCounter / (float)(AccumClockTicks - LastFPSUpdatedTicks);
        AccumFPS += LastFPS;
        LastFPSUpdatedTicks = AccumClockTicks;
        AccumFPSSecondCount++;
        FrameCounter = 0;
        //if (pRenderThread)
       //     LastDisplayFrames = pRenderThread->GetFrames();
            UpdateFpsDisplay();
    }
    
    // Update stats more frequently; each 1/5 of a second.
    if ((LastStatUpdatedTicks + 1000000/5) <= AccumClockTicks)
    {
        LastStatUpdatedTicks = AccumClockTicks;
        UpdateStatReports();
    }
}

void FxPlayerAppBase::OnAutoUpdateFrame()
{
    // Perform the playback.
    AutoTest->PerformPlayback();

    // Stick an Advance 0 at the end - if the tests didn't contain any advances, log output might not come out.
    pMovie->Advance(0.0f, 0);

    // Wait for the last frame. Exiting immediately may interfere with screenshots.
    if (pRenderThread)
        pRenderThread->WaitForOutstandingDrawFrame();

    // Shutdown
    Shutdown();
}

// Called every second
void FxPlayerAppBase::UpdateFpsDisplay()
{
    if (GetArgs().GetBool("LogFPS"))
    {
        float avgFPS = AccumFPS / AccumFPSSecondCount;
        printf("FPS: %6.2f, AVG FPS: %6.2f\n", LastFPS, avgFPS);
    }
}


FxPlayerCommand* FxPlayerAppBase::KeyToCommand(UInt32 key)
{
    Hash< UInt32, Ptr<FxPlayerCommand> >::Iterator it = KeyCommandMap.Find(key);
    if (it != KeyCommandMap.End())
    {
        return it->Second;
    }
    return NULL;
}

FxPlayerCommand* FxPlayerAppBase::PadKeyToCommand(PadKeyCode key)
{
    Hash< UInt32, Ptr<FxPlayerCommand> >::Iterator it = KeyCommandMap.Find(key);
    if (it != KeyCommandMap.End())
    {
        return it->Second;
    }
    return NULL;
}


// An example of ExternalInterface handler implementation
class CustomEIHandler : public ExternalInterface
{
public:
    void Callback(Movie* pmovieView, const char* methodName, const Value* args, unsigned argCount)
    {
        Ptr<GFx::Log> pLog = pmovieView->GetLog();
        pLog->LogMessage("\nCallback! %s, nargs = %d\n", (methodName)?methodName:"(null)", argCount);
        for (unsigned i = 0; i < argCount; ++i)
        {
            pLog->LogMessage("  arg(%d) = ", i);
            switch(args[i].GetType())
            {
            case Value::VT_String:	pLog->LogMessage("%s", args[i].GetString()); break;
            case Value::VT_Number:	pLog->LogMessage("%f", static_cast<double>(args[i].GetNumber())); break;
			case Value::VT_Int:		pLog->LogMessage("%d", args[i].GetInt()); break;
			case Value::VT_UInt:	pLog->LogMessage("%u", args[i].GetUInt()); break;
            case Value::VT_Boolean: pLog->LogMessage("%s", (args[i].GetBool())?"true":"false"); break;
                // etc...
            default:
                break;
            }
            pLog->LogMessage("\n");
        }
    }
};


// Default implementation of UrlNavigator
class FxPlayerUrlNavigator : public UrlNavigator
{
public:
    FxPlayerUrlNavigator(FxPlayerAppBase* papp) : pApp(papp) {}
    void NavigateToUrl(const String& url)
    {
        pApp->GetAppImpl()->ProcessUrl(url);
    }
protected:
    FxPlayerAppBase*    pApp;
};

// Load a new movie from a file and initialize timing
bool FxPlayerAppBase::LoadMovie( const String& filename)
{
    String filenameStr = filename;
    String argString;

#ifdef GFX_ENABLE_VIDEO
    if (IsVideoFile(filenameStr))
    {
        // Video USM file to SWF file as an argument.
        argString = filenameStr;
        filenameStr = "_internal_video_player";
        ClippingEnable = false;
    }
#endif

    // Try to load the new movie
    Ptr<MovieDef> pnewMovieDef;
    Ptr<Movie> pnewMovie;
    MovieInfo newMovieInfo;

    MovieCursorType = Platform::Cursor_Arrow;
    SetCursor(MovieCursorType);
    ResetUserMatrix();

    // Get info about the width & height of the movie.
    if (!mLoader.GetMovieInfo(filenameStr, &newMovieInfo, 0, Loader::LoadKeepBindData)) 
    {
        fprintf(stderr, "Error: Failed to get info about %s\n", filenameStr.ToCStr());
        return 0;
    }

    unsigned loadConstants = Loader::LoadAll;
    const Platform::Args& args = GetArgs();
    /*
    Ptr<ImageCreator> imageCreator = *new CustomImageCreator;
    mLoader.SetImageCreator(imageCreator);
    */

    // Set ActionScript verbosity / etc.
    Ptr<ActionControl> pactionControl = *new ActionControl();
    pactionControl->SetVerboseAction(args["VerboseActions"]->GetBool());
#ifdef GFC_NO_FXPLAYER_VERBOSE_ACTION
    if (args["VerboseActions"]->GetBool())
        fprintf(stderr, "VerboseAction is disabled by the GFC_NO_FXPLAYER_VERBOSE_ACTION macro in GFxConfig.h\n");
#endif

    pactionControl->SetActionErrorSuppress(args["SuppressASErrors"]->GetBool());
    pactionControl->SetLogRootFilenames(args["LogRootFilenames"]->GetBool());
    pactionControl->SetLogChildFilenames(!args["NoLogChildFilnames"]->GetBool());
    pactionControl->SetLongFilenames(args["LogFilePath"]->GetBool());


    // For D3D, it is good to override image creator to keep image data,
    // so that it can be restored in case of a lost device.
    if (!mLoader.GetImageCreator())
    {
        SF::Ptr<GFx::ImageCreator> pimageCreator = 
            *new GFx::ImageCreator(pRenderThread->GetTextureManager());
        mLoader.SetImageCreator(pimageCreator);
    }

    // Load the actual new movie and create instance.
    // Don't use library: this will ensure that the memory is released.
    pnewMovieDef = *mLoader.CreateMovie(filenameStr, loadConstants
//                                                                |Loader::LoadKeepBindData
                                                                |Loader::LoadWaitFrame1);
    if (!pnewMovieDef) 
    {
        fprintf(stderr, "Error: Failed to create a movie from '%s'\n", filenameStr.ToCStr());
        return 0;
    }

    // Tell the directory implementation of the  filename.
    ContentDir.SetCurrentFile(filenameStr);

    pnewMovie = *pnewMovieDef->CreateInstance(MemoryParams(), false, pactionControl, pRenderThread);
    if (!pnewMovie) 
    {
        fprintf(stderr, "Error: Failed to create movie instance\n");
        return 0;
    }

    // If this succeeded, replace the old movie with the new one.
    pMovieDef = pnewMovieDef;
    pMovie = pnewMovie;
    memcpy(&mMovieInfo, &newMovieInfo, sizeof(MovieInfo));

    FileName = filenameStr;

    if (pRenderThread)
    {
        pRenderThread->AddDisplayHandle(pMovie->GetDisplayHandle());
        pRenderThread->ResetRasterizationCount();
    }
    // Release cached memory used by previous file.
//     if (pMovie->GetMeshCacheManager())
//         pMovie->GetMeshCacheManager()->ClearCache();

    // Set a reference to the app
    pMovie->SetUserData(this);

    if (IsCursorEnabled())    
        pMovie->SetMouseCursorCount(GetMouseCount());    
    pMovie->SetControllerCount(Alg::Max(GetKeyboardCount(), GetMouseCount()));

    SetMovieVariables(argString);

    InstallHandlers();

    //!AB, set active movie..this causes OnMovieFocus to get called on the IMEManager
    pMovie->HandleEvent(GFx::SetFocusEvent());

    // init first frame
    {
        ScopedAutoTestTimerReplacement autoTimer(this);
        if (AutoTest && !AutoTest->IsPlayback())
        {
            AutoTest->OnAdvanceMovie(0);
            pMovie->Advance(0.0f, 0);
        }
    }

    // Renderer
    if (Rendering) 
    {
        pMovie->SetEdgeAAMode((AAMode == AAMode_EdgeAA) ?
                              Render::EdgeAA_On : Render::EdgeAA_Disable);
    }

    if (DoSound) 
    { 
        // No built-in sound support currently in GFx. Customers
        // can play their own sound through fscommand() callbacks.
    }

    // Disable pause.
    Paused = false;
    AdjustFrameTime();    
    AccumClockTicks = 0;
    LastFPSUpdatedTicks = 0;
    LastStatUpdatedTicks = 0;
    PrevTicks = Timer::GetTicks();

    // Reset FPS counters used for FPS averaging.
    FrameCounter = 0;    
    LastFPS = 0.0f;
    AccumFPS = 0.0f;
    AccumFPSSecondCount = 0;

    // If you are using batch highlighting, reset the counter.
    BatchHighlight = -1;
    GetRenderThread()->SetBatchHighlight(BatchHighlight);

    // Update the view
    UpdateViewSize();
    AmpDirty = true;

    if(pGestureManager)
	{
		pGestureManager->SetMovie(pMovie);
		SF_DEBUG_MESSAGE(1, "Gesture recognizer setmovie\n");
	}

    return 1;
}

// Called when sizing begins and ends.
void FxPlayerAppBase::OnSizeEnter(bool enterSize) 
{
    // When leaving size, adjust to new width/height.
    if (!enterSize) 
    {
        SizingEntered = 0;
        
        // If the size is not the same as the current view size, then actually do the resize. Note that,
        // this requires that GetViewSize (which calls updateConfig internally), returns the window size before
        // the resize occurs. This may require usage of different resize callbacks from the OS, for example using
        // windowWillResize as opposed to windowDidResize (on OSX).
        if (SizingSize != GetViewSize())
        {
            Platform::ViewConfig cfg;
            GetViewConfig(&cfg);
            cfg.ViewSize = SizingSize;
            cfg.ViewSize.Height += FXPLAYER_LOGO_BORDER;
            ReconfigureGraphics(cfg);
            UpdateViewSize();
        }
    } 
    else 
    {
        // Set SizingEntered flag so that we can differentiate size-grip
        // resize from instantaneous resize (maximize).
        SizingEntered = 1;        
        SizingSize = GetViewSize();
    }
}

// Sizing; by default, re-initalizes the renderer
void FxPlayerAppBase::OnSize(const Size<unsigned>& newSize) 
{
    SizingSize = newSize;
    if (!SizingEntered) 
    {
        // Commit sizing immediately if it was due to maximize.
        OnSizeEnter(0);
    } 
    else
    {
        if (!ViewConfigHasFlag(Platform::View_FullScreen))
            pRenderThread->PresentOnResize();
    }
}


#ifdef FXPLAYER_LOGO
Size<unsigned>  FxPlayerAppBase::GetViewSize() const
{
    Size<unsigned> winSize = AppBase::GetViewSize();
    winSize.Height -= FXPLAYER_LOGO_BORDER;
    return winSize;
}
#endif

// Updates the view size based on the ScaleEnable flag and window size.
void FxPlayerAppBase::UpdateViewSize() 
{
    if (ScaleEnable) 
    {
        int width = (int)Alg::Max<unsigned>(GetViewSize().Width, 4);
        int height= (int)Alg::Max<unsigned>(GetViewSize().Height, 4);

        if (ClippingEnable) 
        {
            // Determine movie size and location based on the aspect ratio
            float hw;
            if (mMovieInfo.Height != 0 && mMovieInfo.Width != 0)
                hw = (float) mMovieInfo.Height / (float) mMovieInfo.Width;
            else
                hw = (float) height / (float) width;
            if (width * hw > height) 
            {
                // Use height as the basis for aspect ratio
                MViewSize.Width = Alg::IRound((float) height / hw);
                MViewSize.Height = height;
            } 
            else 
            {
                // Use width
                MViewSize.Width = width;
                MViewSize.Height = Alg::IRound(width * hw);
            }
        } 
        else 
        {
            MViewSize.Width = width;
            MViewSize.Height = height;
        }

        // Compute input scale
        if ( mMovieInfo.Width != 0)
            ScaleX = (float) MViewSize.Width / (float) mMovieInfo.Width;
        else
            ScaleX = 1.0f;
        if ( mMovieInfo.Height != 0)
            ScaleY = (float) MViewSize.Height / (float) mMovieInfo.Height;
        else
            ScaleY = 1.0f;
    } 
    else 
    {
        // No scaling, just center the image
        MViewSize.Width = mMovieInfo.Width;
        MViewSize.Height = mMovieInfo.Height;
        ScaleX = ScaleY = 1.0f;
    }

   // MA TBD:
   // if (pRenderThread)
   //     pRenderThread->RecreateRenderer2D();
}



void FxPlayerAppBase::ResetUserMatrix() 
{
    Move = PointF(0.0f);
    Zoom = 1.0f;
    UpdateUserMatrix();
}

void FxPlayerAppBase::UpdateUserMatrix() 
{
    if (!pRenderThread)
        return;
    UserMatrix = Render::Matrix2F::Scaling(Zoom);
    UserMatrix.AppendTranslation(Move.x, Move.y);

    //  Render::Matrix2F m3d;
    //  m3d.AppendScaling(Zoom);
    //  m3d.AppendTranslation(Move.x * 2.f / MViewSize.Width, -Move.y * 2.f / MViewSize.Height);
    pRenderThread->SetUserMatrix(UserMatrix);    
}

//SizeF FxPlayerAppBase::GetMovieScaleSize() 
//{
//    SizeF scale;
//    scale.Width = (pMovie->GetVisibleFrameRect().Width() / ViewWidth);
//    scale.Height = (pMovie->GetVisibleFrameRect().Height() / ViewHeight);
//    return scale;
//}

#ifdef SF_AMP_SERVER
bool FxPlayerAppBase::HandleAmpRequest(const GFx::AMP::MessageAppControl* message)
{
    Lock::Locker locker(&AmpCallbackLock);
    AmpAppControlMsgs.PushBack(const_cast<GFx::AMP::MessageAppControl*>(message));
    return true;
}
#endif


void FxPlayerAppBase::SetRendererProfiling(bool on)
{
    SF_UNUSED(on);
}

bool FxPlayerAppBase::TakeScreenShot( const String& filename )
{
    pRenderThread->WaitForOutstandingDrawFrame();
    pRenderThread->DrawFrame();
    bool screenShot = pRenderThread->TakeScreenShot(filename);
    pRenderThread->DrawFrame();
    return screenShot;
}

void FxPlayerAppBase::ToggleAaMode()
{
    bool renderChange = false, edgeChange = false;
    
    switch(AAMode)
    {
    case AAMode_None:
        AAMode = AAMode_EdgeAA;
        edgeChange = 1;
        break;
    case AAMode_EdgeAA:
        AAMode = AAMode_FSAA;
        edgeChange = renderChange = 1;
        break;
    case AAMode_FSAA:
        AAMode = AAMode_None;
        renderChange = 1;
        break;
    }

    if (renderChange) 
    {
        // FSAA toggle.
        bool fsaa = (AAMode == AAMode_FSAA);
        Platform::ViewConfig config;        
        GetViewConfig(&config);

        if (fsaa != config.HasFlag(Platform::View_FSAA))
        {
            config.SetFlag(Platform::View_FSAA, fsaa);
            ReconfigureGraphics(config);
        }
    }
    if (edgeChange && pMovie)
    {
        pMovie->SetEdgeAAMode((AAMode == AAMode_EdgeAA) ?
                              Render::EdgeAA_On : Render::EdgeAA_Disable);
    }
    AmpDirty = true;

}

void FxPlayerAppBase::ToggleStrokeType()
{
//     UInt32 rf = pRenderConfig->GetRenderFlags();
//     UInt32 stroke = rf & RenderConfig::RF_StrokeMask;
// 
//     switch (stroke) 
//     {
//     case RenderConfig::RF_StrokeHairline:
//         stroke = RenderConfig::RF_StrokeNormal;
//         break;
//     case RenderConfig::RF_StrokeNormal:
//         stroke = RenderConfig::RF_StrokeCorrect;
//         break;
//     case RenderConfig::RF_StrokeCorrect:
//         stroke = RenderConfig::RF_StrokeHairline;
//         break;
//     }
//     pRenderConfig->SetRenderFlags((rf & ~RenderConfig::RF_StrokeMask) | stroke);
//     AmpDirty = true;
}

// Switch international font.
void FxPlayerAppBase::CycleFontConfig()
{
    if ((FontConfigIndex != -1) && (FontConfigs.GetSize() > 1))
    {
        // Apply different settings and reload file.

        FontConfigIndex++;
        FontConfigIndex %= (int)FontConfigs.GetSize();
        FontConfig* pconfig = GetCurrentFontConfig();
        if (pconfig)
        {
            pconfig->Apply(&mLoader);
            const char* language = (FontConfigIndex == -1) ? "Default"
                : pconfig->ConfigName.ToCStr();
            pMovie->Invoke("_root.OnLanguageChanged", "%s", language);
        }
        AmpDirty = true;
    }
}

#ifdef SF_AMP_SERVER
void FxPlayerAppBase::HandleAmpAppMessages()
{
    AmpCallbackLock.DoLock();
    for (UPInt i = 0; i < AmpAppControlMsgs.GetSize(); ++i)
    {
        AMP::MessageAppControl* msg = AmpAppControlMsgs[i];
        if (msg->IsToggleWireframe())
        {
            Wireframe = !Wireframe;
            pRenderThread->SetWireframe(Wireframe);
        }
        if (msg->IsToggleOverdraw())
        {
            if (ProfileMode != Render::Profile_Overdraw)
            {
                ProfileMode = Render::Profile_Overdraw;
            }
            else
            {
                ProfileMode = Render::Profile_None;
            }
            pRenderThread->SetProfileMode(ProfileMode);
        }
        if (msg->IsToggleBatch())
        {
            if (ProfileMode != Render::Profile_Batch)
            {
                ProfileMode = Render::Profile_Batch;
            }
            else
            {
                ProfileMode = Render::Profile_None;
            }
            pRenderThread->SetProfileMode(ProfileMode);
        }
        if (msg->IsToggleBlending())
        {
            if (ProfileMode != Render::Profile_Blending)
            {
                ProfileMode = Render::Profile_Blending;
            }
            else
            {
                ProfileMode = Render::Profile_None;
            }
            pRenderThread->SetProfileMode(ProfileMode);
        }
        if (msg->IsToggleTextureDensity())
        {
            if (ProfileMode != Render::Profile_TextureDensity)
            {
                ProfileMode = Render::Profile_TextureDensity;
            }
            else
            {
                ProfileMode = Render::Profile_None;
            }
            pRenderThread->SetProfileMode(ProfileMode);
        }
        if (msg->IsTogglePause())
        {
            Paused = !Paused;
            OnPause(Paused);
        }
        if (msg->IsToggleFastForward())
        {
            FastForward = !FastForward;
        }
        if (msg->IsToggleAaMode())
        {
            ToggleAaMode();
        }
        if (msg->IsToggleStrokeType())
        {
            ToggleStrokeType();
        }
        if (msg->IsRestartMovie())
        {
            SafeMovieRestart();
        }
        if (msg->IsNextFont())
        {
            CycleFontConfig();
        }
        if (msg->GetBatchChange() != 0)
        {
            BatchHighlight += msg->GetBatchChange();
            if (BatchHighlight < 0)
            {
                BatchHighlight = -1;
            }
            pRenderThread->SetBatchHighlight(BatchHighlight);
        }
//         if (msg->IsCurveToleranceUp())
//         {
//             CurvePixelError = Alg::Min(10.0f, CurvePixelError + 0.5f);
//             pRenderConfig->SetMaxCurvePixelError(CurvePixelError);
//         }
//         if (msg->IsCurveToleranceDown())
//         {
//             CurvePixelError = Alg::Max(0.5f, CurvePixelError - 0.5f);
//             pRenderConfig->SetMaxCurvePixelError(CurvePixelError);
//         }
        if (!msg->GetLoadMovieFile().IsEmpty())
        {
            LoadMovie(msg->GetLoadMovieFile());
        }
        AmpDirty = true;
    }
    AmpAppControlMsgs.Clear();
    AmpCallbackLock.Unlock();

    UpdateAmpState();
}
#endif

void    FxPlayerAppBase::SafeMovieRestart()
{
    if (!pMovie)
        return;

    Paused = false;
    OnPause(Paused);
    pMovie->Restart(false);
    String argString;
    SetMovieVariables(argString);
    {
        ScopedAutoTestTimerReplacement autoTimer(this);
        if (AutoTest && !AutoTest->IsPlayback())
        {
            AutoTest->OnAdvanceMovie(0);
            pMovie->Advance(0.0f, 0);
        }
    }
    AdjustFrameTime();
}

#ifdef SF_AMP_SERVER
void FxPlayerAppBase::UpdateAmpState()
{
    if (AmpDirty)
    {
        Ptr<GFx::AMP::ServerState> ampState = *SF_NEW GFx::AMP::ServerState();

        ampState->StateFlags = AmpServer::GetInstance().GetCurrentState();
        ampState->ProfileLevel = AmpServer::GetInstance().GetProfileLevel();

        // Map Profile types to AMP flags. 
        static const unsigned ProfileModeFlags[Render::Profile_Count] =
        {
            0,
            GFx::AMP::Amp_RenderOverdraw,
            GFx::AMP::Amp_RenderBatch,
            GFx::AMP::Amp_RenderBlending,
            GFx::AMP::Amp_RenderTextureDensity
        };
        // Set the state with all flags disabled, then enable the one that is enabled now.
        for (unsigned profileMode = 0; profileMode < Render::Profile_Count; ++profileMode)
        {
            ampState->StateFlags &= ~(ProfileModeFlags[profileMode]);
        }
        ampState->StateFlags |= ProfileModeFlags[ProfileMode];

        if (Wireframe)
        {
            ampState->StateFlags |= GFx::AMP::Amp_App_Wireframe;
        }
        else
        {
            ampState->StateFlags &= ~GFx::AMP::Amp_App_Wireframe;
        }
        if (Paused)
        {
            ampState->StateFlags |= GFx::AMP::Amp_App_Paused;
        }
        else
        {
            ampState->StateFlags &= ~GFx::AMP::Amp_App_Paused;
        }
        if (FastForward)
        {
            ampState->StateFlags |= GFx::AMP::Amp_App_FastForward;
        }
        else
        {
            ampState->StateFlags &= ~GFx::AMP::Amp_App_FastForward;
        }

        ampState->ConnectedApp = GetAppTitle();
        ampState->ConnectedFile = FileName;
        ampState->CurveToleranceMin = 0.5f;
        ampState->CurveToleranceMax = 10.0f;
        ampState->CurveToleranceStep = 0.5f;
        for (unsigned i = 0; i < FontConfigs.GetSize(); ++i)
        {
            ampState->Locales.PushBack(FontConfigs[i]->ConfigName);
        }
        switch (AAMode) 
        {
        case AAMode_None:
            ampState->AaMode = "None";
            break;
        case AAMode_EdgeAA:
            // Display a custom message if edge AA is #ifdef-ed out.
#ifndef SF_NO_FXPLAYER_EDGEAA
            ampState->AaMode = "Edge AA";
            //@TODO
//             {
//                 // Report if EdgeAA is not supported by renderer.
//                 Render::Renderer::RenderCaps caps;
//                 caps.CapBits = 0;
//                 if (GetRenderHAL())
//                     GetRenderHAL()->GetRenderCaps(&caps);
//                 if (!(caps.CapBits & Renderer::Cap_FillGouraud))
//                     ampState->AaMode = "Edge AA (Not Supported)";
//                 else if (!(caps.CapBits & Renderer::Cap_FillGouraudTex))
//                     ampState->AaMode = "Edge AA (Limited)";
//             }
#else
            ampState->AaMode = "Edge AA [#disabled]";
#endif
            break;
        case AAMode_FSAA:
            if (ViewConfigHasFlag(Platform::View_FSAA))
                ampState->AaMode = "HW FSAA";
            else
                ampState->AaMode = "HW FSAA (Not Supported)";
            break;
        }

        // Display a custom message if stroke is #ifdef-ed out.
// #ifndef SF_NO_FXPLAYER_STROKER
//         ampState->StrokeType = "Correct";
//         if (stroke == RenderConfig::RF_StrokeNormal)
//             ampState->StrokeType = "Normal";
// #else
//         ampState->StrokeType = "Correct [#disabled]";
//         if (stroke == RenderConfig::RF_StrokeNormal)
//             ampState->StrokeType = "Normal [#disabled]";
// #endif

        ampState->CurrentLocale = (FontConfigIndex == -1) ? "Default"
            : FontConfigs[FontConfigIndex]->ConfigName.ToCStr(); 
        ampState->CurveTolerance = CurvePixelError;

        AmpServer::GetInstance().UpdateState(ampState);
        AmpDirty = false;
    }
}
#endif

// Handle dropped file
void FxPlayerAppBase::OnDropFiles(const String& path)
{
    // unload the translator
    if (FontConfigIndex >= 0)
        mLoader.SetTranslator(NULL);

    if (LoadDefaultFontConfigFromPath(path) ) 
    {
        // set the font config
       if (FontConfigs.GetSize() && FontConfigs[0]->FontLibFiles.GetSize())
            mLoader.SetDefaultFontLibName(FontConfigs[0]->FontLibFiles[0].ToCStr());
        FontConfig* pconfig = GetCurrentFontConfig();
        if (pconfig)
            pconfig->Apply(&mLoader);
    }

    // Unload current movie. If new movie fails to load try to re-load previous
    // if re-load fails we will see blank player.
    String prevPath;
    if (pMovieDef)
        prevPath = pMovieDef->GetFileURL();
    pMovie = 0;
    pMovieDef = 0;
    if (!LoadMovie(path) && (!prevPath.IsEmpty()))
        LoadMovie(prevPath);

    //!AB, set active movie
    //   if (pMovie)
    //       pMovie->HandleEvent(Event::SetFocus);

    BringWindowToFront();
}


void FxPlayerAppBase::ProcessCommand(unsigned controllerIdx, const FxPlayerCommand& cmd, bool downFlag)
{
#if defined(GFX_ENABLE_XML)
    if (AutoTest && !AutoTest->IsPlayback())
        cmd.RecordAndExecute(this, controllerIdx, downFlag);
    else
        cmd.Execute(this, controllerIdx, downFlag);
#else // GFX_ENABLE_XML

    cmd.Execute(this, controllerIdx, downFlag);
#endif
}

void FxPlayerAppBase::OnKeyFinishSeek()
{
    Paused = true;
    pMovie->SetPlayState(GFx::State_Playing);
    // Ensure tag actions are executed. This may change frame state to Stopped.
    {
        ScopedAutoTestTimerReplacement autoTimer(this);
        if (AutoTest && !AutoTest->IsPlayback())
        {
            AutoTest->OnAdvanceMovie(0);
            pMovie->Advance(0.0f, 0);
        }
    }
    pMovie->SetPause(Paused);
    PausedState = pMovie->GetPlayState();
    AdjustFrameTime();
    AmpDirty = true;
}

void FxPlayerAppBase::ApplyLanguage(const char* name)
{
    if (name != NULL)
    {
        int idx = GetFontConfigIndexByName(name);
        if (idx != -1)
        {
            FontConfigIndex = idx;
            FontConfig* pconfig = FontConfigs[FontConfigIndex];
            pconfig->Apply(&mLoader);
            const char* language = pconfig->ConfigName.ToCStr();
            //pmovie->SetVariable("_global.gfxLanguage", Value(language));
            Value retVal;
            pMovie->Invoke("_root.OnLanguageChanged", &retVal, "%s", language);
        }
        else
        {
            if (pMovie->GetLog())
            {
                pMovie->GetLog()->LogError("\nLanguage support for '%s' not found!\n\n", name);
            }
        }
        FontLibChanged = true;
    }
}

void FxPlayerAppBase::InstallHandlers()
{
    pMovie->SetFSCommandHandler(Ptr<FSCommandHandler>(*new FxPlayerFSCallback(this)));
    pMovie->SetUserEventHandler(Ptr<UserEventHandler>(*new FxPlayerUserEventHandler(this)));

    pMovie->SetUrlNavigator(Ptr<FxPlayerUrlNavigator>(*new FxPlayerUrlNavigator(this)));

    // setting ExternalInterface handler
    Ptr<CustomEIHandler> pei = *new CustomEIHandler();
    pMovie->SetExternalInterface(pei);

#if defined(SF_OS_WIN32) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE) && !defined(_DURANGO)
    if (GetAppImpl()->IsMultitouchSupported())
        pMovie->SetMultitouchInterface(Ptr<MultitouchInterface>(*new FxPlayerMultitouchInterface()));
#endif
}

void FxPlayerAppBase::SetFontProvider()
{
    // Create a system font provider on Windows. If this is not done,
    // system font characters will be displayed as squares.
    if (!GetArgs().GetBool("NoSystemFont")) 
    {
#if defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
        Ptr<GFx::FontProviderWin32> fontProvider = *new GFx::FontProviderWin32(::GetDC(0));
        mLoader.SetFontProvider(fontProvider);
#endif

        //// An example of using FreeType-2 font provider
        //Ptr<GFx::FontProviderFT2> fontProvider = *new GFx::FontProviderFT2;
        ////fontProvider->MapFontToFile("AsiaHS-NC-0502", 0, "C:\\Windows\\Fonts\\AsiaHS_Large.ttf", 0, Font::HintAll, Font::DontHint);

        //fontProvider->MapFontToFile("Times New Roman", 0,                   "C:\\WINDOWS\\Fonts\\times.ttf");
        //fontProvider->MapFontToFile("Times New Roman", Font::FF_Bold,       "C:\\WINDOWS\\Fonts\\timesbd.ttf");
        //fontProvider->MapFontToFile("Times New Roman", Font::FF_Italic,     "C:\\WINDOWS\\Fonts\\timesi.ttf");
        //fontProvider->MapFontToFile("Times New Roman", Font::FF_BoldItalic, "C:\\WINDOWS\\Fonts\\timesbi.ttf");

        //fontProvider->MapFontToFile("Arial",           0,                  "C:\\WINDOWS\\Fonts\\arial.ttf");
        //fontProvider->MapFontToFile("Arial",           Font::FF_Bold,      "C:\\WINDOWS\\Fonts\\arialbd.ttf");
        //fontProvider->MapFontToFile("Arial",           Font::FF_Italic,    "C:\\WINDOWS\\Fonts\\ariali.ttf");
        //fontProvider->MapFontToFile("Arial",           Font::FF_BoldItalic,"C:\\WINDOWS\\Fonts\\arialbi.ttf");

        //fontProvider->MapFontToFile("Verdana",         0,                  "C:\\WINDOWS\\Fonts\\verdana.ttf");
        //fontProvider->MapFontToFile("Verdana",         Font::FF_Bold,      "C:\\WINDOWS\\Fonts\\verdanab.ttf");
        //fontProvider->MapFontToFile("Verdana",         Font::FF_Italic,    "C:\\WINDOWS\\Fonts\\verdanai.ttf");
        //fontProvider->MapFontToFile("Verdana",         Font::FF_BoldItalic,"C:\\WINDOWS\\Fonts\\verdanaz.ttf");
        //fontProvider->MapFontToFile("Georgia",         0,                  "C:\\WINDOWS\\Fonts\\georgia.ttf");
        //mLoader.SetFontProvider(fontProvider);

        // An example of using Vita libpvf font provider
        //String arial = String(GetDefaultFilePath())   + "arial.ttf";
        //String times = String(GetDefaultFilePath())   + "times.ttf";
        //String georgia = String(GetDefaultFilePath()) + "georgia.ttf";
        //Ptr<GFx::FontProviderNGP> fontProvider = *new GFx::FontProviderNGP(8, 0);
        //fontProvider->MapFontToFile("Arial", 0, arial);
        //fontProvider->MapFontToFile("Times New Roman", 0, times);
        //fontProvider->MapFontToFile("Georgia", 0, georgia);
        //mLoader.SetFontProvider(fontProvider);

    }
}

bool FxPlayerAppBase::HandleFsCommand(const char* pcommand, const char* parg)
{
    if (SFstrcmp(pcommand, "GFxApplyLanguage") == 0)
    {
        ApplyLanguage(parg);
    }
    else if (SFstrcmp(pcommand, "wireframe") == 0)
    {
        if (parg != NULL)
        {
            Wireframe = (atoi(parg) != 0);
            pRenderThread->SetWireframe(Wireframe);
            AmpDirty = true;
        }
    }
    else if (SFstrcmp(pcommand, "SaveScreenShot") == 0)
    {
        FxPlayerCommandTakeScreenshot screenshot;
        screenshot.Execute(this, 0, true);
    }
    else
    {
        return false;
    }
    return true;
}

bool FxPlayerAppBase::HandleEvent(const GFx::Event& event)
{
    bool handled = false;
    const MouseCursorEvent& mcEvent = static_cast<const MouseCursorEvent&>(event);
    
    switch(event.Type)
    {
    case GFx::Event::DoShowMouse:
        if (mcEvent.MouseIndex == 0)
            SetCursor(MovieCursorType);
        handled = true;
        break;
    case GFx::Event::DoHideMouse:
        if (mcEvent.MouseIndex == 0)
            SetCursor(Platform::Cursor_Hidden);
        handled = true;
        break;

    case GFx::Event::DoSetMouseCursor:
        if (mcEvent.MouseIndex == 0)
        {
            switch(mcEvent.GetCursorShape())
            {
            default:
            case MouseCursorEvent::ARROW:
                MovieCursorType = Platform::Cursor_Arrow;
                break;
            case MouseCursorEvent::HAND:
                MovieCursorType = Platform::Cursor_Hand;
                break;
            case MouseCursorEvent::IBEAM:
                MovieCursorType = Platform::Cursor_IBeam;
                break;
            }
            if (GetCursor() != Platform::Cursor_Hidden)
                SetCursor(MovieCursorType);
        }
        handled = true;
        break;
    case GFx::Event::EnableClipping:
    case GFx::Event::DisableClipping:
         ClippingEnable = (event.Type == GFx::Event::EnableClipping);
         UpdateViewSize();
         handled = true;
         break;
    default:
        break;
    }
    return handled;
}

bool FxPlayerAppBase::IsImeEnabled() const
{
#if defined(SF_ENABLE_IME) && defined(SF_ENABLE_IME_WIN32)
    return true;
#else
    return false;
#endif
}

bool FxPlayerAppBase::OnIMEEvent(unsigned message, UPInt wParam, UPInt lParam, UPInt hWND, bool preprocess)
{
#ifndef SF_NO_IME_SUPPORT

    if (preprocess)
    {
        GFx::IMEWin32Event ev(GFx::IMEEvent::IME_PreProcessKeyboard, hWND, message, wParam, lParam, 0);
        if (pMovie)
        {
            UInt32 handleEvtRetVal = pMovie->HandleEvent(ev);
            return (handleEvtRetVal & Movie::HE_NoDefaultAction) != 0;
        }
        return Movie::HE_NotHandled;
    }
    GFx::IMEWin32Event ev(GFx::IMEEvent::IME_Default, hWND, message, wParam, lParam, true);
    if (pMovie)
    {
        UInt32 handleEvtRetVal = pMovie->HandleEvent(ev);
        return (handleEvtRetVal & Movie::HE_NoDefaultAction) != 0;
    }
    return Movie::HE_NotHandled;
    //  ForwardEventToMovie(event);
    //    return IMEHelper::OnEvent(event, pMovie);
#else
    SF_UNUSED5(message, wParam, lParam, hWND, preprocess);
    return false;
#endif
}

void FxPlayerAppBase::OnPadStick(unsigned controllerIdx, PadKeyCode keyCode, float xpos, float ypos)
{
    if (StickMode == PSM_NoStick)
    {
        return;
    }
    
    GamePadAnalogEvent event(keyCode, xpos, ypos, (Scaleform::UInt8)controllerIdx);
    
    if (pMovie)
    {
        pMovie->HandleEvent(event);
    }
}

void FxPlayerAppBase::OnPad(unsigned controllerIdx, PadKeyCode keyCode, bool downFlag)
{
    if (!pMovie)
    {
        return;
    }
    
    switch(keyCode)
    {
    case SF::Pad_Left:
        if (StickMode != PSM_NoPad)
        {
            OnKeyEvent(controllerIdx, Key::Left, 0, downFlag);
        }
        return;
    case SF::Pad_Right:
        if (StickMode != PSM_NoPad)
        {
            OnKeyEvent(controllerIdx, Key::Right, 0, downFlag);
        }
        return;
    case SF::Pad_Up:
        if (StickMode != PSM_NoPad)
        {
            OnKeyEvent(controllerIdx, Key::Up, 0, downFlag);
        }
        return;
    case SF::Pad_Down:
        if (StickMode != PSM_NoPad)
        {
            OnKeyEvent(controllerIdx, Key::Down, 0, downFlag);
        }
        return;
    default:
        break;
    }

    if (PadPassthrough)
    {
        Hash< PadKeyCode, Key::Code >::Iterator it = PadPassthroughMap.Find(keyCode);
        if (it != PadPassthroughMap.End())
        {
           Key::Code kc = it->Second;
           OnKeyEvent(controllerIdx, kc, 0, downFlag);
           return;
        }
    }

    FxPlayerCommand* command = PadKeyToCommand(keyCode);
    if (command != NULL)
    {
        if (downFlag || command->HandleKeyUp())
        {
            ProcessCommand(controllerIdx, *command, downFlag);
        }
    }
}


void FxPlayerAppBase::OnKey(unsigned controllerIndex, KeyCode keyCode,
                            unsigned wcharCode, bool downFlag, KeyModifiers mods) 
{
    if (!pMovie)
    {
        return;
    }

    bool ctrl = false;

    UInt32 keyPlusMods = mods.States & (KeyModifiers::Key_CtrlPressed |
                                        KeyModifiers::Key_AltPressed |
                                        KeyModifiers::Key_ShiftPressed);
    keyPlusMods <<= 16;
    keyPlusMods |= keyCode;

    if (!NoControlKeys) 
    {
        ctrl = mods.IsCtrlPressed();

        if (keyCode == Key::Control && !NoCtrlKey) 
        {
            return;
        }

        // TODO ???
        //if (keyCode == VK_MENU && downFlag)
        //    ControlKeyDown = false; // to enable Ctrl-Alt-... combinations to work

        if (downFlag && keyCode == Key::Escape)
        {
            // Cancel mouse manipulation
            if (MouseTracking != None) 
            {
                MouseTracking = None;           
                Zoom = ZoomStart;
                Move = MoveStart;
                UpdateUserMatrix();
                return;
            }
        }

        FxPlayerCommand* command = KeyToCommand(keyPlusMods);
        if (command != NULL)
        {
            if (downFlag || command->HandleKeyUp())
            {
                ProcessCommand(controllerIndex, *command, downFlag);

                // If we process the command, don't send it to the movie as well.
                return; 
            }
        }
    }

    if (downFlag && (keyCode == Key::None) && wcharCode)
    {
        CharEvent event(wcharCode);
        pMovie->HandleEvent(event);
    }
    
    // Inform the player about keystroke
    if (!ctrl || NoCtrlKey)
    {
        OnKeyEvent(controllerIndex, keyCode, wcharCode, downFlag, mods);
    }

    // Record the command
    if (AutoTest)
        AutoTest->OnKeyPress(controllerIndex, keyCode, wcharCode, downFlag, mods);
}

// Helper used to convert key codes and route them to GFxPlayer
void FxPlayerAppBase::OnKeyEvent(unsigned controllerIndex, KeyCode keyCode,
                                 unsigned int wcharCode, bool downFlag, KeyModifiers mods)
{
    if (keyCode != Key::None) 
    {
        if (pMovie) 
        {
            KeyEvent event(downFlag ? GFx::Event::KeyDown : KeyEvent::KeyUp,
                           keyCode, 0, wcharCode, mods, (UInt8)controllerIndex);
            pMovie->HandleEvent(event);
        }
    }
}

void FxPlayerAppBase::OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, 
                                    const Point<int>& mousePos, KeyModifiers mods)
{
    bool ControlKeyDown = mods.IsCtrlPressed();
    if (!pMovie)
        return;
    MousePrevPos = mousePos;
    Render::PointF p = AdjustToViewPort(mousePos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    // UpdateStatBags mouse state
    if (downFlag) 
    {
        MouseDownPos = mousePos;

        if (button == 0 && ControlKeyDown && !NoCtrlKey)
        {
            MouseTracking = Zooming;
        }
        else if (button == 1 && ControlKeyDown && !NoCtrlKey)
        {
            MouseTracking = Moving;
        }

        if (MouseTracking != None) 
        {
            ZoomStart = Zoom;
            MoveStart = Move;
            return;
        }

        MouseEvent event(GFx::Event::MouseDown, button, p.x, p.y, 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    } 
    else 
    {
        if (MouseTracking != None)
        {
            MouseTracking = None;
            return;
        } 

        MouseEvent event(GFx::Event::MouseUp, button, p.x, p.y, 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    }

    // Record the command
    if (AutoTest)
        AutoTest->OnMouseButton(mouseIndex, button, downFlag, p, mods);
}


void FxPlayerAppBase::OnMouseWheel(unsigned mouseIndex, float zdelta,
                                   const Point<int>& mousePos, KeyModifiers mods)
{
    bool controlDown = mods.IsCtrlPressed();

    Render::PointF p = AdjustToViewPort(mousePos);

    if (controlDown && !NoCtrlKey)// && MouseTracking == None)
    {
        ZoomStart = Zoom;

        float dZoom = Zoom;
        Zoom += 0.02f * zdelta * Zoom;

        if (Zoom < 0.02f)
            Zoom = dZoom;

        dZoom -= Zoom;

        Render::Matrix m;
        m.AppendScaling(ZoomStart);
        m.AppendTranslation(Move.x, Move.y);
        p = m.TransformByInverse(p);

        Move.x += dZoom * p.x;
        Move.y += dZoom * p.y;

        UpdateUserMatrix();
        return;
    }

    if (!pMovie)
        return;


    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    MouseEvent event(GFx::Event::MouseWheel, 0, p.x, p.y, zdelta, mouseIndex);

    pMovie->HandleEvent(event);

    // Record the command
    if (AutoTest)
        AutoTest->OnMouseWheel(mouseIndex, zdelta, p, mods);
}


void FxPlayerAppBase::OnMouseMove(unsigned mouseIndex,
                                  const Point<int>& mousePos, KeyModifiers mods)
{
    SF_UNUSED(mods);

    Point<int> delta = MousePrevPos - mousePos;
    MousePrevPos = mousePos;

    // Used by NotifyMouseState in the main loop
    if (!pMovie)
        return;

    if (MouseTracking == Zooming) 
    {
        float dZoom = Zoom; 
        Zoom += 0.01f * delta.y * Zoom;

        if (Zoom < 0.02f)
            Zoom = dZoom;

        dZoom -= Zoom;
        Render::PointF p = AdjustToViewPort(MouseDownPos);
        Render::Matrix2F m;
        m.AppendScaling(ZoomStart);
        m.AppendTranslation(MoveStart.x, MoveStart.y);

        p = m.TransformByInverse(p);

        Move.x += dZoom * p.x;
        Move.y += dZoom * p.y;

        UpdateUserMatrix();
        return;
    }

    if (MouseTracking == Moving) 
    {
        Move.x -= delta.x;
        Move.y -= delta.y;

        UpdateUserMatrix();
        return;
    }

    Render::PointF p = AdjustToViewPort(mousePos);
    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    //}
    MouseEvent event(GFx::Event::MouseMove, 0, p.x, p.y, 0.0f, mouseIndex);
    pMovie->HandleEvent(event);

    // Record the command
    if (AutoTest)
        AutoTest->OnMouseMove(mouseIndex, p, mods);
    
    // Mouse HitTest debugging logic
    /*
    Double xmouse = pMovie->GetVariableDouble("_root._xmouse");
    Double ymouse = pMovie->GetVariableDouble("_root._ymouse");

    int hitTest = pMovie->HitTest(p.x, p.y, Movie::HitTest_Shapes);
    //int hitTest = pMovie->HitTest(p.x, p.y, Movie::HitTest_ShapesNoInvisible);
    //int hitTest = pMovie->HitTest(p.x, p.y, Movie::HitTest_Bounds);
    //int hitTest = pMovie->HitTest(p.x, p.y, Movie::HitTest_ButtonEvents);

    pMovie->GetLog()->LogMessage("pMovie->HitTest: %d\n", hitTest);

    GFx::Value r;
    if (pMovie->Invoke("_root.hitTest", &r, "%f%f%d", xmouse, ymouse, 1))
        pMovie->GetLog()->LogMessage("_root.hitTest: %s\n", (int)r.ToString().ToCStr());
    */
}

void FxPlayerAppBase::OnTouchBegin(unsigned, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary) 
{
    SF_UNUSED4(id, pos, contact, primary);
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    if (!pMovie)
        return;

    Render::PointF p = AdjustToViewPort(pos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);

    Render::PointF pp((float)contact.x, (float)contact.y);
    pp = m.TransformByInverse(pp);

    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    TouchEvent event(GFx::Event::TouchBegin, id, p.x, p.y, pp.x, pp.y, primary);
    pMovie->HandleEvent(event);

    if (pGestureManager)
	{
		pGestureManager->SetMovie(pMovie);
		pGestureManager->ProcessDown(id, pos, p);
		//SF_DEBUG_MESSAGE(1, "Gesture recognizer ProcessDown\n");
	}
#endif
}

void FxPlayerAppBase::OnTouchEnd(unsigned, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary) 
{
    SF_UNUSED4(id, pos, contact, primary);
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    if (!pMovie)
        return;

    Render::PointF p = AdjustToViewPort(pos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);

    Render::PointF pp((float)contact.x, (float)contact.y);
    pp = m.TransformByInverse(pp);

    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    TouchEvent event(GFx::Event::TouchEnd, id, p.x, p.y, pp.x, pp.y, primary);
    pMovie->HandleEvent(event);

    if (pGestureManager)
	{
		pGestureManager->SetMovie(pMovie);
        pGestureManager->ProcessUp(id, pos, p);
		//SF_DEBUG_MESSAGE(1, "Gesture recognizer ProcessUp\n");
	}
#endif
}

void FxPlayerAppBase::OnTouchMove(unsigned, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary) 
{
    SF_UNUSED4(id, pos, contact, primary);
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    if (!pMovie)
        return;

    Render::PointF p = AdjustToViewPort(pos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);

    Render::PointF pp((float)contact.x, (float)contact.y);
    pp = m.TransformByInverse(pp);

    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    TouchEvent event(GFx::Event::TouchMove, id, p.x, p.y, pp.x, pp.y, primary);
    pMovie->HandleEvent(event);

    if (pGestureManager)
	{
		pGestureManager->SetMovie(pMovie);
		pGestureManager->ProcessMove(id, pos, p);
		//SF_DEBUG_MESSAGE(1, "Gesture recognizer ProcessMove\n");
	}
#endif
}

void FxPlayerAppBase::OnGestureBegin(unsigned, UInt32 gestureMask, const Point<int>& pos, 
                                          const PointF& translationDelta,
                                          const PointF& scaleDelta,
                                          float rotationDelta)
{
    SF_UNUSED5(gestureMask, pos, translationDelta, scaleDelta, rotationDelta);
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    if (!pMovie)
        return;

    Render::PointF p = AdjustToViewPort(pos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);

    Render::PointF pp = m.TransformByInverse(translationDelta);

    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    GestureEvent event(GFx::Event::GestureBegin, gestureMask, p.x, p.y, pp.x, pp.y, 
        scaleDelta.x, scaleDelta.y, rotationDelta);
    pMovie->HandleEvent(event);
#endif
}

void FxPlayerAppBase::OnGesture(unsigned, UInt32 gestureMask, const Point<int>& pos, 
                                     const PointF& translationDelta,
                                     const PointF& scaleDelta,
                                     float rotationDelta)
{
    SF_UNUSED5(gestureMask, pos, translationDelta, scaleDelta, rotationDelta);
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    if (!pMovie)
        return;
    //printf("!!! %d %d    %f %f\n", x,y,translationDeltaX,translationDeltaY);
    Render::PointF p = AdjustToViewPort(pos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);

    Render::PointF pp = m.TransformByInverse(translationDelta);

    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);
    //printf("!!! %f %f    %f %f\n", p.x,p.y,pp.x,pp.y);

    GestureEvent event(GFx::Event::Gesture, gestureMask, p.x, p.y, pp.x, pp.y, 
        scaleDelta.x, scaleDelta.y, rotationDelta);
    pMovie->HandleEvent(event);
#endif
}

void FxPlayerAppBase::OnGestureEnd(unsigned, UInt32 gestureMask, const Point<int>& pos)
{
    SF_UNUSED2(gestureMask, pos);
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    if (!pMovie)
        return;

    Render::PointF p = AdjustToViewPort(pos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    GestureEvent event(GFx::Event::GestureEnd, gestureMask, p.x, p.y);
    pMovie->HandleEvent(event);
#endif
}

void FxPlayerAppBase::OnUpdateCliboard(const wchar_t* text) 
{
    Ptr<Clipboard> pclipBoard = mLoader.GetClipboard();
    if (pclipBoard)
        pclipBoard->SetText(text);
}


void FxPlayerAppBase::OnFocus(bool set, KeyModifiers mods)
{
    if (AutoTest)
        AutoTest->OnFocus(set, mods);

    if (pMovie)
    {
        if (set)
        {
            // need to update toggled keys when focus is being transferred to
            // the movie's windows.
            pMovie->HandleEvent(GFx::SetFocusEvent(mods));
        }
        else
        {
            pMovie->HandleEvent(GFx::Event(GFx::Event::KillFocus));
        }
    }
}

void FxPlayerAppBase::OnPause(bool paused)
{
    // Note that toggling play state through pMovie->SetPlayState would only pause root clip,
    // not children. So instead we just set the global variable above which prevents Advance.
    // However, we may still need to save/restore state in frame.

    pMovie->ForceCollectGarbage();
    pMovie->SetPause(paused);

    if (paused)
        PausedState = pMovie->GetPlayState();
    else
        pMovie->SetPlayState(PausedState);
}

void FxPlayerAppBase::OnObjectsReport()
{
    pMovie->ForceCollectGarbage();
    pMovie->PrintObjectsReport(Movie::Report_ShortFileNames | 
                               //Movie::Report_SuppressOverallStats | 
                               Movie::Report_AddressesForAnonymObjsOnly |
                               Movie::Report_NoCircularReferences);
}

Render::PointF FxPlayerAppBase::AdjustToViewPort(const Point<int>& pos)
{
    Size<unsigned> szRemainder = GetViewSize() - MViewSize;
    return Render::PointF (static_cast<float>(pos.x - (int)szRemainder.Width/2), 
                           static_cast<float>(pos.y - (int)szRemainder.Height/2));      
}

// Scales coordinates in MovieDef space into the Movie's viewport space.
Render::Point<int> FxPlayerAppBase::AdjustMovieDefToViewPort(const Point<int>& pos)
{
    if ( !pMovie || !pMovieDef )
        return pos;

    Point<int> pt = pos;
    GFx::Viewport vp;
    pMovie->GetViewport(&vp);
    pt.x = (int)(pt.x * (vp.Width / pMovieDef->GetWidth()));
    pt.y = (int)(pt.y * (vp.Height / pMovieDef->GetHeight()));

    Size<unsigned> szRemainder = GetViewSize() - MViewSize;
    pt.x += szRemainder.Width/2;
    pt.y += szRemainder.Height/2;
    return pt;
}

void FxPlayerFSCallback::Callback(Movie* pmovie, const char* pcommand, const char* parg)
{
    Ptr<GFx::Log> plog = pmovie->GetLog();

    // Exclude logging if the app is in testing mode (record or playback).
    if(plog && !App->AutoTest)
    {
        plog->LogMessage("FsCallback: '");
        plog->LogMessage("%s", pcommand);
        plog->LogMessage("' '");
        plog->LogMessage("%s", parg);
        plog->LogMessage("'\n");
    }
    App->HandleFsCommand(pcommand, parg);
}
