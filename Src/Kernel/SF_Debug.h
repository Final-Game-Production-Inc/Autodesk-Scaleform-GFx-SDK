/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Debug.h
Content     :   General purpose debugging support
Created     :   July 18, 2001
Authors     :   Brendan Iribe, Michael Antonov

Notes       :   Debug warning functionality is enabled
                if and only if SF_BUILD_DEBUG is defined.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Debug_H
#define INC_SF_Kernel_Debug_H

#include "SF_Types.h"
#include "SF_Log.h"

namespace Scaleform {

void LogDebugMessage(LogMessageId id, const char* fmt, ...) SF_LOG_VAARG_ATTRIBUTE(2,3);

// If not in debug build, macros do nothing
#ifndef SF_BUILD_DEBUG

    // Simple output with prefix tag                                        
    #define SF_DEBUG_OUTPUT(cond, msgtype, str )                                           ((void)0)
    #define SF_DEBUG_OUTPUT1(cond, msgtype, str, p1)                                       ((void)0)
    #define SF_DEBUG_OUTPUT2(cond, msgtype, str, p1, p2)                                   ((void)0)
    #define SF_DEBUG_OUTPUT3(cond, msgtype, str, p1, p2, p3)                               ((void)0)
    #define SF_DEBUG_OUTPUT4(cond, msgtype, str, p1, p2, p3, p4)                           ((void)0)
    #define SF_DEBUG_OUTPUT5(cond, msgtype, str, p1, p2, p3, p4, p5)                       ((void)0)
    #define SF_DEBUG_OUTPUT6(cond, msgtype, str, p1, p2, p3, p4, p5, p6)                   ((void)0)
    #define SF_DEBUG_OUTPUT7(cond, msgtype, str, p1, p2, p3, p4, p5, p6, p7)               ((void)0)
    #define SF_DEBUG_OUTPUT8(cond, msgtype, str, p1, p2, p3, p4, p5, p6, p7, p8)           ((void)0)
    #define SF_DEBUG_OUTPUT9(cond, msgtype, str, p1, p2, p3, p4, p5, p6, p7, p8, p9)       ((void)0)
    #define SF_DEBUG_OUTPUT10(cond, msgtype, str, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) ((void)0)

    // Assertion with messages
    // Unlike SF_ASSERT, these macros display a message before breaking
    #define SF_DEBUG_ASSERT(cond, str)                                                     ((void)0)
    #define SF_DEBUG_ASSERT1(cond, str, p1)                                                ((void)0)
    #define SF_DEBUG_ASSERT2(cond, str, p1, p2)                                            ((void)0)
    #define SF_DEBUG_ASSERT3(cond, str, p1, p2, p3)                                        ((void)0)
    #define SF_DEBUG_ASSERT4(cond, str, p1, p2, p3, p4)                                    ((void)0)
    #define SF_DEBUG_ASSERT5(cond, str, p1, p2, p3, p4, p5)                                ((void)0)
    #define SF_DEBUG_ASSERT6(cond, str, p1, p2, p3, p4, p5, p6)                            ((void)0)
    #define SF_DEBUG_ASSERT7(cond, str, p1, p2, p3, p4, p5, p6, p7)                        ((void)0)
    #define SF_DEBUG_ASSERT8(cond, str, p1, p2, p3, p4, p5, p6, p7, p8)                    ((void)0)
    #define SF_DEBUG_ASSERT9(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9)                ((void)0)
    #define SF_DEBUG_ASSERT10(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)          ((void)0)

#else 

    // Simple output with prefix tag
    #define SF_DEBUG_OUTPUT(cond, msgtype, str)                                            do { if (cond) Scaleform::LogDebugMessage(msgtype, str); } while (0)
    #define SF_DEBUG_OUTPUT1(cond, msgtype, str, p1)                                       do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1); } while (0)
    #define SF_DEBUG_OUTPUT2(cond, msgtype, str, p1, p2)                                   do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2); } while (0)
    #define SF_DEBUG_OUTPUT3(cond, msgtype, str, p1, p2, p3)                               do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2,p3); } while (0)
    #define SF_DEBUG_OUTPUT4(cond, msgtype, str, p1, p2, p3, p4)                           do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2,p3,p4); } while (0)
    #define SF_DEBUG_OUTPUT5(cond, msgtype, str, p1, p2, p3, p4, p5)                       do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2,p3,p4,p5); } while (0)
    #define SF_DEBUG_OUTPUT6(cond, msgtype, str, p1, p2, p3, p4, p5, p6)                   do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2,p3,p4,p5,p6); } while (0)
    #define SF_DEBUG_OUTPUT7(cond, msgtype, str, p1, p2, p3, p4, p5, p6, p7)               do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2,p3,p4,p5,p6,p7); } while (0)
    #define SF_DEBUG_OUTPUT8(cond, msgtype, str, p1, p2, p3, p4, p5, p6, p7, p8)           do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2,p3,p4,p5,p6,p7,p8); } while (0)
    #define SF_DEBUG_OUTPUT9(cond, msgtype, str, p1, p2, p3, p4, p5, p6, p7, p8, p9)       do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2,p3,p4,p5,p6,p7,p8,p9); } while (0)
    #define SF_DEBUG_OUTPUT10(cond, msgtype, str, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) do { if (cond) Scaleform::LogDebugMessage(msgtype, str, p1,p2,p3,p4,p5,p6,p7,p8,p9,p10); } while (0)

    // Assertion with messages
    // Unlike SF_ASSERT, these macros display a message before breaking
    #define SF_DEBUG_ASSERT(cond, str)                                             do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT1(cond, str, p1)                                        do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT2(cond, str, p1, p2)                                    do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT3(cond, str, p1, p2, p3)                                do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2,p3); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT4(cond, str, p1, p2, p3, p4)                            do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2,p3,p4); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT5(cond, str, p1, p2, p3, p4, p5)                        do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2,p3,p4,p5); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT6(cond, str, p1, p2, p3, p4, p5, p6)                    do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2,p3,p4,p5,p6); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT7(cond, str, p1, p2, p3, p4, p5, p6, p7)                do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2,p3,p4,p5,p6,p7); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT8(cond, str, p1, p2, p3, p4, p5, p6, p7, p8)            do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2,p3,p4,p5,p6,p7,p8); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT9(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9)        do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2,p3,p4,p5,p6,p7,p8,p9); SF_DEBUG_BREAK; } } while (0)
    #define SF_DEBUG_ASSERT10(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)  do { if (!(cond)) { Scaleform::LogDebugMessage(Scaleform::Log_DebugAssert, str, p1,p2,p3,p4,p5,p6,p7,p8,p9,p10); SF_DEBUG_BREAK; } } while (0)

#endif // SF_BUILD_DEBUG


// Conditional warnings - "SF Warning: " prefix
#define SF_DEBUG_WARNING(cond, str)                                                SF_DEBUG_OUTPUT(cond,  Scaleform::Log_DebugWarning, str)
#define SF_DEBUG_WARNING1(cond, str, p1)                                           SF_DEBUG_OUTPUT1(cond, Scaleform::Log_DebugWarning, str,  p1)
#define SF_DEBUG_WARNING2(cond, str, p1, p2)                                       SF_DEBUG_OUTPUT2(cond, Scaleform::Log_DebugWarning, str,  p1,p2)
#define SF_DEBUG_WARNING3(cond, str, p1, p2, p3)                                   SF_DEBUG_OUTPUT3(cond, Scaleform::Log_DebugWarning, str,  p1,p2,p3)
#define SF_DEBUG_WARNING4(cond, str, p1, p2, p3, p4)                               SF_DEBUG_OUTPUT4(cond, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4)
#define SF_DEBUG_WARNING5(cond, str, p1, p2, p3, p4, p5)                           SF_DEBUG_OUTPUT5(cond, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5)
#define SF_DEBUG_WARNING6(cond, str, p1, p2, p3, p4, p5, p6)                       SF_DEBUG_OUTPUT6(cond, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6)
#define SF_DEBUG_WARNING7(cond, str, p1, p2, p3, p4, p5, p6, p7)                   SF_DEBUG_OUTPUT7(cond, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6,p7)
#define SF_DEBUG_WARNING8(cond, str, p1, p2, p3, p4, p5, p6, p7, p8)               SF_DEBUG_OUTPUT8(cond, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6,p7,p8)
#define SF_DEBUG_WARNING9(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9)           SF_DEBUG_OUTPUT9(cond, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6,p7,p8,p9)
#define SF_DEBUG_WARNING10(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)     SF_DEBUG_OUTPUT10(cond, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)
// Conditional warnings - "SF Warning: " prefix - only warns once per session.
#define SF_DEBUG_WARNONCE(cond, str)                                               { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT( warned, Scaleform::Log_DebugWarning, str); }}
#define SF_DEBUG_WARNONCE1(cond, str, p1)                                          { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT1(warned, Scaleform::Log_DebugWarning, str,  p1); }}
#define SF_DEBUG_WARNONCE2(cond, str, p1, p2)                                      { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT2(warned, Scaleform::Log_DebugWarning, str,  p1,p2); }}
#define SF_DEBUG_WARNONCE3(cond, str, p1, p2, p3)                                  { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT3(warned, Scaleform::Log_DebugWarning, str,  p1,p2,p3); }}
#define SF_DEBUG_WARNONCE4(cond, str, p1, p2, p3, p4)                              { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT4(warned, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4); }}
#define SF_DEBUG_WARNONCE5(cond, str, p1, p2, p3, p4, p5)                          { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT5(warned, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5); }}
#define SF_DEBUG_WARNONCE6(cond, str, p1, p2, p3, p4, p5, p6)                      { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT6(warned, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6); }}
#define SF_DEBUG_WARNONCE7(cond, str, p1, p2, p3, p4, p5, p6, p7)                  { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT7(warned, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6,p7); }}
#define SF_DEBUG_WARNONCE8(cond, str, p1, p2, p3, p4, p5, p6, p7, p8)              { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT8(warned, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6,p7,p8); }}
#define SF_DEBUG_WARNONCE9(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9)          { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT9(warned, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6,p7,p8,p9); }}
#define SF_DEBUG_WARNONCE10(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)    { static bool warned = false; if (!warned) { warned = (cond); SF_DEBUG_OUTPUT10(warned, Scaleform::Log_DebugWarning, str,  p1,p2,p3,p4,p5,p6,p7,p8,p9,p10); }}
// Conditional errors - "SF Error: " prefix
#define SF_DEBUG_ERROR(cond, str)                                                  SF_DEBUG_OUTPUT(cond,  Scaleform::Log_DebugError, str)
#define SF_DEBUG_ERROR1(cond, str, p1)                                             SF_DEBUG_OUTPUT1(cond, Scaleform::Log_DebugError, str,  p1)
#define SF_DEBUG_ERROR2(cond, str, p1, p2)                                         SF_DEBUG_OUTPUT2(cond, Scaleform::Log_DebugError, str,  p1,p2)
#define SF_DEBUG_ERROR3(cond, str, p1, p2, p3)                                     SF_DEBUG_OUTPUT3(cond, Scaleform::Log_DebugError, str,  p1,p2,p3)
#define SF_DEBUG_ERROR4(cond, str, p1, p2, p3, p4)                                 SF_DEBUG_OUTPUT4(cond, Scaleform::Log_DebugError, str,  p1,p2,p3,p4)
#define SF_DEBUG_ERROR5(cond, str, p1, p2, p3, p4, p5)                             SF_DEBUG_OUTPUT5(cond, Scaleform::Log_DebugError, str,  p1,p2,p3,p4,p5)
#define SF_DEBUG_ERROR6(cond, str, p1, p2, p3, p4, p5, p6)                         SF_DEBUG_OUTPUT6(cond, Scaleform::Log_DebugError, str,  p1,p2,p3,p4,p5,p6)
#define SF_DEBUG_ERROR7(cond, str, p1, p2, p3, p4, p5, p6, p7)                     SF_DEBUG_OUTPUT7(cond, Scaleform::Log_DebugError, str,  p1,p2,p3,p4,p5,p6,p7)
#define SF_DEBUG_ERROR8(cond, str, p1, p2, p3, p4, p5, p6, p7, p8)                 SF_DEBUG_OUTPUT8(cond, Scaleform::Log_DebugError, str,  p1,p2,p3,p4,p5,p6,p7,p8)
#define SF_DEBUG_ERROR9(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9)             SF_DEBUG_OUTPUT9(cond, Scaleform::Log_DebugError, str,  p1,p2,p3,p4,p5,p6,p7,p8,p9)
#define SF_DEBUG_ERROR10(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)       SF_DEBUG_OUTPUT10(cond, Scaleform::Log_DebugError, str,  p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)
// Conditional messages - no prefix
#define SF_DEBUG_MESSAGE(cond, str)                                                SF_DEBUG_OUTPUT(cond,  Scaleform::Log_DebugMessage, str)
#define SF_DEBUG_MESSAGE1(cond, str, p1)                                           SF_DEBUG_OUTPUT1(cond, Scaleform::Log_DebugMessage, str,  p1)
#define SF_DEBUG_MESSAGE2(cond, str, p1, p2)                                       SF_DEBUG_OUTPUT2(cond, Scaleform::Log_DebugMessage, str,  p1,p2)
#define SF_DEBUG_MESSAGE3(cond, str, p1, p2, p3)                                   SF_DEBUG_OUTPUT3(cond, Scaleform::Log_DebugMessage, str,  p1,p2,p3)
#define SF_DEBUG_MESSAGE4(cond, str, p1, p2, p3, p4)                               SF_DEBUG_OUTPUT4(cond, Scaleform::Log_DebugMessage, str,  p1,p2,p3,p4)
#define SF_DEBUG_MESSAGE5(cond, str, p1, p2, p3, p4, p5)                           SF_DEBUG_OUTPUT5(cond, Scaleform::Log_DebugMessage, str,  p1,p2,p3,p4,p5)
#define SF_DEBUG_MESSAGE6(cond, str, p1, p2, p3, p4, p5, p6)                       SF_DEBUG_OUTPUT6(cond, Scaleform::Log_DebugMessage, str,  p1,p2,p3,p4,p5,p6)
#define SF_DEBUG_MESSAGE7(cond, str, p1, p2, p3, p4, p5, p6, p7)                   SF_DEBUG_OUTPUT7(cond, Scaleform::Log_DebugMessage, str,  p1,p2,p3,p4,p5,p6,p7)
#define SF_DEBUG_MESSAGE8(cond, str, p1, p2, p3, p4, p5, p6, p7, p8)               SF_DEBUG_OUTPUT8(cond, Scaleform::Log_DebugMessage, str,  p1,p2,p3,p4,p5,p6,p7,p8)
#define SF_DEBUG_MESSAGE9(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9)           SF_DEBUG_OUTPUT9(cond, Scaleform::Log_DebugMessage, str,  p1,p2,p3,p4,p5,p6,p7,p8,p9)
#define SF_DEBUG_MESSAGE10(cond, str, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)     SF_DEBUG_OUTPUT10(cond, Scaleform::Log_DebugMessage, str,  p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)


// Convenient simple type value object output
// - v must be a stack object
#define SF_DEBUG_SINT(v)                                       SF_DEBUG_OUTPUT1(1, Scaleform::Log_DebugMessage, #v "(%d)",   int (v))
#define SF_DEBUG_UINT(v)                                       SF_DEBUG_OUTPUT1(1, Scaleform::Log_DebugMessage, #v "(%u)",   unsigned(v))
#define SF_DEBUG_HEX(v)                                        SF_DEBUG_OUTPUT1(1, Scaleform::Log_DebugMessage, #v "(0x%X)", unsigned(v))
#define SF_DEBUG_FLOAT(v)                                      SF_DEBUG_OUTPUT1(1, Scaleform::Log_DebugMessage, #v "(%f)",   double(v))
#define SF_DEBUG_DOUBLE(v)                                     SF_DEBUG_OUTPUT1(1, Scaleform::Log_DebugMessage, #v "(%f)",   double(v))
#define SF_DEBUG_CHAR(v)                                       SF_DEBUG_OUTPUT1(1, Scaleform::Log_DebugMessage, #v "(%c)",   char(v))
#define SF_DEBUG_WCHAR(v)                                      SF_DEBUG_OUTPUT1(1, Scaleform::Log_DebugMessage, #v "(%C)",   wchar_t(v))


} // Scaleform


#endif
