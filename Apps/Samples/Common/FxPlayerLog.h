/**************************************************************************

Filename    :   FxPlayerLog.h
Content     :   Log with color fomatting and counts.
Created     :   January 15, 2008
Authors     :   Michael Antonov, Andrew Reisse, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxPlayerLog_H
#define INC_FxPlayerLog_H

#include "GFx.h"
#include "GFx_AMP.h"
#include "GFx_Kernel.h"
#include "Kernel/SF_MsgFormat.h"

#ifdef SF_OS_WIN32
#include <Windows.h>
#endif

using namespace Scaleform;

// We support color-formatted log only on PC/MAC/Linux platforms,
// on consoles we use regular log through vprintf.
#if (defined (SF_OS_WIN32) && !defined(_DURANGO)) || defined(SF_OS_LINUX) || defined(SF_OS_UNIX) || defined(SF_OS_MAC)

#if !defined(SF_OS_WIN32)
#include <unistd.h> // Because of isatty().
#endif

namespace SF = Scaleform;
using namespace GFx;
/*  GFxPlayerLog - Output log with message counter filtering.

This class implements Log by outputting messages to screen or a redirected
file; it receives messages by overriding the LogMessageVarg virtual function.

If repetitive script error messages arrive, they are combined into one and a
counter is displayed next to a message. Such message counting and filtering
is done DisplayCountedMessage. If counting of messages is not necessary,
vprintf(pfmt, argList) could be used instead of DisplayCountedMessage.
*/

class GFxPlayerLog : public SF::Log
{
public:

    // Variables used for repeated message detection.
    enum { Log_MaxBuffer = 2048 };
    char    LastMessageBuffer[Log_MaxBuffer];
    int     LastMessageLength;
    int     LastMessageCounter;
    int     CounterStrLength;
    bool    LastMessageHadNewline;
    bool    IsFileMode;

#ifdef SF_OS_WIN32
    // Colors used.
    enum ConsoleColor
    {
        ConsoleColor_Gray   = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE,
        ConsoleColor_White  = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY,
        ConsoleColor_Yellow = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY,
        ConsoleColor_Cyan   = FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY,
        ConsoleColor_Green  = FOREGROUND_GREEN|FOREGROUND_INTENSITY,
        ConsoleColor_Red    = FOREGROUND_RED|FOREGROUND_INTENSITY,

        ConsoleColor_Unset // should be last
    };
    // Console output handle and active colors.
    HANDLE          hOutput;
    ConsoleColor    Color, LastColor;

    bool GetFileMode()
    {
        hOutput    = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD Mode;
        return GetConsoleMode(hOutput, &Mode) == 0;
    }

    // Delayed color update - useful to reduce overhead of SetConsoleTextAttribute calls.
    void BeginColor()
    {
        if (!IsFileMode && Color != LastColor)
        {
            ::SetConsoleTextAttribute(hOutput, (WORD)Color);
            LastColor = Color;
        }
    }
    void EndColor()
    {
    }

#else
    // Colors used.
    enum ConsoleColor
    {
        ConsoleColor_Gray   = 0,
        ConsoleColor_White  = 37 | 0x280100,
        ConsoleColor_Yellow = 33 | 0x280100,
        ConsoleColor_Cyan   = 36 | 0x100,
        ConsoleColor_Red    = 36 | 0x1000, //? revise
        ConsoleColor_Green  = 36 | 0x10000,//? revise

        ConsoleColor_Unset // should be last
    };
    // Console output handle and active colors.
    ConsoleColor    Color, LastColor;

    bool GetFileMode()
    {
        return 0==isatty(1);
    }

    void BeginColor()
    {
        if (IsFileMode)
            return;

        if (Color & 0xff0000)
            printf("\x1b[%02d;%02d;%02dm", (Color >> 8) & 0xff, (Color >> 16), Color & 0xff);
        else
            printf("\x1b[%02d;%02dm", (Color >> 8) & 0xff, Color & 0xff);
    }
    void EndColor()
    {
        if (IsFileMode)
            return;

        printf("\x1b[00m");
    }

#endif

    void SetTextColor(ConsoleColor attributes)
    {
        Color = attributes;
    }

    GFxPlayerLog()
    {
        LastMessageLength = 0;
        LastMessageHadNewline = 1;

        // We do custom processing if we are in console (screen-output) mode.
        //  - In screen mode we display the message immediately and use '\b'
        //    character to overwrite the message counter as duplicate messages arrive.
        //  - In file mode, '\b' doesn't work, so we wait until the next message
        //    before outputting the counter.
        IsFileMode = GetFileMode();
        // Default colors.
        Color = ConsoleColor_Gray;
        LastColor = ConsoleColor_Unset;
    }

    ~GFxPlayerLog()
    {
        // Finish last counted log line, if any.
        if (LastMessageLength && LastMessageHadNewline)
        {
            if (IsFileMode && (LastMessageCounter > 1))
                DisplayCounter(LastMessageCounter);
            LogPrint("\n",1);
        }
        SetTextColor(ConsoleColor_Gray);
        BeginColor();
    }

    void LogPrint (const char* buff, unsigned length = 0)
    {
#ifdef SF_OS_WIN32
        if (IsFileMode)
        {
            fputs(buff,stdout);
        }
        else
        {
            if (!length)
                length = (unsigned)SFstrlen(buff);

            HANDLE hConsoleOutput = GetStdHandle( STD_OUTPUT_HANDLE );

            wchar_t buf[256];
            wchar_t* pbuf;
            if (length < sizeof(buf)/sizeof(buf[0]))
                pbuf = buf;
            else
                pbuf = (wchar_t*)SF_ALLOC(sizeof(buf[0])*(length + 1), Stat_Default_Mem);

            length = (unsigned)UTF8Util::DecodeStringSafe(pbuf, length + 1, buff, length);
            DWORD ret;
            WriteConsoleW(hConsoleOutput, pbuf, length, &ret, NULL);

            if (pbuf != buf)
                SF_FREE(pbuf);
        }
#else
        SF_UNUSED(length);
        fputs(buff,stdout);
#endif
    }

    static bool MessageNeedsCounting(SF::LogMessageId messageId)
    {
        return ((int)messageId.GetMessageType() == (int)Log_ScriptError) || ((int)messageId.GetMessageType() == (int)Log_ScriptWarning);
    }


    // Displays counter message and returns the number of characters in it.
    static int  DisplayCounter(int counter, int backspaceCount = 0)
    {
        char    backspaceBuff[16], counterBuff[32];
        char*   pbackspace;
        int     i, counterLength;

        // Concatenate backspaces to output string; it's too slow to display them
        // individually through printf.
        for (i = backspaceCount, pbackspace = backspaceBuff; i > 0; pbackspace++,  i--)
            *pbackspace = '\b';
        *pbackspace = 0;

        counterLength = (int)Format(
            StringDataPtr(counterBuff, sizeof(counterBuff)),
            "{0} (x{1})",
            backspaceBuff,
            counter
            );

        printf("%s", counterBuff);
        return counterLength - backspaceCount;
    }

    void    DisplayCountedMessage(SF::LogMessageId messageId, const char* pfmtStr, va_list argList)
    {
        BeginColor();
        char buffer[MaxLogBufferMessageSize];
        char messageBuffer[Log_MaxBuffer];
        FormatLog(buffer, sizeof(buffer), messageId, pfmtStr, argList);
        int     messageLength;

        // If previous message exist and we don't need counting, just flush and output.
        if (!MessageNeedsCounting(messageId))
        {
            if (LastMessageLength && LastMessageHadNewline)
            {
                //printf("\n");
                LogPrint("\n",1);
                LastMessageLength = 0;
            }
            // Output new message.
            messageLength = (int)SFsprintf(messageBuffer, Log_MaxBuffer-1, "%s", buffer);
            //vprintf(pfmt, argList);
            LogPrint(messageBuffer, messageLength);
            SF_AMP_CODE(AmpServer::GetInstance().SendLog(messageBuffer, messageLength, messageId);)
            EndColor();
            return;
        }

        // Messages might need counting, so do so.

        messageLength = (int)SFsprintf(messageBuffer, Log_MaxBuffer-1, "%s", buffer);

        if (messageLength > 0)
        {
            messageBuffer[messageLength] = 0;

            if (!LastMessageHadNewline)
            {
                // If last message didn't have a newline, just concatenate directly.
                // This is a part of the previous message and we don't count these.
                //printf("%s", messageBuffer);
                LogPrint(messageBuffer, messageLength);
                SF_AMP_CODE(AmpServer::GetInstance().SendLog(messageBuffer, messageLength, messageId);)
                if (messageBuffer[messageLength-1] == '\n')
                {
                    LastMessageHadNewline = 1;
                    LastMessageLength = 0;
                }
            }
            else
            {
                bool haveNewline = 0;
                if (messageBuffer[messageLength-1] == '\n')
                {
                    messageBuffer[messageLength-1] = 0;
                    haveNewline = 1;
                }

                // Consider counting logic.
                if (!LastMessageLength || !haveNewline)
                {
                    // No last message, just output us as-is.
                    //printf("%s", messageBuffer);
                    LogPrint(messageBuffer, messageLength);
                    SF_AMP_CODE(AmpServer::GetInstance().SendLog(messageBuffer, messageLength, messageId);)
                    SFstrcpy(LastMessageBuffer, Log_MaxBuffer, messageBuffer);
                    LastMessageLength       = messageLength;
                    LastMessageCounter      = 1;
                    LastMessageHadNewline   = haveNewline;
                    CounterStrLength        = 0;
                }
                else
                {
                    // We had last message: see if they are the same.
                    if (SFstrcmp(LastMessageBuffer, messageBuffer) == 0)
                    {
                        // Strings match, so erase old count and update counter.
                        LastMessageCounter++;

                        // Update and display new counter.
                        if (!IsFileMode)
                        {
                            CounterStrLength = DisplayCounter(LastMessageCounter, CounterStrLength);
                            //SF_AMP_CODE(AmpServer::GetInstance().SendLog(messageBuffer, messageLength, messageId);)
                        }
                    }
                    else
                    {
                        // No match. Output newline followed by the new string.
                        if (IsFileMode && (LastMessageCounter > 1))
                            DisplayCounter(LastMessageCounter);

                        //printf("\n%s", messageBuffer);
                        LogPrint("\n",1);
                        LogPrint(messageBuffer, messageLength);
                        SF_AMP_CODE(AmpServer::GetInstance().SendLog(messageBuffer, messageLength, messageId);)

                        SFstrcpy(LastMessageBuffer, Log_MaxBuffer, messageBuffer);
                        LastMessageLength       = messageLength;
                        LastMessageCounter      = 1;
                        LastMessageHadNewline   = 1;
                        CounterStrLength        = 0;
                    }
                }
            }
        }

        EndColor();
    }


    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(SF::LogMessageId messageId, const char* pfmt, va_list argList)
    {
        // Output log to console.
        // Highlight script messages for readability.
        switch(messageId.GetChannel())
        {
        case LogChannel_Script:
            if (messageId.GetMessageType() == LogMessage_Text)
                SetTextColor(ConsoleColor_Yellow);
            else if (messageId.GetMessageType() == LogMessage_Error)
                SetTextColor(ConsoleColor_Red);
            else if (messageId.GetMessageType() == LogMessage_Warning)
                SetTextColor(ConsoleColor_Green);
            else // brighter white
                SetTextColor(ConsoleColor_White);
            break;

        case LogChannel_Parse:
            SetTextColor(ConsoleColor_Gray);
            break;
        case LogChannel_Action:
            SetTextColor(ConsoleColor_White);
            break;

        default:
            SetTextColor(ConsoleColor_Cyan);
        }

        // Non-counted alternative: vprintf(pfmt, argList).
        DisplayCountedMessage(messageId, pfmt, argList);

        // Back to normal.
        if ((messageId.GetChannel() == LogChannel_Script) || (messageId.GetMessageType() == LogMessage_Text))
            SetTextColor(ConsoleColor_Gray);
    }
};

#else

class GFxPlayerLog : public Scaleform::GFx::Log
{
public:
    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(Scaleform::LogMessageId messageType, const char* pfmt, va_list argList)
    {
        // Output log to console
        vprintf(pfmt, argList);
    }
};


#endif

class GFxPlayerXmlLog : public Scaleform::GFx::Log
{
public:
    // We override this function in order to do custom logging.
    virtual void LogMessageVarg(Scaleform::LogMessageId messageType, const char* pfmt, va_list argList)
    {
        String tagProperty;
        switch (messageType.GetChannel())
        {
        case LogChannel_General:
        default:
            tagProperty = "General";
            break;
        case LogChannel_Debug:
            tagProperty = "Debug";
            break;
        case LogChannel_Render:
            tagProperty = "Render";
            break;
        case LogChannel_Script:
            tagProperty = "Script";
            break;
        case LogChannel_Parse:
            tagProperty = "Parse";
            break;
        case LogChannel_Action:
            tagProperty = "Action";
            break;
        case LogChannel_Memory:
            tagProperty = "Memory";
            break;
        case LogChannel_Performance:
            tagProperty = "Performance";
            break;
        }

        String tagName;
        switch (messageType.GetMessageType())
        {
        case LogMessage_Text:
        default:
            tagName = "Log";
            break;
        case LogMessage_Warning:
            tagName = "Warning";
            break;
        case LogMessage_Error:
            tagName = "Error";
            break;
        case LogMessage_Assert:
            tagName = "Assert";
            break;
        case LogMessage_Report:
            tagName = "Report";
            break;
        }

        String xmlFormat;
        Format(xmlFormat, "<{0} Channel=\"{1}\">{2}</{3}>\n", tagName.ToCStr(), tagProperty.ToCStr(), pfmt, tagName.ToCStr());

        // Output log to console
        vprintf(xmlFormat.ToCStr(), argList);
    }
};

#endif // INC_FxPlayerLog_H
