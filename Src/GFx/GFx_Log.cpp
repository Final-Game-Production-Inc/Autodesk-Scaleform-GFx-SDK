/**************************************************************************

Filename    :   GFx_Log.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_Log.h"
#include "Kernel/SF_Std.h"
#include "Kernel/SF_MsgFormat.h"

#include <stdio.h>
#include <stdarg.h>

#if defined(SF_OS_WIN32)
    #include <windows.h>
#endif

namespace Scaleform { namespace GFx {

// This virtual function receives all the messages  
void LogState::LogMessageVarg(LogMessageId messageType, const char* fmt, va_list argList)
{   
    Log* log = GetLog();
    if (log)
    {
        log->LogMessageVarg(messageType, fmt, argList);
    }
}

void LogState::LogMessageByType(LogMessageId messageType, const char* pfmt, ...)
{
    va_list argList; va_start(argList, pfmt);
    LogMessageVarg(messageType, pfmt, argList);
    va_end(argList);
}


void StateBag::SetLogState(LogState *log)
{
    SetState(State::State_Log, log);
}
Ptr<LogState> StateBag::GetLogState() const
{
    return *(LogState*)GetStateAddRef(State::State_Log);
}

void StateBag::SetLog(Log *log)
{
    Ptr<LogState> logState = *SF_NEW LogState(log);
    SetState(State::State_Log, logState);
}
Ptr<Log> StateBag::GetLog() const
{
    Ptr<LogState> logState = *(LogState*)GetStateAddRef(State::State_Log);
    return logState->GetLog();
}


char* Format(char* buf, size_t bufSize, const Render::Matrix2F& matrix)
{
    Format(StringDataPtr(buf, bufSize),
        "| {0:4.4} {1:4.4} {2:4.4} |\n"
        "| {3:4.4} {4:4.4} {5:4.4} |\n", 
        matrix.Sx(), matrix.Shx(), Render::TwipsToPixels(matrix.Tx()),
        matrix.Shy(), matrix.Sy(), Render::TwipsToPixels(matrix.Ty())
        );
    return buf;
}

char* Format(char* buf, size_t bufSize, const Render::Cxform& cxform)
{
    Scaleform::Format(StringDataPtr(buf, bufSize),
        "    *         +\n"
        "| {0:4.4} {1:4.4}|\n"
        "| {2:4.4} {3:4.4}|\n"
        "| {4:4.4} {5:4.4}|\n"
        "| {6:4.4} {7:4.4}|\n",
        cxform.M[0][0], cxform.M[1][0],
        cxform.M[0][1], cxform.M[1][1],
        cxform.M[0][2], cxform.M[1][2],
        cxform.M[0][3], cxform.M[1][3]
    );
    return buf;
}

char* Format(char* buf, size_t bufSize, const Render::Color& color)
{
    Scaleform::Format(StringDataPtr(buf, bufSize),
        "RGBA: {0} {1} {2} {3}\n",
        (int)color.Channels.Red, 
        (int)color.Channels.Green, 
        (int)color.Channels.Blue, 
        (int)color.Channels.Alpha
        );
    return buf;
}

}} // Scaleform::GFx
