/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Log.h
Content     :   Logging functionality
Created     :   June 23, 2005
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX__LOG_H
#define INC_SF_GFX__LOG_H

#include "Kernel/SF_Log.h"
#include "Kernel/SF_RefCount.h"
// Include Loader because Log derives from GFxStateBag.
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

// ***** Declared Classes
class LogConstants;
template<class Derived> 
class LogBase;


#define GFXLOG_INLINE inline

#define GFXLOG_MESSAGE_FUNC_DECL(name, message_type) \
    GFXLOG_INLINE void name(const char* pfmt, ...) const SF_LOG_VAARG_ATTRIBUTE(2,3);

#define GFXLOG_MESSAGE_FUNC(name, message_type)                                                     \
    template <class Derived>                                                                        \
    GFXLOG_INLINE void LogBase<Derived>::name(const char* pfmt, ...) const                          \
        { va_list argList; va_start(argList, pfmt);                                                 \
          Derived* p = (Derived*)this;                                                              \
          Log *plog = p->GetLog();                                                                  \
          if (p->IsLogNotNull() || plog) plog->LogMessageVarg(message_type, pfmt, argList);         \
          va_end(argList); }

//Note: IsLogNotNull guarantees that plog is not NULL here!

#define GFXLOG_MESSAGE_FUNC_COND(name, cond_func, message_type)                                     \
    template <class Derived>                                                                        \
    GFXLOG_INLINE void LogBase<Derived>::name(const char* pfmt, ...) const                          \
        { va_list argList; va_start(argList, pfmt);                                                 \
          Derived* p = (Derived*)this;                                                              \
          if (p->P_##cond_func()) {                                                                 \
              Log *plog = p->GetLog();                                                              \
              if (p->IsLogNotNull() || plog) plog->LogMessageVarg(message_type, pfmt, argList); }   \
          va_end(argList); }

//Note: IsLogNotNull guarantees that plog is not NULL here!


// ***** New Log Constants introduced by GFx

enum LogConstants_GFx
{
    // Script errors and warnings
    Log_ScriptError     = LogChannel_Script | LogMessage_Error,
    Log_ScriptWarning   = LogChannel_Script | LogMessage_Warning,
    Log_ScriptMessage   = LogChannel_Script | LogMessage_Text,
    // Parse log messages (generated based on verbosity settings during loading)
    Log_Parse           = LogChannel_Parse | 0,
    Log_ParseShape      = LogChannel_Parse | 1,
    Log_ParseMorphShape = LogChannel_Parse | 2,
    Log_ParseAction     = LogChannel_Parse | 3,
    // Action
    Log_Action          = LogChannel_Action | 0
};



// This class is used as a mix-in base to classes that provide GetLog() function.
// It adds convenience inlines to the class that can be used for logging.

template<class Derived>
class LogBase
{
public:
    virtual ~LogBase () { }

    // *** Parse Configuration

    // These definitions control whether a certain type of logging is enabled
    // or disabled. The functions should be overriden in Derived class if they
    // need to return a programmer-specified value.
    // One can think of these functions as "template-virtual".

//  bool    IsVerboseParse() const              { return 0; }
//  bool    IsVerboseParseShape() const         { return 0; }
//  bool    IsVerboseParseMorphShape() const    { return 0; }
//  bool    IsVerboseParseAction() const        { return 0; }
//  bool    IsVerboseAction() const             { return 0; }


#ifdef SF_DOM
    // Declarations for DOM only (not compiled by C++)
    // Need to put them here since DOM does not handle declarations below correctly.
    virtual bool    IsVerboseParse() const              { return 0; }
    virtual bool    IsVerboseParseShape() const         { return 0; }
    virtual bool    IsVerboseParseMorphShape() const    { return 0; }
    virtual bool    IsVerboseParseAction() const        { return 0; }
    virtual bool    IsVerboseAction() const             { return 0; }

    // Convenient printf-style functions.
    inline  void    LogError(const char* pfmt, ...);
    inline  void    LogWarning(const char* pfmt, ...);
    inline  void    LogMessage(const char* pfmt, ...);
    // Dynamic conditional printf-style functions.
    // Parsing - controls by VerboseParse flags in loader
    inline  void    LogParse(const char* pfmt, ...);
    inline  void    LogParseShape(const char* pfmt, ...);
    inline  void    LogParseMorphShape(const char* pfmt, ...);
    inline  void    LogParseAction(const char* pfmt, ...);
    // Action - controller by Verbose action flags in log
    inline  void    LogAction(const char* pfmt, ...);
    inline  void    LogScriptError(const char* pfmt, ...);
    inline  void    LogScriptWarning(const char* pfmt, ...);
    inline  void    LogScriptMessage(const char* pfmt, ...);
#endif

    // Tell DOM to ignore declarations below:
    // DOM-IGNORE-BEGIN

protected:
#ifndef GFX_VERBOSE_PARSE
    bool    P_IsVerboseParse() const            { return 0; }
    bool    P_IsVerboseParseShape() const       { return 0; }
    bool    P_IsVerboseParseMorphShape() const  { return 0; }
    bool    P_IsVerboseParseAction() const      { return 0; }
#else

    // NOTE: If we get 'IsVerboseParse' undefined error here, it means that the corresponding
    // VerboseParse method was called on an object that doesn't define IsVerboseParse.
    bool    P_IsVerboseParse() const            { return ((Derived*)this)->IsVerboseParse(); }

#ifndef GFX_VERBOSE_PARSE_SHAPE
    bool    P_IsVerboseParseShape() const       { return 0; }
#else
    bool    P_IsVerboseParseShape() const       { return ((Derived*)this)->IsVerboseParseShape(); }
#endif

#ifndef GFX_VERBOSE_PARSE_MORPHSHAPE
    bool    P_IsVerboseParseMorphShape() const  { return 0; }
#else
    bool    P_IsVerboseParseMorphShape() const  { return ((Derived*)this)->IsVerboseParseMorphShape(); }
#endif

#ifndef GFX_VERBOSE_PARSE_ACTION
    bool    P_IsVerboseParseAction() const      { return 0; }
#else
    bool    P_IsVerboseParseAction() const      { return ((Derived*)this)->IsVerboseParseAction(); }
#endif

#endif

#ifndef GFX_VERBOSE_PARSE_ACTION
    bool    P_IsVerboseAction() const           { return 0; }
#else
    bool    P_IsVerboseAction() const           { return ((Derived*)this)->IsVerboseAction(); }
#endif

#ifndef GFX_VERBOSE_PARSE_ACTION
    bool    P_IsVerboseActionErrors() const     { return 1; }
#else
    bool    P_IsVerboseActionErrors() const     { return ((Derived*)this)->IsVerboseActionErrors(); }
#endif
// DOM-IGNORE-END
    virtual bool    IsVerboseActionErrors() const   { return true; } //??

public:

    // Inline used to short-circuit "if (pLog)" conditional in log functions.
    // Implement it in base class to return 1 if GetLog is always 
    // guaranteed to return a valid pointer.
    bool    IsLogNotNull() const                { return 0; }
    

    // *** Non-Conditional (always working) logging lines
// DOM-IGNORE-BEGIN
    // Convenient printf-style functions.
    GFXLOG_MESSAGE_FUNC_DECL(LogError,          Log_Error)
    GFXLOG_MESSAGE_FUNC_DECL(LogWarning,        Log_Warning)
    GFXLOG_MESSAGE_FUNC_DECL(LogMessage,        Log_Message)

    // Dynamic conditional printf-style functions.
    // Parsing - controls by VerboseParse flags in loader
    GFXLOG_MESSAGE_FUNC_DECL(LogParse,          Log_Parse)
    GFXLOG_MESSAGE_FUNC_DECL(LogParseShape,     Log_ParseShape)
    GFXLOG_MESSAGE_FUNC_DECL(LogParseMorphShape,Log_ParseMorphShape)
    GFXLOG_MESSAGE_FUNC_DECL(LogParseAction,    Log_ParseAction)
    // Action - controller by Verbose action flags in log
    GFXLOG_MESSAGE_FUNC_DECL(LogAction,         Log_Action)
    GFXLOG_MESSAGE_FUNC_DECL(LogScriptError,    Log_ScriptError)
    GFXLOG_MESSAGE_FUNC_DECL(LogScriptWarning,  Log_ScriptWarning)
    GFXLOG_MESSAGE_FUNC_DECL(LogScriptMessage,  Log_ScriptMessage)
// DOM-IGNORE-END
};


// Expose Scaleform constants as GFx ones for convenience.
typedef Scaleform::LogMessageId LogMessageId;
typedef Scaleform::Log Log;


// ***** LogState

// Log wrapper state used in GFx. Internal for the most part,
// users should create Log objects and call GFx::Loader::SetLog() instead.

class LogState : public State, public LogBase<LogState>
{
    Ptr<Log> pLog;
public: 
    LogState(Log* log) : State(State_Log), pLog(log) { }
    virtual ~LogState () { }

    // This virtual function receives all the messages,
    // developers should override this function in order to do custom logging
    virtual void    LogMessageVarg(LogMessageId messageType, const char* pfmt, va_list argList);

    // Call the logging function with specific message type, with no type filtering.
    void            LogMessageByType(LogMessageId messageType, const char* pfmt, ...);

    // GFxLogBase implementation (use const_cast for compatibility with all other derived::GetLog declarations)
    Log*    GetLog() const          { return pLog ? pLog.GetPtr() : Log::GetGlobalLog(); }
    bool    IsLogNotNull() const    { return 1; }
};

GFXLOG_MESSAGE_FUNC(LogError,           Log_Error)
GFXLOG_MESSAGE_FUNC(LogWarning,         Log_Warning)
GFXLOG_MESSAGE_FUNC(LogMessage,         Log_Message)

GFXLOG_MESSAGE_FUNC_COND(LogScriptError,    IsVerboseActionErrors, Log_ScriptError)
GFXLOG_MESSAGE_FUNC_COND(LogScriptWarning,  IsVerboseActionErrors, Log_ScriptWarning)
GFXLOG_MESSAGE_FUNC(LogScriptMessage,   Log_ScriptMessage)

GFXLOG_MESSAGE_FUNC_COND(LogParse,  IsVerboseParse, Log_Parse)
GFXLOG_MESSAGE_FUNC_COND(LogParseShape, IsVerboseParseShape, Log_ParseShape)
GFXLOG_MESSAGE_FUNC_COND(LogParseMorphShape, IsVerboseParseMorphShape, Log_ParseMorphShape)
GFXLOG_MESSAGE_FUNC_COND(LogParseAction, IsVerboseParseAction, Log_ParseAction)
GFXLOG_MESSAGE_FUNC_COND(LogAction, IsVerboseAction, Log_Action)


char* Format(char* buf, size_t bufSize, const Render::Matrix2F&);
char* Format(char* buf, size_t bufSize, const Render::Cxform&);
char* Format(char* buf, size_t bufSize, const Render::Color&);


}} // Scaleform::GFx


#endif // INC_SF_GFX_LOG_H

