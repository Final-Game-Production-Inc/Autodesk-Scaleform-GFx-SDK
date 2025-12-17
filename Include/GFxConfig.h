/**************************************************************************

PublicHeader:   None
Filename    :   GFxConfig.h
Content     :   GFx configuration file - contains #ifdefs for
                the optional components of the library
Created     :   June 15, 2005
Authors     :   All Scaleform Engineers

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_Config_H
#define INC_SF_GFX_Config_H

// ***** Prerequisites
//---------------------------------------------------------------------------

// Determine if we are using debug configuration. This needs to be done
// here because some of the later #defines rely on it.
#ifndef SF_BUILD_DEBUG
# if defined(_DEBUG) || defined(_DEBUGOPT)
#   define SF_BUILD_DEBUG
# endif
#endif

#ifndef SF_BUILD_DEBUGOPT
# ifdef _DEBUGOPT
#   define SF_BUILD_DEBUGOPT
# endif
#endif

// must be included after SF_BUILD_DEBUG defns
#include "Kernel/SF_Types.h" // reqd for SF_OS_*** macros

// Allow developers to replace the math.h
//#define SF_MATH_H  <math.h>

#define GFX_MAX_MICE_SUPPORTED          6
#define GFX_MAX_KEYBOARD_SUPPORTED      6

// Default mouse support enable state for GFx. Enabling mouse is also
// dynamically controlled by GFxMovieView::EnableMouseSupport().
#if defined(SF_OS_XBOX) || defined(SF_OS_XBOX360) || \
    defined(SF_OS_PSP) || defined(SF_OS_PS2) || defined(SF_OS_PS3) || \
    defined(SF_OS_GAMECUBE)
#define GFX_MOUSE_SUPPORT_ENABLED   0
#else
#define GFX_MOUSE_SUPPORT_ENABLED   1
#endif

// Enables multitouch support.
//#define GFX_MULTITOUCH_SUPPORT_ENABLE

#if defined(SF_OS_ANDROID) || defined(SF_OS_IPHONE) || defined(SF_OS_3DS) || defined(SF_OS_WINMETRO)
#define GFX_MULTITOUCH_SUPPORT_ENABLE
#endif

// Mobile app features, such as orientation and lifecycle events
#if defined(SF_OS_ANDROID) || defined(SF_OS_IPHONE)

// Initialize native gesture recognition (currently only available on iOS)
#define NATIVE_GESTURE_RECOGNIZE

// Initialize Scaleform gesture recognition (overrides NATIVE_GESTURE_RECOGNIZE)
#define GFX_GESTURE_RECOGNIZE

#define GFX_ENABLE_MOBILE_APP_SUPPORT
#endif


// ***** Install-time configurable add-ons
//---------------------------------------------------------------------------

// Enable/disable addons for MSVC builds
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360) || defined(SF_OS_PS3) || defined(SF_OS_ORBIS) || defined(SF_OS_WIIU)
    #include "GFxConfigAddons.h"

#if defined(SF_OS_WIN32) && defined(GFX_USE_VIDEO_WIN32)
    #define GFX_USE_VIDEO
#elif defined(SF_OS_XBOX360) && defined(GFX_USE_VIDEO_XBOX360)
    #define GFX_USE_VIDEO
#elif defined(SF_OS_PS3) && defined(GFX_USE_VIDEO_PS3) && !defined(GFX_USE_VIDEO)
    #define GFX_USE_VIDEO
#elif defined(SF_OS_WIIU) && defined(GFX_USE_VIDEO_WIIU) && !defined(GFX_USE_VIDEO)
    #define GFX_USE_VIDEO
#elif defined(SF_OS_ORBIS) && defined(GFX_USE_VIDEO_PS4)
    #define GFX_USE_VIDEO
#endif
#endif


// ***** General Scaleform options
//---------------------------------------------------------------------------
// Use a wrapper of built-in data type double.
// #define SF_SAFE_DOUBLE

// If this macro is defined, then code can be compiled with not ANSI-compatible
// support of floating point numbers. (Example: /fp:fast on Windows)
// #define SF_FLOATING_POINT_FAST

// Uncomment to enable SF_FLOATING_POINT_FAST on Durango x64, off by default.
#if !defined(SF_FLOATING_POINT_FAST) && defined(SF_OS_WINMETRO) && defined(_DURANGO) && defined(SF_CPU_X86_64)
//    #define SF_FLOATING_POINT_FAST
#endif

#ifdef SF_FLOATING_POINT_FAST
    #ifndef SF_SAFE_DOUBLE
        #define SF_SAFE_DOUBLE
    #endif
#endif


// Define this macro to eliminate custom wctype tables for functions such as
// G_iswspace, SFtowlower, g_towupper and so on. If this macro is defined GFx 
// will use system Unicode functions (which are incredibly slow on Microsoft 
// Windows).
//#define SF_NO_WCTYPE

// Enable thread support
#define SF_ENABLE_THREADS

// Undefine this to disable statistics tracking; this is useful for the final build.
#define SF_ENABLE_STATS

// Enable use of LIBJPEG. Wjen disabled, JPEGUtil becomes a no-op stub,
// and SWF JPEG image loading will stop functioning.
// Disabling saves roughly 60k in release build.
#define SF_ENABLE_LIBJPEG

// Define this macro if whole JPEGLIB is compiled as C++ code (CPP-ize). 
// By default libjpeg is pure C library and public names are not mangled. 
// Though, it might be necessary to mangle jpeglib's names in order to 
// resolve names clashing issues (for example, with XBox360's xmedia.lib).
//#define SF_CPP_LIBJPEG

// Enable use of ZLIB and GZLibFile class.
// When disabled, compressed SWF files will no longer load.
// Disabling saves roughly 20k in release build.
#define SF_ENABLE_ZLIB

// If disabled, SWF PNG image loading is not functioning.
// Note: beside the uncommenting the line below, it is necessary
// to provide the path and the library name for the linker.
// The name of the library is libpng.lib for all configurations;
// the path is different. For example, VC8, 64-bit, Release Static
// library is located at %(GFX)\3rdParty\libpng\lib\x64\msvc80\release_mt_static
#define SF_ENABLE_LIBPNG

// Enable the use of SIMD optimized functions, if available. If the platform does
// not support a SIMD instruction set, then this option will be disabled in SF_SIMD.h.
// SIMD instructions are disabled in debug builds, because without function inlining,
// they are considerably slower.
#ifndef SF_BUILD_DEBUG
    #define SF_ENABLE_SIMD
#endif

// Enable the use of TCP/IP sockets
// Needed for AMP and AS3 Net Sockets
#define SF_ENABLE_SOCKETS

// Enable use of PCRE - Perl Compatible Regular Expressions
#define SF_ENABLE_PCRE


// Enable AS3 Application Domain 
#define SF_ENABLE_APP_DOMAIN

// ***** Memory/Allocation Configuration
//---------------------------------------------------------------------------

// These defines control memory tracking used for statistics and leak detection.
// By default, memory debug functionality is enabled automatically by SF_BUILD_DEBUG.
#ifdef SF_BUILD_DEBUG
    // Enable debug memory tracking. This passes __FILE__ and __LINE__ data into
    // allocations and enabled DebugStorage in the memory heap used to track
    // statistics and memory leaks.
    #define SF_MEMORY_ENABLE_DEBUG_INFO

    // Wipe memory in the allocator; this is useful for debugging.
    #define SF_MEMORY_MEMSET_ALL

    // Check for corrupted memory in the free chunks. May significantly
    // slow down the allocations.
    //#define SF_MEMORY_CHECK_CORRUPTION
#else
    // Enable debug memory tracking even in the release mode.
    //#define SF_MEMORY_ENABLE_DEBUG_INFO
#endif

// Use for extreme debug only! This define enforces the use of system 
// memory allocator as much as possible. That is: no granulator; absolute 
// minimal heap and system granularities; no reserve; no segment caching.
// It increases the chances of early memory corruption detection on 
// segmentation fault. Works only with GSysAllocWinAPI and GSysAllocMMAP.
//#define SF_MEMORY_FORCE_SYSALLOC

// Use for extreme debug only! Enforces the use of _aligned_malloc/_aligned_free
// No memory heaps functionality, no statistics, no heap memory limits.
//#define SF_MEMORY_FORCE_MALLOC

// Trace all essential operations: CreateHeap, DestroyHeap, Alloc, Free, Realloc.
// See MemoryHeap::HeapTracer for details.
//#define SF_MEMORY_TRACE_ALL




// ***** Renderer Specific options
//---------------------------------------------------------------------------

// If this macro is defined the renderer will use Hairliner to generate thin
// lines. The Hairliner generates meshes about 3-4 times faster than the regular 
// stroker and produces no overlaps in meshes. It also generates about 33% 
// less triangles. Hairliner serves only 1-pixel anti-aliased strokes.
#define SF_RENDER_ENABLE_HAIRLINER

// Tessellator produces better triangles for the price of 2-4% of slowdown
#define SF_RENDER_ENABLE_MOUNTAINS

// Enable EdgeAA; adds roughly 30K in release build.
#if !(defined(SF_OS_IPHONE) && defined(SF_USE_GLES))
    #define SF_RENDER_ENABLE_EDGEAA
#endif

// Enable Render::GlyphPacker
#define SF_RENDER_ENABLE_GLYPH_PACKER //@FONT

// Enable Render::GlyphCache
#define SF_RENDER_ENABLE_GLYPH_CACHE //@FONT

// Enable gradient generation
#define SF_RENDER_ENABLE_GRADIENT_GEN

// Enable morphing (shape tween)
#define SF_RENDER_ENABLE_MORPHING

// Enable scale9grid
//#define SF_RENDER_ENABLE_SCALE9GRID //@TODO, the macro is not used!

#define SF_NO_GRADIENT_GEN 

// Profile view modes (overdraw, etc.)
#if !defined(SF_BUILD_SHIPPING)
    #define SF_RENDERER_PROFILE
#endif

// Enable filters. Undefine this to disable rendering of all filters.
#define SF_RENDER_ENABLE_FILTERS

// In versions of GFx before 4.3, the Lighten and Darken blend modes were estimations, and were
// not always correct. In 4.3, these require render targets to render properly, and (since 4.3.26) require
// a parent clip to have a Layer blend mode. Enabling this define returns these blend modes to their pre-4.3
// behavior.
//#define SF_RENDER_DARKEN_LIGHTEN_OLD_BEHAVIOR

// ***** GFx Logging options
//---------------------------------------------------------------------------

// Define this macro enables support for verbose parsing of input files.
// If this option is set, verbose parse options are available, and the
// GFxLoader::SetVerboseParse call will be able to produce text logs.
// Game production release builds should probably undefine this option.
#define GFX_VERBOSE_PARSE

#ifdef GFX_VERBOSE_PARSE
    // Define this macro to enable support for verbose parsing of actions
    // (support for disassembly during loading).
    #define GFX_VERBOSE_PARSE_ACTION

    // Define this macro to enable support for verbose parsing
    // of shape character structures.
    #define GFX_VERBOSE_PARSE_SHAPE

    // Define this macro to enable support for verbose parsing
    // of morph shape character structures.
    #define GFX_VERBOSE_PARSE_MORPHSHAPE
#endif



// ***** GFx Add-on Options
//---------------------------------------------------------------------------

// FMOD sound support is enabled for MSVC builds (PC/Xbox360) by default
// It will be automatically enabled for other platforms if FMOD is detected by build system
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360) || defined(SF_PS3_MSVC_BUILD) || \
    defined(SF_PSVITA_MSVC_BUILD) || defined(SF_WIIU_MSVC_BUILD) || defined(SF_3DS_MSVC_BUILD) || \
    defined(SF_OS_ORBIS)
    #define GFX_SOUND_FMOD
#endif

// Wwise sound support (AK Wwise SDK is available separately)
//#define GFX_SOUND_WWISE

// Define this macro to enable sound support (including core and ActionScript)
#if (defined(GFX_SOUND_FMOD) || defined(GFX_SOUND_WWISE)) && !defined(GFX_ENABLE_SOUND)
    // Enable sound if FMOD has been enabled/detected
    #define GFX_ENABLE_SOUND
#endif

// Define this macro to enable video support (including core and ActionScript)
#if defined(GFX_USE_VIDEO) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC) || \
    defined(SF_OS_XBOX360) ||  defined(SF_OS_PS3)   || defined(SF_OS_WII) || defined(SF_OS_WIIU) || defined(SF_OS_ORBIS))
    // Enable video only for CRI supported platforms
    #define GFX_ENABLE_VIDEO
#endif

// Enable video system sound interfaces
//#define GFX_VIDEO_USE_SYSTEM_SOUND

// Video sound interfaces based on FMOD & Wwise 
//#define GFX_VIDEO_USE_FMOD
//#define GFX_VIDEO_USE_WWISE

// Use DCR instead of TV for sound output on WiiU.
// By default audio will be played out of the TV speakers only.
//#define GFX_SOUND_OUTPUT_DRC

// Video can be used only with multithreading support
#if !defined(SF_ENABLE_THREADS)
    #undef GFX_ENABLE_VIDEO
#endif

// Enable core CSS support
#define GFX_ENABLE_CSS

// Enable core XML support
#define GFX_ENABLE_XML

#if defined(SF_OS_WINMETRO)
// Use standard C++11 thread library
#ifdef SF_ENABLE_THREADS
#define SF_USE_STD11_THREADS
#endif
// Disable TCP/IP sockets (AMP)
//#undef SF_ENABLE_SOCKETS
// Disable video support (except Durango XDK)
#ifndef _DURANGO
#undef GFX_ENABLE_VIDEO
#endif
// Disable sound and video support for Durango ADK
#ifdef _DURANGO_ADK
#undef GFX_SOUND_FMOD
#undef GFX_ENABLE_SOUND
#undef GFX_ENABLE_VIDEO
#endif
// Disable core IME support
#define SF_NO_IME_SUPPORT
#endif

// Disable core IME support
//#define SF_NO_IME_SUPPORT

#ifdef SF_NO_IME_SUPPORT
#define GFC_NO_LANGBAR_SUPPORT
#undef SF_ENABLE_IME
#undef SF_ENABLE_IME_WIN32
#endif

// Enable built-in Korean IME logic
//#define GFX_ENABLE_BUILTIN_KOREAN_IME

// Enable support for bidirectional text
#define GFX_ENABLE_BIDIRECTIONAL_TEXT

// Enable Adobe Native Extension support globally
#if defined (SF_OS_IPHONE) || defined (SF_OS_ANDROID)
#define SF_ENABLE_ANE
#endif


// ***** Other GFx Options
//---------------------------------------------------------------------------

// Enable DrawText API
#define GFX_ENABLE_DRAWTEXT

// Enable keyboard support. When disabled, no Key AS class will be provided, 
// HandleEvent with KeyEvent will not be supported, PAD keys on consoles 
// will not work.
#define GFX_ENABLE_KEYBOARD

// Enable mouse support. When disabled, this option also disables Mouse AS class.
#define GFX_ENABLE_MOUSE

// Default mouse support enable state for GFx. Enabling mouse is also
// dynamically controlled by GFxMovieView::EnableMouseSupport().
#if ( defined(SF_OS_XBOX360) || defined(SF_OS_PS3) ) && defined(GFX_ENABLE_MOUSE)
#undef GFX_ENABLE_MOUSE
#endif

// Enable game pad analog input support.
#if ( defined(SF_OS_PSVITA) || defined(SF_OS_PS3) || defined(SF_OS_XBOX360) || defined(SF_OS_3DS) || defined(SF_OS_WIIU) || defined(SF_OS_WIN32) || defined(SF_OS_ORBIS) )
#define GFX_ENABLE_ANALOG_GAMEPAD
#endif

// Enable font compactor (compaction during the run-time). When disabled, 
// fonts compacted by the gfxexport (with option -fc) remain working.
#define GFX_ENABLE_FONT_COMPACTOR

// Enable use of compacted fonts (fonts, compacted by gfxexport (option -fc))
#define GFX_ENABLE_COMPACTED_FONTS

// Enable TextField ActionScript extension functions. When disabled,
// standard GFxPlayer's HUD will not work.
#define GFX_ENABLE_TEXTFIELD_EXTENSIONS

// Enable text editing. When disabled, text selection will be disabled 
// as well (since it is a part of text editing).
#define GFX_ENABLE_TEXT_INPUT

// Scale9 hit-testing may affect performance. Disabling it may result 
// in incorrect Scale9 hit-testing.
#define GFX_ENABLE_SCALE9_HITTEST

// Define this macro to throw assertion if any font texture is generated during
// the runtime.
//#define GFX_ASSERT_ON_FONT_BITMAP_GEN

// Define this macro to throw assertion if any gradient texture is generated during
// the runtime.
//#define GFX_ASSERT_ON_GRADIENT_BITMAP_GEN

// Define this macro to throw assertion if any re-sampling occurred in renderer during
// the runtime.
//#define GFX_ASSERT_ON_RENDERER_RESAMPLING

// Define this macro to throw assertion if any mipmap levels generation occurred in
// renderer during the runtime.
//#define GFX_ASSERT_ON_RENDERER_MIPMAP_GEN

// Define this macro to throw assertion if fast math floating point model is used
// since it not officially supported
#define GFX_ASSERT_ON_IMPRECISE_FLOAT

// Check if Fast Math Floating point model is enabled with gcc, and warn user about potential problems
#if  defined (__FAST_MATH__) && defined (GFX_ASSERT_ON_IMPRECISE_FLOAT)
// ctremblay ++ Seems alright on Orbis. It saves us 2-3ms ...
//#error This compiler option can result in incorrect output for programs that depend on an exact implementation 
//#error of IEEE or ISO rules/specifications for math functions, and is not supported by the Scaleform SDK
// ctremblay --
#endif


// Scaleform Profiling configuration (Shipping + Debug Information), internal use only  
#ifdef SF_BUILD_PROFILING
#define SF_BUILD_SHIPPING
#endif


// ***** Remote memory and performance profiling
//---------------------------------------------------------------------------

#if !defined(SF_BUILD_SHIPPING) && defined(SF_ENABLE_THREADS)

#if defined(SF_ENABLE_SOCKETS) && defined(SF_ENABLE_STATS)
    #define SF_AMP_SERVER
#endif

// Enable debug memory tracking when AMP is enabled
//#define SF_MEMORY_ENABLE_DEBUG_INFO

// GPA 4.0 support
#if defined(PROFILE_GPA)
    #define SF_PROFILE_GPA
#endif

#endif

#if defined(SF_BUILD_DEBUG)

// Enables consistency checks for playlist (recommeded).
// May slow down debug build.
#define GFX_CHECK_PLAYLIST_DEBUG

#endif // #if defined(SF_BUILD_DEBUG)


// ***** ActionScript Options
//---------------------------------------------------------------------------

// ActionScript support macros enable AS2 and/or AS3 vms. It is not
// critical to disable these macros since if one of the GFx::ASSupport classes
// is not defined, the majority of the associated VM code isn't linked in.
// However, having ony GFX_AS?_SUPPORT_one macro defined is slightly more efficient
// as it eliminates some extra overhead and virtual calls.
// Lib-only customers who need only one VM shouldn't change these macros and instead
// initialize GFx only one of the supporting class.

// Defined if AS2 support is available, so you can create GFx::AS2Support class.
#define GFX_AS2_SUPPORT
// Defined if AS3 support is available, so you can create GFx::AS3Support class.
#define GFX_AS3_SUPPORT

// Do not change this, as it's controlled by macros above.
#if defined(GFX_AS2_SUPPORT) && defined(GFX_AS3_SUPPORT)
    #define GFX_VM_ABSTRACT(arg) virtual arg = 0
#else
    #define GFX_VM_ABSTRACT(arg) arg
#endif

// Default setting for bitmap smoothing (AS3).
#define GFX_AS3_SMOOTH_BMP_BY_DEFAULT true

// Define this macro to enable support for verbose logging of executed ActionScript opcodes.
// If this macro is undefined, GFxMovie::SetVerboseAction will have no effect.
// Game production release builds should probably #undef this option.
#define GFX_AS2_VERBOSE
// Same thing for AS3
#define GFX_AS3_VERBOSE

//#if !defined(SF_BUILD_DEBUG) && !defined(SF_BUILD_DEBUGOPT)
#if !defined(SF_BUILD_DEBUG) && !defined(SF_BUILD_DEBUGOPT) && !defined(SF_AMP_SERVER)
    // Turn verbose action OFF for Release builds
    #undef GFX_AS2_VERBOSE
    #undef GFX_AS3_VERBOSE
#endif

// Enable cleanup of orphaned GFx::Value instances holding references to objects
// in a Movie VM that is being destroyed. The GFx::Value instances will be set
// to UNDEFINED and its orphaned flag will be set.
#define GFX_AS_ENABLE_GFXVALUE_CLEANUP

// Enable ActionScript object user data storage (GFx::Value::SetUserData/GetUserData)
#define GFX_AS_ENABLE_USERDATA

// Enable support for verbose logging of ActionScript run-time errors.
// If this macro is undefined, GFxMovie::SetVerboseActionErrors will have no effect.
// Game production release builds should probably #undef this option.
#define GFX_AS2_VERBOSE_ERRORS

// Enable garbage collection
#define GFX_AS_ENABLE_GC

// Enable *Filter class support.
#define GFX_AS2_ENABLE_FILTERS

// Enable 'Date' ActionScript class support.
#define GFX_AS2_ENABLE_DATE

// Enable 'Point' ActionScript class support.
#define GFX_AS2_ENABLE_POINT

// Enable 'Rectangle' ActionScript class support
#define GFX_AS2_ENABLE_RECTANGLE

// Enable 'Transform' ActionScript class support
#define GFX_AS2_ENABLE_TRANSFORM

// Enable 'ColorTransform' ActionScript class support
#define GFX_AS2_ENABLE_COLORTRANSFORM

// Enable 'Matrix' ActionScript class support
#define GFX_AS2_ENABLE_MATRIX

// Enable 'TextSnapshot' class support
#define GFX_AS_ENABLE_TEXTSNAPSHOT

// Enable 'SharedObject' class support
#define GFX_AS2_ENABLE_SHAREDOBJECT
#define GFX_AS3_ENABLE_SHAREDOBJECT

// Enable 'MovieClipLoader' ActionScript class support
#define GFX_AS2_ENABLE_MOVIECLIPLOADER

// Enable 'LoadVars' ActionScript class support
#define GFX_AS2_ENABLE_LOADVARS

// Enable 'BitmapData' ActionScript class support. Note, if BitmapData
// is disabled then textfield does not support <IMG> HTML tags and image 
// substitutions.
#define GFX_AS2_ENABLE_BITMAPDATA

// Enable 'System.capabilities' ActionScript class support
#define GFX_AS2_ENABLE_CAPABILITIES

// Enable 'Color' ActionScript class support
#define GFX_AS2_ENABLE_COLOR

// Enable 'TextFormat' ActionScript class support
#define GFX_AS2_ENABLE_TEXTFORMAT

// Enable 'Selection' ActionScript class support.
#define GFX_AS2_ENABLE_SELECTION

// Enable 'Stage' ActionScript class support. Stage.height and Stage.width 
// will not be supported as well.
#define GFX_AS2_ENABLE_STAGE

// Enable 'Mouse' ActionScript class support. 
#define GFX_AS2_ENABLE_MOUSE

// Enable HTTP requests for loading
#if (defined (SF_OS_IPHONE) || defined (SF_OS_ANDROID) || defined (SF_OS_MAC) || defined (SF_OS_WIN32) || defined SF_OS_PS3) && !(defined (_DURANGO) || defined (SF_OS_WINMETRO))
//#define SF_ENABLE_HTTP_LOADING
#endif

// *** Some defines removed from shipping (final) GFX build, to improve performance
// Add them back in if you need the functionality, even in shipping.
#if defined(SF_BUILD_SHIPPING) && !defined(SF_AMP_CLIENT)
#undef SF_ENABLE_STATS
#undef GFX_VERBOSE_PARSE
#undef GFX_AS2_VERBOSE_ERRORS 
#endif

// ***** Example defines for lightweight GFX on small consoles
//---------------------------------------------------------------------------

#ifdef SF_BUILD_LITE
    #undef SF_ENABLE_LIBJPEG
    #undef SF_ENABLE_ZLIB
    #undef SF_ENABLE_LIBPNG

    #undef SF_ENABLE_STATS
    #undef SF_AMP_SERVER

    #undef GFX_VERBOSE_PARSE
    #undef GFX_VERBOSE_PARSE_ACTION
    #undef GFX_VERBOSE_PARSE_SHAPE
    #undef GFX_VERBOSE_PARSE_MORPHSHAPE
    #undef GFX_AS2_VERBOSE
    #undef GFX_AS2_VERBOSE_ERRORS

    #undef GFX_ENABLE_GLYPH_PACKER
    #undef GFX_ENABLE_GRADIENT_GEN

    #undef GFX_ENABLE_CSS
    #undef GFX_ENABLE_XML

    // disable all IME support 
    #define SF_NO_IME_SUPPORT
    #undef GFX_ENABLE_BUILTIN_KOREAN_IME
    #undef SF_ENABLE_IME
    #undef SF_ENABLE_IME_WIN32

    // *** Disabling ActionScript Options
    #undef GFX_AS2_ENABLE_FILTERS
    #undef GFX_AS2_ENABLE_DATE
    #undef GFX_AS2_ENABLE_POINT
    #undef GFX_AS2_ENABLE_RECTANGLE
    #undef GFX_AS2_ENABLE_TRANSFORM
    #undef GFX_AS2_ENABLE_COLORTRANSFORM
    #undef GFX_AS2_ENABLE_MATRIX
    #undef GFX_AS2_ENABLE_TEXTSNAPSHOT
    #undef GFX_AS2_ENABLE_SHAREDOBJECT
    #undef GFX_AS3_ENABLE_SHAREDOBJECT
    #undef GFX_AS2_ENABLE_MOVIECLIPLOADER
    #undef GFX_AS2_ENABLE_LOADVARS
    #undef GFX_AS2_ENABLE_BITMAPDATA
    #undef GFX_AS2_ENABLE_CAPABILITIES
    //#undef GFX_AS2_ENABLE_COLOR
    #undef GFX_AS2_ENABLE_TEXTFORMAT
    #undef GFX_AS2_ENABLE_SELECTION
    //#define GFX_AS2_ENABLE_STAGE
    #undef GFX_ENABLE_DRAWTEXT

    #undef GFX_ENABLE_FONT_COMPACTOR
    #undef GFX_ENABLE_TEXTFIELD_EXTENSIONS

    // no text input support on consoles
    #if defined(SF_OS_XBOX360) || defined(SF_OS_PS3) || defined(SF_OS_WII) || defined(SF_OS_3DS)
        #undef GFX_ENABLE_TEXT_INPUT
    #endif

    // no mouse support on consoles
    #if defined(SF_OS_XBOX360) || defined(SF_OS_PS3) 
        #undef GFX_ENABLE_MOUSE
    #endif
#endif

// Shipping builds that use AS3 Net Sockets need approval from Nintendo, as they use the SO library
// To enable AS3 Net Sockets, comment out the following lines
#if defined(SF_OS_WIIU) && defined(SF_BUILD_SHIPPING)
#undef SF_ENABLE_SOCKETS
#endif


// ***** Automatic class dependency checks (DO NOT EDIT THE CODE BELOW)
//---------------------------------------------------------------------------

#ifndef GFX_ENABLE_MOUSE
    #undef GFX_AS2_ENABLE_MOUSE
#endif
    
#ifndef GFX_AS2_ENABLE_POINT
    // GASRectangle is heavily dependent on GASPoint
    #undef GFX_AS2_ENABLE_RECTANGLE
#else
    #if !defined(GFX_AS2_ENABLE_RECTANGLE) || !defined(GFX_AS2_ENABLE_MATRIX)
        // GASPoint is useless without GASRectangle or GASMatrix
        #undef GFX_AS2_ENABLE_POINT
    #endif
#endif


#ifndef GFX_AS2_ENABLE_TRANSFORM
    // GASColorTransform is useless without GASTransform
    #undef GFX_AS2_COLORTRANSFORM
#endif


#if !defined(GFX_AS2_ENABLE_MATRIX) || \
    !defined(GFX_AS2_ENABLE_RECTANGLE) || \
    !defined(GFX_AS2_ENABLE_COLORTRANSFORM)
    // GASTransform is useless without GASMatrix, GASRectangle, GASColorTransform
    #undef GFX_AS2_ENABLE_TRANSFORM
#endif


#endif
