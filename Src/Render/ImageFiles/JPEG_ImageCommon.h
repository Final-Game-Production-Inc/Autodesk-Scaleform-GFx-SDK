/**************************************************************************

Filename    :   JPEG_ImageCommon.h
Content     :   Common header file for JPEG Reader/Writer implementation
Created     :   June 24, 2005
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "JPEG_ImageFile.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Std.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

#ifdef SF_ENABLE_LIBJPEG

#include <stdlib.h>

#ifndef SF_NO_LONGJMP
#include <setjmp.h>
#endif

#ifdef SF_CC_MSVC
// disable warning "warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable"
#pragma warning(disable:4611)
#endif

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360) || defined(SF_OS_XBOX)
   #if !defined(SF_CPP_LIBJPEG)
       #define SF_CPP_LIBJPEG
   #endif  
#endif

#if !defined(SF_CPP_LIBJPEG)
extern "C" {
#endif //SF_CPP_LIBJPEG

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360) || defined(SF_OS_XBOX) || defined(SF_OS_WINCE)
// Windows typedefs INT32 correctly but not INT16
    typedef short INT16;
// Remove JPEG's INT32 typedef
    #define  XMD_H
// Undef FAR, jpeglib redefines it identically
    #undef FAR
#endif

#ifdef SF_PS3_MSVC_BUILD
#include <jconfigPS3.h>
#define JCONFIG_INCLUDED
#endif

#include <jpeglib.h>
#if !defined(SF_CPP_LIBJPEG)
} // extern "C" {
#endif //SF_CPP_LIBJPEG


namespace Scaleform { namespace Render { namespace JPEG {

enum GJPEGUtilConstants
{
    // This constant should match/be smaller then ZLib_BacktrackSize in ZLibFile.cpp to
    // avoid zlib restarts during embedded JPEG loading in SWFs.
    JPEG_BufferSize = 2048
};

// ***** Error handler
//
struct JpegErrorHandler 
{
    struct jpeg_error_mgr pub;  // "public" fields 
    char    errorMessage[JMSG_LENGTH_MAX];
#ifndef SF_NO_LONGJMP
    //!AB: The jmp_buf should be aligned, and the alignment depends on platform.
    // For example, for Win64 this buffer should be aligned on 16-bytes boundary,
    // because it stores 128-bit XMMx registers in it.
    jmp_buf* psetjmp_buffer;    // for return to caller
#endif

    JpegErrorHandler();
    ~JpegErrorHandler();
};


// Called when jpeglib has a fatal error.
void JpegErrorExit(j_common_ptr cinfo);
// Set up some error handlers for the jpeg lib.
jpeg_error_mgr* SetupJpegErr(JpegErrorHandler* jerr);


// ***** Longjump Handling
#ifndef SF_NO_LONGJMP
#define SF_JPEG_SET_JMP(pcinfo,pjerr,rv) \
    do { \
        if (setjmp(*(pjerr)->psetjmp_buffer)) \
        { \
            jpeg_destroy_decompress(pcinfo); \
            return rv; \
        } \
    } while(0)

#define SF_JPEG_SET_JMP1(_this,rv) \
    do { \
        if (setjmp(*(_this)->JErr.psetjmp_buffer)) \
        { \
            jpeg_destroy_decompress(&(_this)->CInfo); \
            (_this)->CompressorOpened = false; \
            (_this)->ErrorOccurred = true; \
            return rv; \
        } \
    } while(0)

#else
#define SF_JPEG_SET_JMP(pcinfo,pjerr,rv) (void*)0
#define SF_JPEG_SET_JMP1(_this,rv)       (void*)0
#endif


}}} // namespace Scaleform::Render::JPEG

#endif // SF_ENABLE_LIBJPEG

