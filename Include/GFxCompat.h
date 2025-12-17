/**************************************************************************

Filename    :   GFxCompat.h
Content     :   Provides a compatibility layer for GFx 3.X code to work with GFx 4.0
Created     :   July 28, 2010
Authors     :   Mustafa Thamer

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_GFX_COMPAT_H
#define INC_GFX_COMPAT_H

#include "GFx.h"
#include "GFx_Kernel.h"
#include "GFx_AMP.h"
#include "../Src/GFx/GFx_TaskManager.h"

#ifdef SF_OS_WIN32
#include "GFx_FontProvider_Win32.h"
#include "GFx_Renderer_D3D9.h"
#endif

#include "Render/Renderer2D.h"

//
// NAMESPACES, recommended namespace aliases
//
namespace SF = Scaleform;
namespace GFx = SF::GFx;

//
// DEFINES
//
#define GFC_FX_VERSION_STRING GFX_VERSION_STRING
#define GFC_BUILD_STRING    SF_BUILD_STRING
#define GUNUSED             SF_UNUSED
#define GUNUSED1            SF_UNUSED1
#define GUNUSED2            SF_UNUSED2
#define GUNUSED3            SF_UNUSED3
#define GUNUSED4            SF_UNUSED4
#define GFC_64BIT_POINTERS  SF_64BIT_POINTERS
#define GINLINE             SF_INLINE
#define GHEAP_AUTO_NEW      SF_HEAP_AUTO_NEW
#define GHEAP_NEW           SF_HEAP_NEW
#define GHEAP_NEW_ID        SF_HEAP_NEW_ID
#define GNEW                SF_NEW
#define GASSERT             SF_ASSERT
#define GALLOC              SF_ALLOC
#define GMEMALIGN           SF_MEMALIGN
#define GREALLOC            SF_REALLOC
#define GFREE               SF_FREE
#define GFREE_ALIGN         SF_FREE_ALIGN
#define GUTF8Util           SF::UTF8Util
#define GCDECL              SF_CDECL

// commonly used configuration defines
#ifdef SF_D3D_VERSION
#define GFC_D3D_VERSION     SF_D3D_VERSION
#endif

#ifndef GFX_ENABLE_DRAWTEXT
#define GFC_NO_DRAWTEXT_SUPPORT
#endif

#ifndef SF_ENABLE_THREADS
#define GFC_NO_THREADSUPPORT
#endif

#ifndef GFX_ENABLE_SOUND
#define GFC_NO_SOUND
#endif

#ifdef SF_NO_IME_SUPPORT
#define GFC_NO_IME_SUPPORT
#endif

#ifndef GFX_AS2_ENABLE_SHAREDOBJECT
#define GFC_NO_FXPLAYER_AS_SHAREDOBJECT
#endif

#ifdef SF_BUILD_DEBUG
#define GFC_BUILD_DEBUG     
#endif

#ifdef SF_BUILD_DEBUGOPT
#define GFC_BUILD_DEBUGOPT  
#endif

#ifdef SF_OS_MAC
#define GFC_OS_MAC          
#endif

#ifdef SF_OS_WIN32
#define GFC_OS_WIN32        
#endif

#ifdef SF_OS_XBOX360
#define GFC_OS_XBOX360      
#endif

#ifdef SF_OS_PS3
#define GFC_OS_PS3          
#endif

#ifdef SF_OS_WII
#define GFC_OS_WII          
#endif

#ifdef GFX_ENABLE_VIDEO
#define GFC_USE_VIDEO       
#endif

// defines for templates and functions
#define GPtr                SF::Ptr
#define GRefCountBase       SF::RefCountBase
#define GArray              SF::Array
#define GArrayLH            SF::ArrayLH
#define GHashLH             SF::HashLH
#define G_Format            SF::Format

// commonly used from SF_Std.h
#define G_strncpy           SF::SFstrncpy
#define G_strcpy            SF::SFstrcpy
#define G_strchr            SF::SFstrchr
#define G_strlen            SF::SFstrlen
#define G_stricmp           SF::SFstricmp
#define G_strcmp            SF::SFstrcmp
#define G_strncmp           SF::SFstrncmp
#define G_strnicmp          SF::SFstrnicmp
#define G_vsprintf          SF::SFvsprintf

#define G_wcsncpy           SF::SFwcsncpy
#define G_wcscpy            SF::SFwcscpy
#define G_wcschr            SF::SFwcschr
#define G_wcslen            SF::SFwcslen
#define G_wcsicmp           SF::SFwcsicmp
#define G_wcscmp            SF::SFwcscmp
#define G_wcsncmp           SF::SFwcsncmp
#define G_wcsnicmp          SF::SFwcsnicmp

#define G_Min               SF::Alg::Min
#define G_Max               SF::Alg::Max
#define G_Clamp             SF::Alg::Clamp
#define G_Chop              SF::Alg::Chop
#define G_Lerp              SF::Alg::Lerp
#define G_IRound            SF::Alg::IRound
#define G_URound            SF::Alg::URound
#define G_Abs               SF::Alg::Abs

//
// TYPES
//
typedef unsigned                        UInt;
typedef int                             SInt;
typedef float                           Float;
using namespace                         SF::BaseTypes;

typedef SF::String                      GString;
typedef SF::StringLH                    GStringLH;
typedef SF::StringBuffer                GStringBuffer;
typedef SF::StatBag                     GStatBag;
typedef SF::StatInfo                    GStatInfo;
typedef SF::StringDataPtr               GStringDataPtr;  

typedef SF::MemoryHeap                  GMemoryHeap;
typedef SF::Memory                      GMemory;
typedef SF::SysFile                     GSysFile;
typedef SF::File                        GFile;
typedef SF::FileStat                    GFileStat;
typedef SF::Lock                        GLock;
typedef SF::Thread                      GThread;
typedef SF::Timer                       GTimer;

typedef SF::Render::Viewport            GViewport;
typedef SF::Render::Color               GColor;
typedef SF::Render::RectF               GRectF;
typedef SF::Render::PointF              GPointF;
typedef SF::Render::SizeF               GSizeF;

#ifdef GFX_ENABLE_SOUND
typedef SF::Sound::SoundRenderer        GSoundRenderer;
#endif

typedef GFx::System                     GFxSystem;
typedef GFx::Loader                     GFxLoader;
typedef GFx::FileOpener                 GFxFileOpener;
typedef GFx::MovieDef                   GFxMovieDef;
typedef GFx::Movie                      GFxMovie;
typedef GFx::Movie                      GFxMovieView;
typedef GFx::MovieInfo                  GFxMovieInfo;
typedef GFx::FSCommandHandler           GFxFSCommandHandler;
typedef GFx::Log                        GFxLog;
typedef GFx::ImageCreator               GFxImageCreator;
typedef SF::Key                         GFxKey;
typedef GFx::KeyEvent                   GFxKeyEvent;
typedef GFx::CharEvent                  GFxCharEvent;
typedef GFx::Event                      GFxEvent;
typedef GFx::MouseEvent                 GFxMouseEvent;
typedef GFx::MouseCursorEvent           GFxMouseCursorEvent;
#ifdef GFX_ENABLE_DRAWTEXT
typedef GFx::DrawTextManager            GFxDrawTextManager;
typedef GFx::DrawText                   GFxDrawText;
#endif
typedef GFx::Value                      GFxValue;
typedef GFx::UserEventHandler           GFxUserEventHandler;
typedef GFx::URLBuilder                 GFxURLBuilder;
typedef GFx::Clipboard                  GFxTextClipboard;
typedef GFx::ParseControl               GFxParseControl;
#ifdef SF_ENABLE_THREADS
typedef GFx::ThreadedTaskManager        GFxThreadedTaskManager;
#endif
#ifdef SF_OS_WIN32
typedef GFx::FontProviderWin32          GFxFontProviderWin32;
#endif
typedef GFx::FontLib                    GFxFontLib;
#ifdef GFX_ENABLE_SOUND
typedef GFx::Audio                      GFxAudio;
#endif
#ifdef GFX_ENABLE_XML
typedef GFx::XML::Parser                GFxXMLParser;
typedef GFx::XML::ParserExpat           GFxXMLParserExpat;
typedef GFx::XML::Support               GFxXMLSupport;
typedef GFx::XML::SupportBase           GFxXMLSupportBase;
#endif
typedef GFx::ExternalInterface          GFxExternalInterface;
typedef GFx::ActionControl              GFxActionControl;
#ifndef GFX_ENABLE_TEXT_INPUT
typedef SF::KeyModifiers           		GFxSpecialKeysState;
#endif
typedef GFx::SetFocusEvent              GFxSetFocusEvent;

typedef GFx::AMP::AppControlInterface   GFxAmpAppControlInterface;
typedef GFx::AMP::MessageAppControl     GFxAmpMessageAppControl;
typedef SF::MemItem                     GFxAmpMemItem;
typedef SF::AmpServer                   GFxAmpServer;
typedef SF::AmpFunctionTimer            GFxAmpFunctionTimer;

//
// ENUMS
//

// StatBasicValues
enum 
{
    GStatGroup_Default      = SF::StatGroup_Default,       
    GStatGroup_Core         = SF::StatGroup_Core,      
    GStat_Default_Mem       = SF::Stat_Default_Mem
};

enum StatRenderer
{
    GStatRender_Default = SF::StatGroup_Default,

        GStatRender_Mem = SF::StatRender_Mem
};

enum 
{
    GFxStatMV_Default = GFx::StatMV_Default,

    // MovieView memory.
    GFxStatMV_Mem = GFx::StatMV_Mem,
    GFxStatMV_MovieClip_Mem = GFx::StatMV_MovieClip_Mem,
    GFxStatMV_ActionScript_Mem = GFx::StatMV_ActionScript_Mem,
    GFxStatMV_Text_Mem = GFx::StatMV_Text_Mem,
    GFxStatMV_XML_Mem = GFx::StatMV_XML_Mem,
    GFxStatMV_Other_Mem = GFx::StatMV_Other_Mem,

    // MovieView Timings.
    GFxStatMV_Tks = GFx::StatMV_Tks,
    GFxStatMV_Advance_Tks = GFx::StatMV_Advance_Tks,
    GFxStatMV_Action_Tks = GFx::StatMV_Action_Tks,
    GFxStatMV_Seek_Tks = GFx::StatMV_Seek_Tks,
    GFxStatMV_Timeline_Tks = GFx::StatMV_Timeline_Tks,
    GFxStatMV_Input_Tks = GFx::StatMV_Input_Tks,
    GFxStatMV_Mouse_Tks = GFx::StatMV_Mouse_Tks,
    GFxStatMV_ScriptCommunication_Tks = GFx::StatMV_ScriptCommunication_Tks,
    GFxStatMV_GetVariable_Tks = GFx::StatMV_GetVariable_Tks,
    GFxStatMV_SetVariable_Tks = GFx::StatMV_SetVariable_Tks,
    GFxStatMV_Invoke_Tks = GFx::StatMV_Invoke_Tks,
    GFxStatMV_InvokeAction_Tks = GFx::StatMV_InvokeAction_Tks,
    GFxStatMV_Display_Tks = GFx::StatMV_Display_Tks,
    GFxStatMV_FontThrash_Tks = GFx::StatMV_FontThrash_Tks,
    GFxStatMV_GradientGen_Tks = GFx::StatMV_GradientGen_Tks,
    GFxStatMV_FontMiss_Tks = GFx::StatMV_FontMiss_Tks,

    // Counters.
    GFxStatMV_Counters = GFx::StatMV_Counters,
    GFxStatMV_Invoke_Cnt = GFx::StatMV_Invoke_Cnt,
    GFxStatMV_MCAdvance_Cnt = GFx::StatMV_MCAdvance_Cnt,
    GFxStatMV_Tessellate_Cnt = GFx::StatMV_Tessellate_Cnt,
};

#define SF_AMP_SCOPE_RENDER_TIMER_DISPLAY(functionName)      SF::AmpFunctionTimer _amp_timer_(SF::AmpServer::GetInstance().GetDisplayStats(), (functionName), SF::Amp_Profile_Level_Low, SF::Amp_Native_Function_Id_Display)


//
// MANUAL CHANGES
//
// GFxMovie::PlayState => GFx::PlayState
// GFxMovie::Playing => GFx::State_Playing
// GFxMovie::Stopped => GFx::State_Stopped
//
// GLock.Lock() => DoLock()
//

#endif      // INC_GFX_COMPAT_H
