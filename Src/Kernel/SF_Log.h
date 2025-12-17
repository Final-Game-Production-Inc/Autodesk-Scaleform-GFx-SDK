/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Log.h
Content     :   Logging support
Created     :   July 18, 2001
Authors     :   Brendan Iribe, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Log_H
#define INC_SF_Kernel_Log_H

#include "SF_Types.h"
#include "SF_RefCount.h"
#include <stdarg.h>

namespace Scaleform {


// ***** LogMessageType

// LogMessageType described the severity of the message and automatic prefix/suffix
// that should be given to it. LogMessageType bits are combined with LogChannel
// and an additional ingeger value to form LogMessageId.
    
// Different log messages have separate purposes and should
// be used appropriately as follows:
//
// Text:       General output of information to channel without prefix/newline.
//             Debug-only messages should combine with with LogChannel_Debug.
//
// Warning:    Warning messages should be emitted when the API is misused or
//             is in such a state that the desired operation will most likely
//             not perform as expected. However, the problem that caused the
//             warning is still handled gracefully by the system and will
//             not cause a crash. 

// Error:      Error messages should be emitted if there is an internal problem
//             that will cause a system or component to malfunction. These messages
//             may be generates when certain critical resources (files, devices,
//             memory) are not available or do not function properly. Error
//             messages can also be generated if the condition is serious enough
//             but will not be checked for in the release build (resource leak,
//             for example). Note that conditions for these messages should not cause
//             an immediate crash even in release build.

// Assert:     Assertion failures should be generated when the program data structure
//             or parameter is in an invalid state that will cause an immediate
//             failure or crash. Assertion failure usually indicates a program logic
//             error or invalid use of an API (assert messages and should NOT be 
//             generated due to invalid user input or device/file states that can
//             be handled more gracefully). Assert conditions will not be checked for in
//             the release build and will most likely cause a crash.

enum LogMessageType
{    
    LogMessage_Text    = 0x00000, // No prefix, no newline.
    LogMessage_Warning = 0x20000, // "Warning: %s\n". For unexpected conditions handled gracefully.
    LogMessage_Error   = 0x30000, // "Error: %s\n". For runtime errors resulting in API fail (but not crash).
    LogMessage_Assert  = 0x40000, // "Assert: %s\n". Used for condition leading to crash; debug only.
    // GFx-specific 
    LogMessage_Report  = 0x50000, // For reports (performance, memory, etc)
    // Bit mask used to extract LogMessageType.
    LogMessage_Mask    = 0xF0000
};

// *** Channels

// The idea of message channels is that they correspond to separate
// logs that can be displayed individually. Several message sub-types
// can, however, be generated within the same channel. End users may
// choose to ignore the recommended channel structure and either
// combine all of the messages (default output), or separate them
// further into individual logs.

enum LogChannel
{
    LogChannel_General  = 0x1000, // Used for end-user messages about I/O, errors, etc.
    LogChannel_Debug    = 0x2000, // Reserved flag for mapping GDebug messages into single log object.
    LogChannel_Render   = 0x3000, // Reserved flag for mapping GDebug messages into single log object.
    // GFx-specific channels.
    LogChannel_Script   = 0x4000, // Used for script-related messages (bad function parameters, etc.)
    LogChannel_Parse    = 0x5000, // Used for parse log generated during loading
    LogChannel_Action   = 0x6000, // Used for action-script instruction execution log 
    LogChannel_Memory   = 0x7000, // Used for memory reporting
    LogChannel_Performance  = 0x8000, // Used for performance reporting
    LogChannel_Mask     = 0xF000,
};


// LogMessageId wraps a message Id passed to Log::LogMessageVarg.

class LogMessageId
{
    int Id;
public:

    LogMessageId(int id = 0) : Id(id) { }
    LogMessageId(const LogMessageId& other) : Id(other.Id) { }

    LogMessageId& operator = (int id) { Id = id; return *this; }
    LogMessageId& operator = (LogMessageId& other) { Id = other.Id; return *this; }

    operator int () const { return Id; }

    LogMessageType  GetMessageType() const { return (LogMessageType)(Id & LogMessage_Mask); }
    LogChannel      GetChannel() const     { return (LogChannel)(Id & LogChannel_Mask); }
};

// ***** Combined Log Message types

// Log MessageId values used by Kernel.
enum LogConstants
{
    // General I/O errors and warnings
    Log_Message      = LogChannel_General | LogMessage_Text,
    Log_Warning      = LogChannel_General | LogMessage_Warning,
    Log_Error        = LogChannel_General | LogMessage_Error,
    
    // Debug-only messages (not generated in release build)
    Log_DebugMessage = LogChannel_Debug | LogMessage_Text,
    Log_DebugWarning = LogChannel_Debug | LogMessage_Warning,
    Log_DebugError   = LogChannel_Debug | LogMessage_Error,
    Log_DebugAssert  = LogChannel_Debug | LogMessage_Assert
};


// GFXLOG_MESSAGE_FUNC macro, generates convenience printf-style inline functions for message types
#ifdef __GNUC__
#define SF_LOG_VAARG_ATTRIBUTE(a,b) __attribute__((format (printf, a, b)))
#else
#define SF_LOG_VAARG_ATTRIBUTE(a,b)
#endif


//--------------------------------------------------------------------
// ***** Log

// Log defines a base class interface that can be implemented to catch both
// debug and runtime messages.
//  - Debug logging can be overridden by calling Log::SetGlobalLog.
//  - GFx logging can also be configured per GFx::Loader/Movie by calling
//    SetLog functions on those objects. Local movie log takes precedence
//    over the globally installed one.

class Log : public RefCountBase<Log, Stat_Default_Mem>
{
    friend class System;
public: 
    Log() { }
    virtual ~Log();

    // This virtual function receives all the messages,
    // developers should override this function in order to do custom logging
    virtual void    LogMessageVarg(LogMessageId messageId, const char* fmt, va_list argList);

    // Call the logging function with specific message type, with no type filtering.
    void            LogMessageById(LogMessageId messageId,
                                   const char* fmt, ...) SF_LOG_VAARG_ATTRIBUTE(3,4);

    // Log formating buffer size used by default LogMessageVarg. Longer strings are truncated.
    enum { MaxLogBufferMessageSize = 4096 };

    // Helper used by LogMessageVarg to format the log message, writing the resulting
    // string into buffer. It formats text based on fmt and appends prefix/new line
    // based on LogMessageType.
    static void    FormatLog(char* buffer, unsigned bufferSize, LogMessageId messageId,
                             const char* fmt, va_list argList);

    // Default implementation of LogMessageVarg called by LogMessageVarg.
    // Outputs content only in debug build.
    static void     DefaultLogMessageVarg(LogMessageId messageId,
                                          const char* fmt, va_list argList);

    // Standard Log Inlines
    inline void LogMessage(const char* fmt, ...) SF_LOG_VAARG_ATTRIBUTE(2,3)
    {
        va_list argList; va_start(argList, fmt);
        LogMessageVarg(Log_Message, fmt, argList);
        va_end(argList);
    }
    inline void LogError(const char* fmt, ...) SF_LOG_VAARG_ATTRIBUTE(2,3)
    {
        va_list argList; va_start(argList, fmt);
        LogMessageVarg(Log_Error, fmt, argList);
        va_end(argList);
    }
    inline void LogWarning(const char* fmt, ...) SF_LOG_VAARG_ATTRIBUTE(2,3)
    {
        va_list argList; va_start(argList, fmt);
        LogMessageVarg(Log_Warning, fmt, argList);
        va_end(argList);
    }


    // *** Global APIs

    // Global Log registration APIs.
    //  - Global log is used for SF_DEBUG messages.
    //  - Global log is NOT AddRefed, so users must keep it alive for logging.
    //  - It is also used by GFx by default GFx::Log when no log is specified.   
    static void     SetGlobalLog(Log *log);
    static Log*     GetGlobalLog();

    // Returns a pointer to default log object used for debugging.
    // This object is used by default for Global log if an alternative wasn't set.
    //  - Default implementation outputs to Debug output / console in
    //    DEBUG build only.   
    static Log*     GetDefaultLog();
};


} // Scaleform 

#endif
