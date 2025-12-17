/**************************************************************************

Filename    :   FxPlayerConfig.h
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :   January 10, 2008
Authors     :   Michael Antonov, Maxim Didenko, Boris Rayskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxPlayerConfig_H
#define INC_FxPlayerConfig_H

// Adds the word "Debug" to the application
// title if in debug build mode
#ifdef GFC_BUILD_DEBUG
#define GFC_DEBUG_STRING    " " GFC_BUILD_STRING
#else
#define GFC_DEBUG_STRING
#endif

// This includes the appropriate system-specific header files
// and defines the following macros:
//
//  - FXPLAYER_APP              - Base application class we will use.
//  - FXPLAYER_APP_TITLE        - Title-bar message.
//  - FXPLAYER_FILEDIRECTORY    - Platform specific sample file directory.
//  - FXPLAYER_FILENAME         - Initial file to load, if any.
//  - FXPLAYER_VIEWWIDTH        - Default width of window or video-mode.
//  - FXPLAYER_VIEWHEIGHT       - Default height of window or video-mode.
//  - FXPLAYER_FONT_SIZE        - Scaling size for HUD font on some platforms.

#include "FxApp.h"

#if defined(FXPLAYER_X11)
    // OpenGL application class
    #include "../Common/OpenGLX11App.h"
    #undef None
    #undef Status
    #define FXPLAYER_APP    OpenGLX11App
    // Window name
    #define     FXPLAYER_APP_TITLE  "Scaleform GFxPlayer OpenGL v" GFC_FX_VERSION_STRING GFC_DEBUG_STRING
    #define WHEEL_DELTA 1
    #define FXPLAYER_FILEPATH           ""
    #define FXPLAYER_VIEWWIDTH          1024
    #define FXPLAYER_VIEWHEIGHT         768
    #include <unistd.h>

#elif defined(GFC_OS_WIN32)
    #ifdef  FXPLAYER_RENDER_DIRECT3D
      #ifndef GFC_D3D_VERSION
      #define GFC_D3D_VERSION 9
      #endif
      #undef FXPLAYER_RENDER
      #if (GFC_D3D_VERSION == 9)
        // Direct3D application class
        #include "Direct3DWin32App.h"
        #define FXPLAYER_APP    Direct3DWin32App
        // Window name
        #define     FXPLAYER_APP_TITLE  "Scaleform GFxPlayer D3D9 v" GFC_FX_VERSION_STRING GFC_DEBUG_STRING
      #elif (GFC_D3D_VERSION == 8)
        // Direct3D application class
        #include "Direct3DWin32App.h"
        #define FXPLAYER_APP    Direct3DWin32App
        // Window name
        #define     FXPLAYER_APP_TITLE  "Scaleform GFxPlayer D3D8 v" GFC_FX_VERSION_STRING GFC_DEBUG_STRING
      #elif (GFC_D3D_VERSION == 10)
        // Direct3D application class
        #include "Direct3DWin32App.h"
        #define FXPLAYER_APP    Direct3DWin32App
        // Window name
        #define     FXPLAYER_APP_TITLE  "Scaleform GFxPlayer D3D10 v" GFC_FX_VERSION_STRING GFC_DEBUG_STRING
      #elif (GFC_D3D_VERSION == 11)
        // Direct3D application class
        #include "Direct3DWin32App.h"
        #define FXPLAYER_APP    Direct3DWin32App
        // Window name
        #define     FXPLAYER_APP_TITLE  "Scaleform GFxPlayer D3D11 v" GFC_FX_VERSION_STRING GFC_DEBUG_STRING
      #endif
    #else
        // OpenGL application class
        #include "OpenGLWin32App.h"
        #define FXPLAYER_APP    OpenGLWin32App
        // Window name
        #define     FXPLAYER_APP_TITLE  "Scaleform GFxPlayer OpenGL v" GFC_FX_VERSION_STRING GFC_DEBUG_STRING
    #endif
    #define FXPLAYER_FILEPATH           ""
    #define FXPLAYER_VIEWWIDTH          1024
    #define FXPLAYER_VIEWHEIGHT         768
    #define GFC_SOUND_FMOD
    #include <zmouse.h> // for WHEEL_DELTA macro
    #include "GFx_FontProvider_Win32.h"

#elif defined(GFC_OS_MAC)
    // OpenGL application class
    #include "../Common/OpenGLMacApp.h"
    #undef None
    #define FXPLAYER_APP    OpenGLMacApp
    // Window name
    #define     FXPLAYER_APP_TITLE  "Scaleform GFxPlayer OpenGL v" GFC_FX_VERSION_STRING GFC_DEBUG_STRING
    #define WHEEL_DELTA 1
    #define FXPLAYER_FILEPATH           ""
    #define FXPLAYER_VIEWWIDTH          1024
    #define FXPLAYER_VIEWHEIGHT         768
    #include <unistd.h>

#elif defined(GFC_OS_XBOX360)
    // Direct3D application class
    #include "Direct3DXbox360App.h"
    #define FXPLAYER_APP                Direct3DXboxApp
    #define FXPLAYER_APP_TITLE          "Scaleform GFxPlayer XBox 360 v" GFC_FX_VERSION_STRING
    #define FXPLAYER_RENDER_DIRECT3D
    // - FXPLAYER_FILENAME must be located in this path
    #define FXPLAYER_FILEDIRECTORY          "D:\\Samples\\"

    #define FXPLAYER_FILENAME           "3DGenerator.swf"

    #define FXPLAYER_FILEPATH           FXPLAYER_FILEDIRECTORY FXPLAYER_FILENAME
    // Resolution at which the player will start, normally 640x480.
    // Specify 1280 and 720 for 720p res.
    #define FXPLAYER_VIEWWIDTH          640
    #define FXPLAYER_VIEWHEIGHT         480
    #define FXPLAYER_FONT_SIZE          10
    #define GFC_SOUND_FMOD
    #define WHEEL_DELTA 1

	#if _XDK_VER >= 9328
		#ifdef GFC_BUILD_DEBUG
			#pragma comment(lib, "xmahald")
			#pragma comment(lib, "xuirund")
			#pragma comment(lib, "xuirenderd")
		#else
			#pragma comment(lib, "xmahal")
			#pragma comment(lib, "xuirun")
			#pragma comment(lib, "xuirender")
		#endif
	#else
		#ifdef GFC_BUILD_DEBUG
			#pragma comment(lib, "xactd")
			#pragma comment(lib, "xaudiod")
		#else
			#pragma comment(lib, "xact")
			#pragma comment(lib, "xaudio")
		#endif
	#endif

#elif defined (GFC_OS_PS3)
  #if defined(FXPLAYER_RENDER_GCM)
    #include "../Common/GcmPS3App.h"
    #define FXPLAYER_APP                GcmPS3App
  #else
    #include "../Common/OpenGLPS3App.h"
    #define FXPLAYER_APP                OpenGLPS3App
  #endif
    #include <sys/paths.h>
    #include <sys/spu_initialize.h>
    #include <sys/spu_thread.h>
    #include <sys/spu_thread_group.h>
    #include <sys/ppu_thread.h>
    #include <sys/process.h>
    #include <cell/sysmodule.h>
    #include <cell/spurs.h>
    #include <cell/spurs/control.h>
    #define FXPLAYER_APP_TITLE          "Scaleform GFxPlayer PS3 v" GFC_FX_VERSION_STRING
    #define FXPLAYER_FILEDIRECTORY      SYS_HOST_ROOT "/usr/local/cell/FxPlayer/"
    #define FXPLAYER_FILENAME           "flash.swf"
    #define FXPLAYER_FILEPATH           FXPLAYER_FILEDIRECTORY FXPLAYER_FILENAME
    #define FXPLAYER_VIEWWIDTH          640
    #define FXPLAYER_VIEWHEIGHT         480
    #define FXPLAYER_FONT_SIZE          10
    #define WHEEL_DELTA 1

#elif defined (GFC_OS_PS2)
   #include "../Common/PS2App.h"
   #define FXPLAYER_APP                PS2App
   // The path that will be searched for files
   // - FXPLAYER_FILENAME must be located in this path
   #define FXPLAYER_FILEDIRECTORY      "host0:/usr/local/sce/FxPlayer/"
   #define FXPLAYER_FILENAME           "flash.swf"
   #define FXPLAYER_FILEPATH           FXPLAYER_FILEDIRECTORY FXPLAYER_FILENAME
   #define FXPLAYER_APP_TITLE          "Scaleform GFxPlayer PS2 v" GFC_FX_VERSION_STRING
   #define FXPLAYER_VIEWWIDTH          640
   #define FXPLAYER_VIEWHEIGHT         448
   #define FXPLAYER_FONTCACHE_SMALL
   #define FXPLAYER_FONT_SIZE          10
   #define WHEEL_DELTA 1

#elif defined (GFC_OS_PSP)
   #include "../Common/PSPApp.h"
   #define FXPLAYER_APP                PSPApp
   // The path that will be searched for files
   // - FXPLAYER_FILENAME must be located in this path
   #define FXPLAYER_FILEDIRECTORY      "host0:/usr/local/psp/FxPlayer/"
   #define FXPLAYER_FILENAME           "flash.swf"
   #define FXPLAYER_FILEPATH           FXPLAYER_FILEDIRECTORY FXPLAYER_FILENAME
   #define FXPLAYER_VIEWWIDTH          640
   #define FXPLAYER_VIEWHEIGHT         480
   #define WHEEL_DELTA 1
   #define FXPLAYER_APP_TITLE          "Scaleform GFxPlayer PSP v" GFC_FX_VERSION_STRING
   #define FXPLAYER_FONTCACHE_SMALL
   #define FXPLAYER_FONT_SIZE          10

#elif defined (GFC_OS_WII)
   #include "../Common/WiiApp.h"
   #define FXPLAYER_APP                WiiApp
   // The path that will be searched for files
   // - FXPLAYER_FILENAME must be located in this path
   #define FXPLAYER_FILEDIRECTORY      "FxPlayer/"
   #define FXPLAYER_FILENAME           "flash.swf"
   #define FXPLAYER_FILEPATH           FXPLAYER_FILEDIRECTORY FXPLAYER_FILENAME
   #define FXPLAYER_VIEWWIDTH          640
   #define FXPLAYER_VIEWHEIGHT         480
   #define WHEEL_DELTA 1
   #define FXPLAYER_APP_TITLE          "Scaleform GFxPlayer Wii v" GFC_FX_VERSION_STRING
   #define FXPLAYER_FONTCACHE_SMALL
   #define FXPLAYER_FONT_SIZE          10

#endif

#ifndef GFC_NO_IME_SUPPORT
    class GFxIMEManager;
    template<class TApp>
    class TIMEHelper
    {
    public:
        static GFxIMEManager* CreateManager(TApp* app) { return NULL; }
        static bool OnEvent(const FxAppIMEEvent&, GFxMovieView*) { return false; }
    };
    #if defined(GFC_OS_WIN32)
    #if defined(GFC_USE_IME)
        #include <GFxIMEManager.h>
    #endif
    #if defined(GFC_USE_IME) && defined(GFC_USE_IME_WIN32)
        #include "GFxIMEManagerWin32.h"
        #endif // GFC_USE_IME && GFC_USE_IME_WIN32
        template<>
        class TIMEHelper<FXPLAYER_APP>
        {
        public:
            static GFxIMEManager* CreateManager(FXPLAYER_APP* app)
            {
                GUNUSED(app);
                GFxIMEManager* pimemanager = NULL;
                 #if defined(GFC_USE_IME) && defined(GFC_USE_IME_WIN32)
                    pimemanager = new GFxIMEManagerWin32(app->hWND);
                #endif // GFC_USE_IME
                return pimemanager;
            }
            static bool OnEvent(const FxAppIMEEvent& event, GFxMovieView* pmovie)
            {
                if (!pmovie)
                    return false;
                const FxWin32IMEEvent& appevent = static_cast<const FxWin32IMEEvent&>(event);
                if (appevent.Preprocess)
                {
                    GFxIMEWin32Event ev(GFxIMEEvent::IME_PreProcessKeyboard,
                                        (UPInt)appevent.hWND, appevent.message, appevent.wParam, appevent.lParam, 0);
                    UInt handleEvtRetVal = pmovie->HandleEvent(ev);
                    return (handleEvtRetVal & GFxMovieView::HE_NoDefaultAction) != 0;
                }
                GFxIMEWin32Event ev(GFxIMEWin32Event::IME_Default,
                                    (UPInt)appevent.hWND, appevent.message, appevent.wParam, appevent.lParam, true);
                UInt handleEvtRetVal = pmovie->HandleEvent(ev);
                return (handleEvtRetVal & GFxMovieView::HE_NoDefaultAction) != 0;
            }
        };
    #endif // GFC_OS_WIN32

    #if defined(GFC_OS_XBOX360)
    #if defined(GFC_USE_IME)
    #include <GFxIMEManager.h>
    #endif
    #if defined(GFC_USE_IME) && defined(GFC_USE_IME_XBOX360)
    #include "GFxIMEXBox360.h"
    #endif // GFC_USE_IME && GFC_USE_IME_XBOX360
            template<>
            class TIMEHelper<FXPLAYER_APP>
            {
            public:
                static GFxIMEManager* CreateManager(FXPLAYER_APP* app)
                {
                    GUNUSED(app);
                    GFxIMEManager* pimemanager = NULL;
    #if defined(GFC_USE_IME) && defined(GFC_USE_IME_XBOX360)
                    GFxIMEManagerXBox360* pxboximemanager = new GFxIMEManagerXBox360();
                    // Japanese Dictionaries
                    const int   iNumDicsJP = 5;
#if _XDK_VER < 9328
                    
                    const char  szMainDicPath[] = "D:\\Samples\\ximejpmain.dic";
                    const char  szBopomofoDicPath[] = "D:\\Samples\\ximechtbopomofo.dic";
                    const char  szSubDicPath[] = "D:\\Samples\\ximejpsub.dic";
                    const char  szTitleDicPath[] = "D:\\Samples\\ximejpnameaddr.dic";
                    const char  szUserTitleDicPath[] = "D:\\Samples\\XimeJPTitleDictionary.dic";
                    static char UserDicMemory[ XEIME_JP_SIZE_OF_USERDIC ];

                    //Initialize parameters
                    XIME_DICTIONARY    ximeDicJp[ iNumDicsJP ];
                    ZeroMemory( ximeDicJp, sizeof( ximeDicJp ) );
                    ximeDicJp[ 0 ].eDicType    = XIME_DIC_JP_MAIN;
                    ximeDicJp[ 0 ].pcszDicFile = szMainDicPath;
                    ximeDicJp[ 1 ].eDicType    = XIME_DIC_JP_SUB;
                    ximeDicJp[ 1 ].pcszDicFile = szSubDicPath;
                    ximeDicJp[ 2 ].eDicType    = XIME_DIC_JP_TITLE;
                    ximeDicJp[ 2 ].pcszDicFile = szTitleDicPath;
                    ximeDicJp[ 3 ].eDicType    = XIME_DIC_JP_TITLE;
                    ximeDicJp[ 3 ].pcszDicFile = szUserTitleDicPath;
                    ximeDicJp[ 4 ].eDicType    = XIME_DIC_JP_USER;
                    ximeDicJp[ 4 ].fDicOnMemory = true;
                    ximeDicJp[ 4 ].pvDicMemory = UserDicMemory;
                    ZeroMemory( ximeDicJp[ 4 ].pvDicMemory, XEIME_JP_SIZE_OF_USERDIC );
#else  
                    LPCSTR ximeDicJp[iNumDicsJP];
#endif
                    // Set up dictionary paths..
                    pxboximemanager->SetJapaneseDictionaryParameters(ximeDicJp, iNumDicsJP, 5);

                    // Chinese Dictionaries
                    const int   iNumDicsTC = 1;

#if _XDK_VER < 9328
                    XIME_DICTIONARY    ximeDicTc[ iNumDicsTC ];
                    ZeroMemory( ximeDicTc, sizeof( ximeDicTc ) );
                    ximeDicTc[ 0 ].eDicType    = XIME_DIC_CHT_BOPOMOFO;
                    ximeDicTc[ 0 ].pcszDicFile = szBopomofoDicPath;
#else
                    LPCSTR ximeDicTc[iNumDicsTC];
#endif

                    pxboximemanager->SetChineseDictionaryParameters(ximeDicTc, iNumDicsTC, 5);

                    pxboximemanager->SetCharacterLimit(20);
                    pimemanager = pxboximemanager;

    #endif // GFC_USE_IME
                    return pimemanager;
                }
                static bool OnEvent(const FxAppIMEEvent& event, GFxMovieView* pmovie)
                {
                    if (!pmovie)
                        return false;
                    const FxXBoxIMEEvent& appevent = static_cast<const FxXBoxIMEEvent&>(event);

                    GFxIMEXBox360Event ev(appevent.key.VirtualKey, appevent.key.Unicode, appevent.key.Flags,
                        appevent.key.UserIndex, appevent.key.HidCode, appevent.pLastKey, appevent.eType);

                    UInt handleEvtRetVal = pmovie->HandleEvent(ev);

                    return (handleEvtRetVal) == 0;
                }
            };
    #endif // GFC_OS_XBOX360

    #if defined(GFC_OS_PS3)
    #if defined(GFC_USE_IME)
    #include <GFxIMEManager.h>
    #endif
    #if defined(GFC_USE_IME) && defined(GFC_USE_IME_PS3)
    #include "GFxIMEManagerPS3.h"
    #endif // GFC_USE_IME
                template<>
                class TIMEHelper<FXPLAYER_APP>
                {
                public:
                    static GFxIMEManager* CreateManager(FXPLAYER_APP* app)
                    {
                        GUNUSED(app);
                        GFxIMEManager* pimemanager = NULL;
   #if defined(GFC_USE_IME) && defined (GFC_USE_IME_PS3)
                        pimemanager = new GFxIMEManagerPS3();
    #endif // GFC_USE_IME && GFC_USE_IME_PS3
                        return pimemanager;
                    }
                    static bool OnEvent(const FxAppIMEEvent& event, GFxMovieView* pmovie)
                    {
                        if (!pmovie)
                            return false;
                        const FxPS3IMEEvent& appevent = static_cast<const FxPS3IMEEvent&>(event);
                        GFxIMEPS3Event ev((appevent.KeyCode), (appevent.CharCode), (appevent.Modifiers),
                            (appevent.LastKeyCode), (appevent.LastCharCode), (appevent.LastModifiers));
                        UInt handleEvtRetVal    = pmovie->HandleEvent(ev);
                        appevent.LastKeyCode    = ev.LastKeyCode;
                        appevent.LastCharCode   = ev.LastCharCode;
                        appevent.LastModifiers  = ev.LastModifiers;
                        return (handleEvtRetVal) != 0;
                    //    return true;
                    }
                };
    #endif // GFC_OS_PS3

    typedef  TIMEHelper<FXPLAYER_APP> IMEHelper;
#endif // GFC_NO_IME_SUPPORT

// Define commands handled by FxPlayerApp::ProcessCommand(). These are mapped
// to different keyboard keys and/or gamepad buttons based on platform.
enum FxPlayerCommand
{
    FPC_Unknown,
    FPC_Quit,
    FPC_ScaledDisplay,
    FPC_Wireframe,
    FPC_SafeRect,
    FPC_AntialiasingMode,
    FPC_FastMode,
    FPC_FastForward,
    FPC_VSync,
    FPC_Pause,
    FPC_FontConfig,
    FPC_Restart,
    FPC_StageClipping,
    FPC_StageCulling,
    FPC_TriangleOptimization,
    FPC_StepBack_1,
    FPC_StepForward_1,
    FPC_StepBack_10,
    FPC_StepForward_10,
    FPC_CurveToleranceDown,
    FPC_CurveToleranceUp,
    FPC_InfoHelp,
    FPC_InfoSummary,
    FPC_HudNextTab,
    FPC_HudPreviousTab,
    FPC_HudClose,
	FPC_HudOpen,
    FPC_RefreshHud,
    FPC_Fullscreen,
    FPC_DynamicTextures,
    FPC_StrokeMode,
    FPC_Background,
    FPC_CtrlKey,
    FPC_BackgroundColor,
    FPC_ResetUserMatrix,
    FPC_LoadNextFile,
    FPC_LoadPrevFile,
    FPC_UnloadMovie,
    FPC_ShowMouseCursor,
    FPC_Home,
    FPC_End,
#ifndef GFC_NO_SOUND
    FPC_Mute,
    FPC_VolumeUp,
    FPC_VolumeDown,
#endif
	FPC_HudReport,
    FPC_Mem_Report,
    FPC_ToggleOverdrawProfile,
    KEY_Return,
    KEY_Escape
};

struct FxPlayerCommandKeyEntry {
    FxApp::KeyCode      keyCode;
    FxPlayerCommand     cmd;
    const char*         helpText;
} FxPlayerCommandKeyMap[] =
{
#if defined(GFC_OS_WIN32) || defined(GFC_OS_XBOX360) || defined(FXPLAYER_X11) || defined(GFC_OS_MAC)
    { FxApp::S,         FPC_ScaledDisplay,            "  CTRL S          Toggle scaled display\n"       },
#if !defined(FXPLAYER_RENDER_DIRECT3D) || (GFC_D3D_VERSION != 10)
    { FxApp::W,         FPC_Wireframe,                "  CTRL W          Toggle wireframe\n"            },
#endif
    { FxApp::A, FPC_AntialiasingMode,         "  CTRL A          Toggle antialiasing mode\n"            },
#if !defined(GFC_OS_XBOX360)
    { FxApp::U,         FPC_Fullscreen,               "  CTRL U          Toggle fullscreen\n"           },
#endif
#if defined(FXPLAYER_RENDER_DIRECT3D) && (GFC_D3D_VERSION != 10) && !defined(GFC_OS_XBOX360)
    { FxApp::V,         FPC_DynamicTextures,          "  CTRL V          Toggle dynamic textures\n"     },
#endif
    { FxApp::F,             FPC_FastMode,               "  CTRL F          Toggle fast mode (FPS)\n"    },
    { FxApp::G,             FPC_FastForward,            "  CTRL G          Toggle fast forward\n"       },
    { FxApp::Y,             FPC_VSync,                  "  CTRL Y          Toggle VSync\n"              },
    { FxApp::P,             FPC_Pause,                  "  CTRL P          Toggle pause\n"              },
    { FxApp::N,             FPC_FontConfig,             "  CTRL N          Next font config\n"          },
    { FxApp::R,             FPC_Restart,                "  CTRL R          Restart the movie\n"         },
    { FxApp::D,             FPC_StageClipping,          "  CTRL D          Toggle stage clipping\n"     },
    { FxApp::C,             FPC_StageCulling,           "  CTRL C          Toggle stage culling\n"      },
    { FxApp::Z,             FPC_ResetUserMatrix,        "  CTRL Z          Reset Zoom\n"                },
    { FxApp::O,             FPC_TriangleOptimization,   "  CTRL O          Toggle triangle optimization\n"  },
    { FxApp::T,             FPC_StrokeMode,             "  CTRL T          Switch stroke type\n"        },
    { FxApp::E,             FPC_ToggleOverdrawProfile,  "  CTRL E          Overdraw Profile\n"          },

    { FxApp::F5, FPC_HudReport,					  "  CTRL F5         Print HUD report to console\n" },
    { FxApp::F6, FPC_Mem_Report,			  "  CTRL F6         Print full memory report to console\n" },
#ifndef GFC_NO_SOUND
    { FxApp::M,             FPC_Mute,					"  CTRL M          Toggle Mute\n"               },
    { FxApp::Up,            FPC_VolumeUp,			    "  CTRL Up         Increase Sound Volume \n"    },
    { FxApp::Down,          FPC_VolumeDown,			    "  CTRL Down       Decrease Sound Volume \n"    },
#endif
    { FxApp::Right,         FPC_StepForward_1,          "  CTRL Right      Step forward one frame\n"    },
    { FxApp::F8,            FPC_BackgroundColor,        "  F8              Cycle background color\n" },
    { FxApp::B,             FPC_Background,             "  CTRL B          Toggle swf background\n" },
    { FxApp::Left,          FPC_StepBack_1,             "  CTRL Left       Step back one frame\n"       },
    { FxApp::PageUp,        FPC_StepForward_10,         "  CTRL PageUp     Step forward 10 frames\n"    },
    { FxApp::PageDown,      FPC_StepBack_10,            "  CTRL PageDown   Step back 10 frames\n"       },
    { FxApp::Minus,         FPC_CurveToleranceDown,     "  CTRL -          Curve tolerance down\n"      },
    { FxApp::Equal,         FPC_CurveToleranceUp,       "  CTRL +          Curve tolerance up\n"        },
    { FxApp::BracketLeft,   FPC_LoadPrevFile,           "  CTRL [          Load previous file\n"        },
    { FxApp::BracketRight,  FPC_LoadNextFile,           "  CTRL ]          Load next file\n"            },
    { FxApp::L,             FPC_UnloadMovie,            "  CTRL L          UnloadMovie\n"            },
    { FxApp::F1,            FPC_InfoHelp,               "  F1              Toggle Help\n"               },
    { FxApp::F2,            FPC_InfoSummary,	        "  F2              Toggle Summary \n"           },
    { FxApp::H,             FPC_HudNextTab,	            "  CTRL H          Cycle HUD Tabs \n" },    
 	{ FxApp::F9,			FPC_CtrlKey,	            "  F9              Toggle Ctrl key handling between Player and movie\n" },   
    { FxApp::F12,           FPC_HudClose,	            "  F12             Close HUD\n"                },
    { FxApp::Home,          FPC_Home,                   "  CTRL Home       Increase Transparency\n"     },
    { FxApp::End,           FPC_End,                    "  CTRL End        Decrease Transparency\n\n"   },
    { FxApp::Q,             FPC_Quit,                   "  CTRL Q          Quit"                        },
#endif
    { FxApp::VoidSymbol,    FPC_Unknown,                "  Unknown"                                    }
};

struct FxPlayerCommandPadKeyEntry
{
    FxApp::PadKeyCode   keyCode;
    FxPlayerCommand     cmd;
    FxPlayerCommand     hudCmd; //Alternative command when Hud is in active
    const char*         helpText;
} FxPlayerCommandPadKeyMap[] =
{
#if defined(GFC_OS_XBOX360) || defined(GFC_OS_WIN32)
    { FxApp::Pad_A,     KEY_Return,             KEY_Return,          "  A          Enter or Mouse button\n" },
	{ FxApp::Pad_B,     KEY_Escape,             KEY_Escape,          "  B          Escape (Toggle Wireframe in HUD)\n" },
    { FxApp::Pad_Y,     FPC_Restart,            FPC_RefreshHud,      "  Y          Restart the movie (Refresh Stats Tree in HUD)\n" },
    { FxApp::Pad_X,     FPC_HudOpen,            FPC_HudClose,        "  X          Toggle HUD\n" },
    { FxApp::Pad_Start, FPC_Pause,              FPC_Pause,           "  Start      Toggle pause\n" },
    { FxApp::Pad_L1,    FPC_AntialiasingMode,   FPC_HudPreviousTab,  "  LB         Toggle anti-aliasing (Previous tab in HUD)\n" },
    { FxApp::Pad_R1,    FPC_Wireframe,            FPC_HudNextTab,      "  RB         Next tab in HUD\n" },
    { FxApp::Pad_L2,    FPC_LoadPrevFile,       FPC_LoadPrevFile,    "  LT         Previous file\n" },
    { FxApp::Pad_R2,    FPC_LoadNextFile,       FPC_LoadNextFile,    "  RT         Next file\n" },
    { FxApp::Pad_LT,    FPC_FastForward,        FPC_FastForward,     "  LS         Toggle Fast Forward\n" },
#if defined(GFC_OS_XBOX360) 
	{ FxApp::Pad_RT,    FPC_ShowMouseCursor,    FPC_ShowMouseCursor, "  RS         Enable mouse; Show/Hide cursor\n" },
#endif
#elif defined(GFC_OS_PS3) || defined(GFC_OS_PS2)
    { FxApp::Pad_X,     KEY_Return,             KEY_Return,          "  X          Enter or Mouse button\n" },
    { FxApp::Pad_O,     KEY_Escape,             KEY_Escape,       "  Circle     Escape (Toggle Wireframe in HUD)\n" },
    { FxApp::Pad_T,     FPC_Restart,            FPC_RefreshHud,      "  Triangle   Restart the movie (Refresh Stats Tree in HUD)\n" },
    { FxApp::Pad_S,     FPC_HudOpen,            FPC_HudClose,        "  Square     Toggle HUD\n" },
    { FxApp::Pad_Start, FPC_Pause,              FPC_Pause,           "  Start      Toggle pause\n" },
    { FxApp::Pad_L1,    FPC_AntialiasingMode,   FPC_HudPreviousTab,  "  L1         Toggle anti-aliasing (Previous tab in HUD)\n" },
    { FxApp::Pad_R1,    FPC_Wireframe,            FPC_HudNextTab,      "  R1         Next tab in HUD\n" },
    { FxApp::Pad_L2,    FPC_LoadPrevFile,       FPC_LoadPrevFile,    "  L2         Previous file\n" },
    { FxApp::Pad_R2,    FPC_LoadNextFile,       FPC_LoadNextFile,    "  R2         Next file\n" },
    { FxApp::Pad_LT,    FPC_FastForward,        FPC_FastForward,     "  LS         Toggle Fast Forward\n" },
    { FxApp::Pad_RT,    FPC_ShowMouseCursor,    FPC_ShowMouseCursor, "  RS         Enable mouse; Show/Hide cursor\n" },

#elif defined(GFC_OS_PSP)
    { FxApp::Pad_X,     KEY_Return,            KEY_Return,           "" },
    { FxApp::Pad_O,     KEY_Escape,            FPC_HudNextTab,       "  Circle     Escape (Next Tab in HUD)\n" },
    { FxApp::Pad_T,     FPC_Restart,           FPC_RefreshHud,       "  Triangle   Restart the movie (Refresh Stats Tree in HUD)\n" },
    { FxApp::Pad_S,     FPC_HudOpen,           FPC_HudClose,         "  Square     Toggle HUD\n" },
    { FxApp::Pad_Start, FPC_Pause,             FPC_Pause,            "  Start      Toggle pause\n" },
    { FxApp::Pad_Select,FPC_AntialiasingMode,  FPC_AntialiasingMode, "  Select     Toggle anti-aliasing\n" },
    { FxApp::Pad_L1,    FPC_LoadPrevFile,      FPC_LoadPrevFile,     "  L          Previous file\n" },
    { FxApp::Pad_R1,    FPC_LoadNextFile,      FPC_LoadNextFile,     "  R          Next file\n" },

#elif defined(GFC_OS_WII)
    { FxApp::Pad_A,     KEY_Return,            KEY_Return,          "  A          Enter & Mouse button\n" },
    { FxApp::Pad_B,     KEY_Escape,            KEY_Escape,           "  B          Escape (Select in HUD)\n" },
    { FxApp::Pad_Home,  FPC_Pause,             FPC_Pause,            "  Home       Toggle pause\n" },
    { FxApp::Pad_1,     FPC_HudOpen,           FPC_HudClose,         "  1          Toggle HUD\n" },
    { FxApp::Pad_2,     FPC_Unknown,           FPC_HudNextTab,       "  2          Toggle Help/Stats\n" },
    { FxApp::Pad_Minus, FPC_LoadPrevFile,      FPC_LoadPrevFile,     "  -          Previous file\n" },
    { FxApp::Pad_Plus,  FPC_LoadNextFile,      FPC_LoadNextFile,     "  +          Next file\n" },
    { FxApp::Pad_C,     FPC_FastForward,       FPC_FastForward,      "  C          Toggle Fast Forward\n" },
    { FxApp::Pad_Z,     FPC_AntialiasingMode,  FPC_AntialiasingMode, "  Z          Toggle anti-aliasing\n" },

#endif
    { FxApp::VoidPadKey, FPC_Unknown, FPC_Unknown, "Unknown" }
};



#endif // INC_FxPlayerConfig_H
