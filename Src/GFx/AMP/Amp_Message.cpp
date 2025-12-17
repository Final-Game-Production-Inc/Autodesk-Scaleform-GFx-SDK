/**************************************************************************

Filename    :   Amp_Message.cpp
Content     :   Messages sent back and forth to AMP
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Amp_Message.h"

#ifdef SF_ENABLE_STATS
#include "Kernel/SF_HeapNew.h"
#include "Amp_Stream.h"
#include "Amp_Interfaces.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_AmpInterface.h"
#include "Kernel/SF_MsgFormat.h"

#ifdef SF_ENABLE_ZLIB
#include "../../../3rdParty/zlib-1.3.1/zlib.h"
#endif

namespace Scaleform {
namespace GFx {
namespace AMP {


void Message::ReadString(File& inFile, String* str)
{
    str->Clear();
    UInt32 stringLength = inFile.ReadUInt32();
    for (UInt32 i = 0; i < stringLength; ++i)
    {
        str->AppendChar(inFile.ReadSByte());
    }
}

void Message::WriteString(File& outFile, const String& str)
{
    outFile.WriteUInt32(static_cast<UInt32>(str.GetLength()));
    for (UPInt i = 0; i < str.GetLength(); ++i)
    {
        outFile.WriteSByte(str[i]);
    }
}

String Message::MsgTypeToMsgName(UInt32 msgType)
{
    String msgTypeName;
    switch (msgType)
    {
    case Msg_Heartbeat:
        msgTypeName = MessageHeartbeat::GetStaticTypeName();
        break;
    case Msg_Log:
        msgTypeName = MessageLog::GetStaticTypeName();
        break;
    case Msg_CurrentState:
        msgTypeName = MessageCurrentState::GetStaticTypeName();
        break;
    case Msg_ProfileFrame:
        msgTypeName = MessageProfileFrame::GetStaticTypeName();
        break;
    case Msg_SwdFile:
        msgTypeName = MessageSwdFile::GetStaticTypeName();
        break;
    case Msg_SourceFile:
        msgTypeName = MessageSourceFile::GetStaticTypeName();
        break;
    case Msg_ObjectsReport:
        msgTypeName = MessageObjectsReport::GetStaticTypeName();
        break;
    case Msg_SwdRequest:
        msgTypeName = MessageSwdRequest::GetStaticTypeName();
        break;
    case Msg_SourceRequest:
        msgTypeName = MessageSourceRequest::GetStaticTypeName();
        break;
    case Msg_ObjectsReportRequest:
        msgTypeName = MessageObjectsReportRequest::GetStaticTypeName();
        break;
    case Msg_AppControl:
        msgTypeName = MessageAppControl::GetStaticTypeName();
        break;
    case Msg_InitState:
        msgTypeName = MessageInitState::GetStaticTypeName();
        break;
    case Msg_Port:
        msgTypeName = MessagePort::GetStaticTypeName();
        break;
    case Msg_ImageRequest:
        msgTypeName = MessageImageRequest::GetStaticTypeName();
        break;
    case Msg_ImageData:
        msgTypeName = MessageImageData::GetStaticTypeName();
        break;
    case Msg_FontRequest:
        msgTypeName = MessageFontRequest::GetStaticTypeName();
        break;
    case Msg_FontData:
        msgTypeName = MessageFontData::GetStaticTypeName();
        break;
    case Msg_Compressed:
        msgTypeName = MessageCompressed::GetStaticTypeName();
        break;
    default:
        break;
    }
    return msgTypeName;
}

Message::PlatformType Message::GetLocalPlatform()
{
#if defined(SF_OS_WIN32)
    return PlatformWindows;
#elif defined(SF_OS_LINUX)
    return PlatformLinux;
#elif defined(SF_OS_MAC)
    return PlatformMac;
#elif defined(SF_OS_PS3)
    return PlatformPs3;
#elif defined(SF_OS_XBOX360)
    return PlatformXbox360;
#elif defined(SF_OS_WII)
    return PlatformWii;
#elif defined(SF_OS_3DS)
    return Platform3DS;
#elif defined(SF_OS_ANDROID)
    return PlatformAndroid;
#elif defined(SF_OS_IPHONE)
    return PlatformIphone;
#elif defined(SF_OS_WIIU)
    return PlatformWiiU;
#elif defined(SF_OS_PSVITA)
    return PlatformNgp;
#else
    return PlatformOther;
#endif
}


// Constructor
Message::Message() : Version(Version_Latest), GFxVersion(4)
{
}

// Serialization
void Message::Read(File& str)
{
    UByte msgType = str.ReadUByte();
    if (msgType == 0)
    {
        String msgTypeName;
        ReadString(str, &msgTypeName);
    }
    Version = str.ReadUInt32();
    if (Version >= 22)
    {
        GFxVersion = str.ReadUByte();
    }
}

// Serialization
void Message::Write(File& str) const
{
    if (Version < 28)
    {
        str.WriteUByte(static_cast<UByte>(GetMessageType()));
    }
    else
    {
        str.WriteUByte(0);
        WriteString(str, GetMessageName());
    }
    str.WriteUInt32(Version);
    if (Version >= 22)
    {
        str.WriteUByte(GFxVersion);
    }
}

#ifdef SF_ENABLE_ZLIB
// Memory allocation functions for ZLib
static voidpf ZLibAllocFunc_AMP(voidpf opaque, uInt items, uInt size)
{
    return SF_HEAP_AUTO_ALLOC(opaque, size * items);
}

static void ZLibFreeFunc_AMP(voidpf, voidpf address)
{
    SF_FREE(address);
}
#endif

bool Message::Compress(Array<UByte>& compressedData) const
{
#ifdef SF_ENABLE_ZLIB
    static const unsigned zlibOutBufferSize = 1024;
    UByte zlibOutBuffer[zlibOutBufferSize];
    z_stream strm;
    strm.zalloc = ZLibAllocFunc_AMP;
    strm.zfree = ZLibFreeFunc_AMP;
    strm.opaque = (voidpf) this;

    int ret = deflateInit(&strm, Z_BEST_SPEED);
    SF_DEBUG_MESSAGE1(ret != Z_OK, "Zlib deflateInit failed with code %d", ret);
    if (ret != Z_OK)
    {
        return false;
    }

    Ptr<AmpStream> serializedMsg = *SF_HEAP_AUTO_NEW(this) AmpStream();
    Write(*serializedMsg);
    strm.avail_in = static_cast<uInt>(serializedMsg->GetBufferSize());
    strm.next_in = const_cast<UByte*>(serializedMsg->GetBuffer());

    do 
    {
        strm.avail_out = zlibOutBufferSize;
        strm.next_out = zlibOutBuffer;

        ret = deflate(&strm, Z_FINISH);
        SF_ASSERT(ret != Z_STREAM_ERROR);

        unsigned addSize = zlibOutBufferSize - strm.avail_out;
        for (unsigned i = 0; i < addSize; ++i)
        {
            compressedData.PushBack(zlibOutBuffer[i]);
        }

    } while (strm.avail_out == 0);
    SF_ASSERT(ret == Z_STREAM_END);
    deflateEnd(&strm);

    return true;
#else
	SF_UNUSED(compressedData);
    return false;
#endif
}


////////////////////////////////////////////////////////////////////

// Constructor
MessageInitState::MessageInitState(UInt32 initStateFlags, UInt32 initProfileLevel)
    : InitStateFlags(initStateFlags)
    , InitProfileLevel(initProfileLevel)
{
}

// Accessor
UInt32 MessageInitState::GetInitStateFlags() const
{
    return InitStateFlags;
}

// Accessor
UInt32 MessageInitState::GetInitProfileLevel() const
{
    return InitProfileLevel;
}

// Serialization
void MessageInitState::Read(File& str)
{
    Message::Read(str);
    InitStateFlags = str.ReadUInt32();
    InitProfileLevel = str.ReadUInt32();
}

// Serialization
void MessageInitState::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(InitStateFlags);
    str.WriteUInt32(InitProfileLevel);
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageLog::MessageLog(const String& logText, UInt32 logCategory, const UInt64 timeStamp)
{
    SetLog(logText, logCategory, timeStamp);
}

// Serialization
void MessageLog::Read(File& str)
{
    Message::Read(str);
    ReadString(str, &LogText);
    LogCategory = str.ReadUInt32();
    ReadString(str, &TimeStamp);

    if (Version <= 2)
    {
        for (int i = 0; i < 128; ++i)
        {
            str.ReadUInt32();
        }
    }
}

// Serialization
void MessageLog::Write(File& str) const
{
    Message::Write(str);
    WriteString(str, LogText);
    str.WriteUInt32(LogCategory);
    WriteString(str, TimeStamp);

    if (Version <= 2)
    {
        for (int i = 0; i < 128; ++i)
        {
            str.WriteUInt32(0);
        }
    }
}

// Accessor
const StringLH& MessageLog::GetLogText() const
{
    return LogText;
}

// Accessor
// Category of the log message. Error, warning, informational, etc
UInt32 MessageLog::GetLogCategory() const
{
    return LogCategory;
}

// Accessor
const StringLH& MessageLog::GetTimeStamp() const
{
    return TimeStamp;
}

void MessageLog::SetLog(const String& logText, UInt32 logCategory, const UInt64 timeStamp)
{
    LogText = logText;
    LogCategory = logCategory;

    static const int textSize = 9;
    char timeStampText[textSize];
    UInt32 seconds = static_cast<UInt32>(timeStamp % 60);
    UInt32 minutes = static_cast<UInt32>((timeStamp / 60) % 60);
    UInt32 hours = static_cast<UInt32>((timeStamp / 3600) % 24);
    SFsprintf(timeStampText, textSize, "%02u:%02u:%02u", hours, minutes, seconds);
    TimeStamp = timeStampText;
}


/////////////////////////////////////////////////////////////////////////

// Constructor
MessageCurrentState::MessageCurrentState(const ServerState* state)
{
    State = *SF_HEAP_AUTO_NEW(this) ServerState();
    if (state != NULL)
    {
        *State = *state;
    }
}

// Serialization
void MessageCurrentState::Read(File& str)
{
    Message::Read(str);
    State->Read(str, Version);
}

// Serialization
void MessageCurrentState::Write(File& str) const
{
    Message::Write(str);
    State->Write(str, Version);
}

/////////////////////////////////////////////////////////////////////////

// Constructor
MessageProfileFrame::MessageProfileFrame(Ptr<ProfileFrame> frameInfo) : 
    FrameInfo(frameInfo)
{
}

// Serialization
void MessageProfileFrame::Read(File& str)
{
    Message::Read(str);
    FrameInfo = *SF_HEAP_AUTO_NEW(this) ProfileFrame();
    FrameInfo->Read(str, Version);
}

// Serialization
void MessageProfileFrame::Write(File& str) const
{
    Message::Write(str);
    SF_ASSERT(FrameInfo);
    FrameInfo->Write(str, Version);
}

// Accessor
const ProfileFrame* MessageProfileFrame::GetFrameInfo() const
{
    return FrameInfo;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageSwdFile::MessageSwdFile(UInt32 swfHandle, UByte* bufferData, 
                               unsigned bufferSize, const char* filename) : 
    Handle(swfHandle), 
    Filename(filename)
{
    FileData.Resize(bufferSize);
    for (UPInt i = 0; i < bufferSize; ++i)
    {
        FileData[i] = *(bufferData + i);
    }
}

// Serialization
void MessageSwdFile::Read(File& str)
{
    Message::Read(str);
    Handle = str.ReadUInt32();
    UInt32 dataSize = str.ReadUInt32();
    FileData.Resize(dataSize);
    for (UInt32 i = 0; i < dataSize; ++i)
    {
        FileData[i] = str.ReadUByte();
    }
    ReadString(str, &Filename);
}

// Serialization
void MessageSwdFile::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(Handle);
    str.WriteUInt32(static_cast<UInt32>(FileData.GetSize()));
    for (UInt32 i = 0; i < FileData.GetSize(); ++i)
    {
        str.WriteUByte(FileData[i]);
    }
    WriteString(str, Filename);
}

UInt32 MessageSwdFile::GetHandle() const
{
    return Handle;
}

// Accessor
const ArrayLH<UByte>& MessageSwdFile::GetFileData() const
{
    return FileData;
}

// Accessor
const char* MessageSwdFile::GetFilename() const
{
    return Filename;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageSourceFile::MessageSourceFile(UInt64 fileHandle, UByte* bufferData, 
                                     unsigned bufferSize, const char* filename) : 
    FileHandle(fileHandle),
    Filename(filename)
{
    FileData.Resize(bufferSize);
    for (UPInt i = 0; i < bufferSize; ++i)
    {
        FileData[i] = *(bufferData + i);
    }
}

// Serialization
void MessageSourceFile::Read(File& str)
{
    Message::Read(str);
    FileHandle = str.ReadUInt64();
    UInt32 dataSize = str.ReadUInt32();
    FileData.Resize(dataSize);
    for (UInt32 i = 0; i < dataSize; ++i)
    {
        FileData[i] = str.ReadUByte();
    }
    ReadString(str, &Filename);
}

// Serialization
void MessageSourceFile::Write(File& str) const
{
    Message::Write(str);

    str.WriteUInt64(FileHandle);
    str.WriteUInt32(static_cast<UInt32>(FileData.GetSize()));
    for (UInt32 i = 0; i < FileData.GetSize(); ++i)
    {
        str.WriteUByte(FileData[i]);
    }
    WriteString(str, Filename);
}


// Accessor
UInt64 MessageSourceFile::GetFileHandle() const
{
    return FileHandle;
}

// Accessor
const ArrayLH<UByte>& MessageSourceFile::GetFileData() const
{
    return FileData;
}

// Accessor
const char* MessageSourceFile::GetFilename() const
{
    return Filename;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageObjectsReport::MessageObjectsReport(const char* objectsReport,
    Ptr<AmpMovieObjectDesc> root, UInt32 movieHandle) : 
    ObjectsReport(objectsReport != NULL ? objectsReport : ""),
    ObjectsRoot(root),
    MovieHandle(movieHandle)
{
}

// Serialization
void MessageObjectsReport::Read(File& str)
{
    Message::Read(str);
    ReadString(str, &ObjectsReport);
    if (Version >= 36)
    {
        UInt32 numRoots = str.ReadUInt32();
        for (UInt32 i = 0; i < numRoots; ++i)
        {
            ObjectsRoot = *SF_HEAP_AUTO_NEW(this) AmpMovieObjectDesc();
            ObjectsRoot->Read(str);
        }
    }
    if (Version >= 39)
    {
        MovieHandle = str.ReadUInt32();
    }
}

// Serialization
void MessageObjectsReport::Write(File& str) const
{
    Message::Write(str);
    WriteString(str, ObjectsReport);
    if (Version >= 36)
    {
        if (ObjectsRoot)
        {
            str.WriteUInt32(1);
            ObjectsRoot->Write(str);
        }
        else
        {
            str.WriteUInt32(0);
        }
    }
    if (Version >= 39)
    {
        str.WriteUInt32(MovieHandle);
    }
}

// Accessor
const char* MessageObjectsReport::GetObjectsReport() const
{
    return ObjectsReport;
}

// Accessor
const Ptr<AmpMovieObjectDesc> MessageObjectsReport::GetObjectsRoot() const
{
    return ObjectsRoot;
}

UInt32 MessageObjectsReport::GetMovieHandle() const
{
    return MovieHandle;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageSwdRequest::MessageSwdRequest(UInt32 swfHandle, bool requestContents) : 
    Handle(swfHandle), 
    RequestContents(requestContents)
{
}

// Serialization
void MessageSwdRequest::Read(File& str)
{
    Message::Read(str);
    Handle = str.ReadUInt32();
    RequestContents = (str.ReadUByte() != 0);
}

// Serialization
void MessageSwdRequest::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(Handle);
    str.WriteUByte(RequestContents ? 1 : 0);
}

// Accessor
bool MessageSwdRequest::IsRequestContents() const
{
    return RequestContents;
}

// Accessor
UInt32 MessageSwdRequest::GetHandle() const
{
    return Handle;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageSourceRequest::MessageSourceRequest(UInt64 handle, bool requestContents) : 
    FileHandle(handle),
    RequestContents(requestContents)
{
}

// Serialization
void MessageSourceRequest::Read(File& str)
{
    Message::Read(str);
    FileHandle = str.ReadUInt64();
    RequestContents = (str.ReadUByte() != 0);
}

// Serialization
void MessageSourceRequest::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt64(FileHandle);
    str.WriteUByte(RequestContents ? 1 : 0);
}

// Accessor
UInt64 MessageSourceRequest::GetFileHandle() const
{
    return FileHandle;
}

// Accessor
bool MessageSourceRequest::IsRequestContents() const
{
    return RequestContents;
}


////////////////////////////////////////////////////////////////////

// Constructor
MessageObjectsReportRequest::MessageObjectsReportRequest() : 
    MovieHandle(0),
    ShortFilenames(false),
    NoCircularReferences(false),
    SuppressOverallStats(false), 
    AddressesForAnonymObjsOnly(false),
    SuppressMovieDefsStats(false),
    NoEllipsis(false)
{
}

// Constructor
MessageObjectsReportRequest::MessageObjectsReportRequest(UInt32 movieHandle,
                                                         bool shortFilenames, 
                                                         bool noCircularReferences,
                                                         bool suppressOverallStats,
                                                         bool addressesForAnonymObjsOnly,
                                                         bool suppressMovieDefsStats,
                                                         bool noEllipsis) : 
    MovieHandle(movieHandle),
    ShortFilenames(shortFilenames),
    NoCircularReferences(noCircularReferences),
    SuppressOverallStats(suppressOverallStats), 
    AddressesForAnonymObjsOnly(addressesForAnonymObjsOnly),
    SuppressMovieDefsStats(suppressMovieDefsStats),
    NoEllipsis(noEllipsis)
{
}

// Serialization
void MessageObjectsReportRequest::Read(File& str)
{
    Message::Read(str);
    MovieHandle = str.ReadUInt32();
    ShortFilenames = (str.ReadUByte() != 0);
    NoCircularReferences = (str.ReadUByte() != 0);
    SuppressOverallStats = (str.ReadUByte() != 0);
    AddressesForAnonymObjsOnly = (str.ReadUByte() != 0);
    SuppressMovieDefsStats = (str.ReadUByte() != 0);
    NoEllipsis = (str.ReadUByte() != 0);
}

// Serialization
void MessageObjectsReportRequest::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(MovieHandle);
    str.WriteUByte(ShortFilenames ? 1 : 0);
    str.WriteUByte(NoCircularReferences ? 1 : 0);
    str.WriteUByte(SuppressOverallStats ? 1 : 0);
    str.WriteUByte(AddressesForAnonymObjsOnly ? 1 : 0);
    str.WriteUByte(SuppressMovieDefsStats ? 1 : 0);
    str.WriteUByte(NoEllipsis ? 1 : 0);
}

// Accessor
UInt32 MessageObjectsReportRequest::GetMovieHandle() const
{
    return MovieHandle;
}

// Accessor
bool MessageObjectsReportRequest::IsShortFilenames() const
{
    return ShortFilenames;
}

// Accessor
bool MessageObjectsReportRequest::IsNoCircularReferences() const
{
    return NoCircularReferences;
}
// Accessor
bool MessageObjectsReportRequest::IsSuppressOverallStats() const
{
    return SuppressOverallStats;
}
// Accessor
bool MessageObjectsReportRequest::IsAddressesForAnonymObjsOnly() const
{
    return AddressesForAnonymObjsOnly;
}
// Accessor
bool MessageObjectsReportRequest::IsSuppressMovieDefsStats() const
{
    return SuppressMovieDefsStats;
}
// Accessor
bool MessageObjectsReportRequest::IsNoEllipsis() const
{
    return NoEllipsis;
}


////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////

// Constructor
MessageAppControl::MessageAppControl(UInt32 flags) : 
    OptionBits(flags), ProfileLevel(Amp_Profile_Level_Null), BatchChange(0)
{
}

// Serialization
void MessageAppControl::Read(File& str)
{
    Message::Read(str);
    OptionBits = str.ReadUInt32();
    ReadString(str, &LoadMovieFile);
    if (Version >= 20)
    {
        ProfileLevel = str.ReadSInt32();
    }
    if (Version >= 41)
    {
        BatchChange = str.ReadSInt32();
    }
}

// Serialization
void MessageAppControl::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(OptionBits);
    WriteString(str, LoadMovieFile);
    if (Version >= 20)
    {
        str.WriteSInt32(ProfileLevel);
    }
    if (Version >= 41)
    {
        str.WriteSInt32(BatchChange);
    }
}

// Flags accessor
UInt32 MessageAppControl::GetFlags() const
{
    return OptionBits;
}

// Setting this flag toggles wireframe mode on the app
bool MessageAppControl::IsToggleWireframe() const
{
    return (OptionBits & OF_ToggleWireframe) != 0;
}

// Setting this flag toggles wireframe mode on the app
void MessageAppControl::SetToggleWireframe(bool wireframeToggle)
{
    if (wireframeToggle)
    {
        OptionBits |= OF_ToggleWireframe;
    }
    else
    {
        OptionBits &= (~OF_ToggleWireframe);
    }
}

// Setting this flag toggles pause on the movie
bool MessageAppControl::IsTogglePause() const
{
    return (OptionBits & OF_TogglePause) != 0;
}

// Setting this flag toggles pause on the movie
void MessageAppControl::SetTogglePause(bool pauseToggle)
{
    if (pauseToggle)
    {
        OptionBits |= OF_TogglePause;
    }
    else
    {
        OptionBits &= (~OF_TogglePause);
    }
}

// Setting this flag toggles fast forward on the movie
bool MessageAppControl::IsToggleFastForward() const
{
    return (OptionBits & OF_ToggleFastForward) != 0;
}

// Setting this flag toggles fast forward on the movie
void MessageAppControl::SetToggleFastForward(bool ffToggle)
{
    if (ffToggle)
    {
        OptionBits |= OF_ToggleFastForward;
    }
    else
    {
        OptionBits &= (~OF_ToggleFastForward);
    }
}

// Setting this flag toggles between edge anti-aliasing modes
bool MessageAppControl::IsToggleAaMode() const
{
    return (OptionBits & OF_ToggleAaMode) != 0;
}

// Setting this flag toggles between edge anti-aliasing modes
void MessageAppControl::SetToggleAaMode(bool aaToggle)
{
    if (aaToggle)
    {
        OptionBits |= OF_ToggleAaMode;
    }
    else
    {
        OptionBits &= (~OF_ToggleAaMode);
    }
}

// Setting this flag toggles between stroke types
bool MessageAppControl::IsToggleStrokeType() const
{
    return (OptionBits & OF_ToggleStrokeType) != 0;
}

// Setting this flag toggles between stroke types
void MessageAppControl::SetToggleStrokeType(bool strokeToggle)
{
    if (strokeToggle)
    {
        OptionBits |= OF_ToggleStrokeType;
    }
    else
    {
        OptionBits &= (~OF_ToggleStrokeType);
    }
}

// Setting this flag restarts the flash movie on the app
bool MessageAppControl::IsRestartMovie() const
{
    return (OptionBits & OF_RestartMovie) != 0;
}

// Setting this flag restarts the flash movie on the app
void MessageAppControl::SetRestartMovie(bool restart)
{
    if (restart)
    {
        OptionBits |= OF_RestartMovie;
    }
    else
    {
        OptionBits &= (~OF_RestartMovie);
    }
}

// Setting this flag cycles through the font configurations 
bool MessageAppControl::IsNextFont() const
{
    return (OptionBits & OF_NextFont) != 0;
}

// Setting this flag cycles through the font configurations 
void MessageAppControl::SetNextFont(bool next)
{
    if (next)
    {
        OptionBits |= OF_NextFont;
    }
    else
    {
        OptionBits &= (~OF_NextFont);
    }
}

// Setting this flag increases the curve tolerance
bool MessageAppControl::IsCurveToleranceUp() const
{
    return (OptionBits & OF_CurveToleranceUp) != 0;
}

// Setting this flag increases the curve tolerance
void MessageAppControl::SetCurveToleranceUp(bool up)
{
    if (up)
    {
        OptionBits |= OF_CurveToleranceUp;
    }
    else
    {
        OptionBits &= (~OF_CurveToleranceUp);
    }
}

// Setting this flag decreases the curve tolerance
bool MessageAppControl::IsCurveToleranceDown() const
{
    return (OptionBits & OF_CurveToleranceDown) != 0;
}

// Setting this flag decreases the curve tolerance
void MessageAppControl::SetCurveToleranceDown(bool down)
{
    if (down)
    {
        OptionBits |= OF_CurveToleranceDown;
    }
    else
    {
        OptionBits &= (~OF_CurveToleranceDown);
    }
}

bool MessageAppControl::IsDebugPause() const
{
    return (OptionBits & OF_DebugPause) != 0;
}

void MessageAppControl::SetDebugPause(bool debug)
{
    if (debug)
    {
        OptionBits |= OF_DebugPause;
    }
    else
    {
        OptionBits &= (~OF_DebugPause);
    }
}

bool MessageAppControl::IsDebugStep() const
{
    return (OptionBits & OF_DebugStep) != 0;
}

void MessageAppControl::SetDebugStep(bool debug)
{
    if (debug)
    {
        OptionBits |= OF_DebugStep;
    }
    else
    {
        OptionBits &= (~OF_DebugStep);
    }
}

bool MessageAppControl::IsDebugStepIn() const
{
    return (OptionBits & OF_DebugStepIn) != 0;
}

void MessageAppControl::SetDebugStepIn(bool debug)
{
    if (debug)
    {
        OptionBits |= OF_DebugStepIn;
    }
    else
    {
        OptionBits &= (~OF_DebugStepIn);
    }
}

bool MessageAppControl::IsDebugStepOut() const
{
    return (OptionBits & OF_DebugStepOut) != 0;
}

void MessageAppControl::SetDebugStepOut(bool debug)
{
    if (debug)
    {
        OptionBits |= OF_DebugStepOut;
    }
    else
    {
        OptionBits &= (~OF_DebugStepOut);
    }
}

bool MessageAppControl::IsDebugNextMovie() const
{
    return (OptionBits & OF_DebugNextMovie) != 0;
}

void MessageAppControl::SetDebugNextMovie(bool debug)
{
    if (debug)
    {
        OptionBits |= OF_DebugNextMovie;
    }
    else
    {
        OptionBits &= (~OF_DebugNextMovie);
    }
}

bool MessageAppControl::IsToggleMemReport() const
{
    return (OptionBits & OF_ToggleMemReport) != 0;

}

void MessageAppControl::SetToggleMemReport(bool toggle)
{
    if (toggle)
    {
        OptionBits |= OF_ToggleMemReport;
    }
    else
    {
        OptionBits &= (~OF_ToggleMemReport);
    }
}

// Setting this flag toggles amp sending profile messages every frame
bool MessageAppControl::IsToggleAmpRecording() const
{
    return (OptionBits & OF_ToggleAmpRecording) != 0;
}

// Setting this flag toggles amp sending profile messages every frame
void MessageAppControl::SetToggleAmpRecording(bool recordingToggle)
{
    if (recordingToggle)
    {
        OptionBits |= OF_ToggleAmpRecording;
    }
    else
    {
        OptionBits &= (~OF_ToggleAmpRecording);
    }
}

// Setting this flag toggles amp renderer to draw hot spots
bool MessageAppControl::IsToggleOverdraw() const
{
    return (OptionBits & OF_ToggleOverdraw) != 0;
}

// Setting this flag toggles amp renderer to draw hot spots
void MessageAppControl::SetToggleOverdraw(bool overdrawToggle)
{
    if (overdrawToggle)
    {
        OptionBits |= OF_ToggleOverdraw;
    }
    else
    {
        OptionBits &= (~OF_ToggleOverdraw);
    }
}

// Setting this flag toggles amp renderer to draw hot spots
bool MessageAppControl::IsToggleBatch() const
{
    return (OptionBits & OF_ToggleBatch) != 0;
}

// Setting this flag toggles amp renderer to draw hot spots
void MessageAppControl::SetToggleBatch(bool batchToggle)
{
    if (batchToggle)
    {
        OptionBits |= OF_ToggleBatch;
    }
    else
    {
        OptionBits &= (~OF_ToggleBatch);
    }
}

bool MessageAppControl::IsToggleBlending() const
{
    return (OptionBits & OF_ToggleBlending) != 0;
}

void MessageAppControl::SetToggleBlending(bool blendingToggle)
{
    if (blendingToggle)
    {
        OptionBits |= OF_ToggleBlending;
    }
    else
    {
        OptionBits &= (~OF_ToggleBlending);
    }
}

bool MessageAppControl::IsToggleTextureDensity() const
{
    return (OptionBits & OF_ToggleTextureDensity) != 0;
}

void MessageAppControl::SetToggleTextureDensity(bool texDensityToggle)
{
    if (texDensityToggle)
    {
        OptionBits |= OF_ToggleTextureDensity;
    }
    else
    {
        OptionBits &= (~OF_ToggleTextureDensity);
    }
}

// Setting this flag toggles instruction profiling for per-line timings
bool MessageAppControl::IsToggleInstructionProfile() const
{
    return (OptionBits & OF_ToggleInstructionProfile) != 0;
}

// Setting this flag toggles instruction profiling for per-line timings
void MessageAppControl::SetToggleInstructionProfile(bool instToggle)
{
    if (instToggle)
    {
        OptionBits |= OF_ToggleInstructionProfile;
    }
    else
    {
        OptionBits &= (~OF_ToggleInstructionProfile);
    }
}

// Accessor
const StringLH& MessageAppControl::GetLoadMovieFile() const
{
    return LoadMovieFile;
}

// Accessor
void MessageAppControl::SetLoadMovieFile(const char* pcFileName)
{
    LoadMovieFile = pcFileName;
}

// Accessor
SInt32 MessageAppControl::GetProfileLevel() const
{
    return ProfileLevel;
}

// Accessor
void MessageAppControl::SetProfileLevel(SInt32 profileLevel)
{
    ProfileLevel = profileLevel;
}

SInt32 MessageAppControl::GetBatchChange() const
{
    return BatchChange;
}

void MessageAppControl::SetBatchChange(SInt32 change)
{
    BatchChange = change;
}


///////////////////////////////////////////////////////////////////////////////////

MessagePort::MessagePort(UInt32 port, const char* appName, const char* fileName) :
    Port(port)
{
    if (appName != NULL)
    {
        AppName = appName;
    }
    if (fileName != NULL)
    {
        FileName = fileName;
    }
    Platform = GetLocalPlatform();
}

// Serialization
void MessagePort::Read(File& str)
{
    Message::Read(str);
    Port = str.ReadUInt32();
    ReadString(str, &AppName);
    if (Version >= 5)
    {
        Platform = static_cast<PlatformType>(str.ReadUInt32());
        ReadString(str, &FileName);
    }
}

// Serialization
void MessagePort::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(Port);
    WriteString(str, AppName);
    if (Version >= 5)
    {
        str.WriteUInt32(Platform);
        WriteString(str, FileName);
    }
}

// Accessor
UInt32 MessagePort::GetPort() const
{
    return Port;
}

UInt32 MessagePort::GetAddress() const
{
    return Address;
}

const StringLH& MessagePort::GetPeerName() const
{
    return PeerName;
}

// Accessor
const StringLH& MessagePort::GetAppName() const
{
    return AppName;
}

// Accessor
const StringLH& MessagePort::GetFileName() const
{
    return FileName;
}

// Accessor
Message::PlatformType MessagePort::GetPlatform() const
{
    return Platform;
}

void MessagePort::SetPeerName(const char* name)
{
    PeerName = name;
}

void MessagePort::SetPeerAddress(UInt32 address)
{
    Address = address;
}

void MessagePort::SetPlatform(PlatformType platform)
{
    Platform = platform;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageImageRequest::MessageImageRequest(UInt32 imageId) : 
    ImageId(imageId)
{
}

// Serialization
void MessageImageRequest::Read(File& str)
{
    Message::Read(str);
    ImageId = str.ReadUInt32();
}

// Serialization
void MessageImageRequest::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(ImageId);
}

UInt32 MessageImageRequest::GetImageId() const
{
    return ImageId;
}


////////////////////////////////////////////////////////////////////

// Constructor
MessageImageData::MessageImageData(UInt32 imageId) : 
    ImageId(imageId), PngFormat(true)
{
    ImageDataStream = SF_HEAP_AUTO_NEW(this) AmpStream();
}

// Destructor
MessageImageData::~MessageImageData()
{
    ImageDataStream->Release();
}

// Serialization
void MessageImageData::Read(File& str)
{
    Message::Read(str);
    ImageId = str.ReadUInt32();
    ImageDataStream->Read(str);
    if (Version >= 26)
    {
        PngFormat = (str.ReadUByte() != 0);
    }
}

// Serialization
void MessageImageData::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(ImageId);
    if (Version >= 26 || PngFormat)
    {
        ImageDataStream->Write(str);
    }
    else
    {
        str.WriteUInt32(0);
    }
    if (Version >= 26)
    {
        str.WriteUByte(PngFormat ? 1 : 0);
    }
}


// Accessor
UInt32 MessageImageData::GetImageId() const
{
    return ImageId;
}

// Accessor
AmpStream* MessageImageData::GetImageData() const
{
    return ImageDataStream;
}

void MessageImageData::SetImageData(AmpStream* imageData)
{
    ImageDataStream->Release();
    ImageDataStream = imageData;
    ImageDataStream->AddRef();
}

bool MessageImageData::IsPngFormat() const
{
    return PngFormat;
}

void MessageImageData::SetPngFormat(bool pngFormat)
{
    PngFormat = pngFormat;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageFontRequest::MessageFontRequest(UInt32 fontId) : 
    FontId(fontId)
{
}

// Serialization
void MessageFontRequest::Read(File& str)
{
    Message::Read(str);
    FontId = str.ReadUInt32();
}

// Serialization
void MessageFontRequest::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(FontId);
}

UInt32 MessageFontRequest::GetFontId() const
{
    return FontId;
}


////////////////////////////////////////////////////////////////////

// Constructor
MessageFontData::MessageFontData(UInt32 fontId) : 
    FontId(fontId),
    NumTextures(1)
{
    FontDataStream = SF_HEAP_AUTO_NEW(this) AmpStream();
}

// Destructor
MessageFontData::~MessageFontData()
{
    FontDataStream->Release();
}

// Serialization
void MessageFontData::Read(File& str)
{
    Message::Read(str);
    FontId = str.ReadUInt32();
    if (Version >= 38)
    {
        NumTextures = str.ReadUInt32();
    }
    FontDataStream->Read(str);
}

// Serialization
void MessageFontData::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(FontId);
    if (Version >= 38)
    {
        str.WriteUInt32(NumTextures);
    }
    FontDataStream->Write(str);
}

// Accessor
UInt32 MessageFontData::GetFontId() const
{
    return FontId;
}

// Accessor
UInt32 MessageFontData::GetNumTextures() const
{
    return NumTextures;
}

// Accessor
void MessageFontData::SetNumTextures(UInt32 numTextures)
{
    NumTextures = numTextures;
}

// Accessor
AmpStream* MessageFontData::GetImageData() const
{
    return FontDataStream;
}

// Accessor
void MessageFontData::SetImageData(AmpStream* imageData)
{
    FontDataStream->Release();
    FontDataStream = imageData;
    FontDataStream->AddRef();
}

/////////////////////////////////////////////////////////////////////

void MessageCompressed::Read(File& str)
{
    Message::Read(str);
    UInt32 dataSize = str.ReadUInt32();
    CompressedData.Resize(dataSize);
    for (UInt32 i = 0; i < dataSize; ++i)
    {
        CompressedData[i] = str.ReadUByte();
    }
}

void MessageCompressed::Write(File& str) const
{
    Message::Write(str);
    str.WriteUInt32(static_cast<UInt32>(CompressedData.GetSize()));
    for (UInt32 i = 0; i < CompressedData.GetSize(); ++i)
    {
        str.WriteUByte(CompressedData[i]);
    }
}

bool MessageCompressed::Uncompress(Array<UByte>& uncompressedData)
{
#ifdef SF_ENABLE_ZLIB
    z_stream strm;
    strm.zalloc = ZLibAllocFunc_AMP;
    strm.zfree = ZLibFreeFunc_AMP;
    strm.opaque = (voidpf) this;
    strm.avail_in = static_cast<uInt>(CompressedData.GetSize());
    strm.next_in = &CompressedData[0];
    int ret = inflateInit(&strm);
    SF_DEBUG_MESSAGE1(ret != Z_OK, "Zlib inflateInit failed with code %d", ret);
    if (ret != Z_OK)
    {
        return false;
    }

    unsigned currentPosition = 0;
    static const unsigned chunkSize = 1024;

    do 
    {
        uncompressedData.Resize(currentPosition + chunkSize);
        strm.avail_out = chunkSize;
        strm.next_out = (&uncompressedData[0] + currentPosition);

        ret = inflate(&strm, Z_NO_FLUSH);
        SF_ASSERT(ret != Z_STREAM_ERROR);  /* state not clobbered */

        currentPosition += chunkSize - strm.avail_out;

    } while (strm.avail_out == 0);

    uncompressedData.Resize(currentPosition);
    inflateEnd(&strm);

    return true;
#else
	SF_UNUSED(uncompressedData);
    return false;
#endif
}

void MessageCompressed::AddCompressedData(UByte* data, UPInt dataSize)
{
    for (UPInt i = 0; i < dataSize; ++i)
    {
        CompressedData.PushBack(data[i]);
    }
}

const ArrayLH<UByte>& MessageCompressed::GetCompressedData() const
{
    return CompressedData;
}



} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif
