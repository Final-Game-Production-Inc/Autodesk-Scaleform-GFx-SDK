/**************************************************************************

Filename    :   Amp_Server.cpp
Content     :   Class encapsulating communication with AMP
                Embedded in application to be profiled
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef SF_AMP_SERVER

#include "Amp_Server.h"
#include "Kernel/SF_HeapNew.h"
#include "Amp_ThreadMgr.h"
#include "GFx/GFx_ASMovieRootBase.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_LoadProcess.h"
#include "Amp_ViewStats.h"
#include "Amp_ServerCallbacks.h"
#include "Amp_Visitors.h"
#include "Amp_Stream.h"
#include "Amp_MessageRegistry.h"
#include "Amp_ObjectsLog.h"
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_MemItem.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/Renderer2DImpl.h"
#include "Render/Render_GlyphCache.h"
#include "Render/Render_MeshCache.h"
#include <time.h>

#define SF_AMP_DEBUG_HEAP

namespace Scaleform {
namespace GFx {
namespace AMP {


class AppControlMsgHandler : public IMessageHandler
{
public:
    AppControlMsgHandler(Server* ampServer) : AmpSrv(ampServer) { }
    virtual void Handle(Message* message)
    {
        SF_ASSERT(message->GetMessageName() == MessageAppControl::GetStaticTypeName());
        AmpSrv->HandleAppControl(static_cast<MessageAppControl*>(message));
    }

private:
    Server* AmpSrv;
};

class InitStateMsgHandler : public IMessageHandler
{
public:
    InitStateMsgHandler(Server* ampServer) : AmpSrv(ampServer) { }
    virtual void Handle(Message* message)
    {
        SF_ASSERT(message->GetMessageName() == MessageInitState::GetStaticTypeName());
        AmpSrv->HandleInitState(static_cast<MessageInitState*>(message));
    }

private:
    Server* AmpSrv;
};

class SwdRequestMsgHandler : public IMessageHandler
{
public:
    SwdRequestMsgHandler(Server* ampServer) : AmpSrv(ampServer) { }
    virtual void Handle(Message* message)
    {
        SF_ASSERT(message->GetMessageName() == MessageSwdRequest::GetStaticTypeName());
        AmpSrv->HandleSwdRequest(static_cast<MessageSwdRequest*>(message));
    }

private:
    Server* AmpSrv;
};

class SourceRequestMsgHandler : public IMessageHandler
{
public:
    SourceRequestMsgHandler(Server* ampServer) : AmpSrv(ampServer) { }
    virtual void Handle(Message* message)
    {
        SF_ASSERT(message->GetMessageName() == MessageSourceRequest::GetStaticTypeName());
        AmpSrv->HandleSourceRequest(static_cast<MessageSourceRequest*>(message));
    }

private:
    Server* AmpSrv;
};

class ObjectsReportRequestMsgHandler : public IMessageHandler
{
public:
    ObjectsReportRequestMsgHandler(Server* ampServer) : AmpSrv(ampServer) { }
    virtual void Handle(Message* message)
    {
        SF_ASSERT(message->GetMessageName() == MessageObjectsReportRequest::GetStaticTypeName());
        AmpSrv->HandleObjectsReportRequest(static_cast<MessageObjectsReportRequest*>(message));
    }

private:
    Server* AmpSrv;
};

class ImageRequestMsgHandler : public IMessageHandler
{
public:
    ImageRequestMsgHandler(Server* ampServer) : AmpSrv(ampServer) { }
    virtual void Handle(Message* message)
    {
        SF_ASSERT(message->GetMessageName() == MessageImageRequest::GetStaticTypeName());
        AmpSrv->HandleImageRequest(static_cast<MessageImageRequest*>(message));
    }

private:
    Server* AmpSrv;
};

class FontRequestMsgHandler : public IMessageHandler
{
public:
    FontRequestMsgHandler(Server* ampServer) : AmpSrv(ampServer) { }
    virtual void Handle(Message* message)
    {
        SF_ASSERT(message->GetMessageName() == MessageFontRequest::GetStaticTypeName());
        AmpSrv->HandleFontRequest(static_cast<MessageFontRequest*>(message));
    }

private:
    Server* AmpSrv;
};

// Static
// AMP Server initialization
void Server::Init()
{
#ifndef SF_AMP_DEBUG_HEAP
    // So memory stats include AMP
    MemoryHeap* heap = Memory::GetGlobalHeap()->CreateHeap("AMP");
#else
    MemoryHeap* heap = Memory::GetGlobalHeap()->CreateHeap("AMP", MemoryHeap::Heap_UserDebug);
#endif
    heap->SetLimit(1024 * 1024);
    Server* serverInstance = SF_HEAP_NEW(heap) Server();
    AmpServerSingleton = serverInstance;
    heap->ReleaseOnFree(serverInstance);
    SF_DEBUG_MESSAGE(true, "AMP Server initialized\n");
}

// Static
// AMP Server uninitialization
void Server::Uninit()
{
    static_cast<Server*>(AmpServerSingleton)->Release();
    AmpServerSingleton = NULL;
    SF_DEBUG_MESSAGE(true, "AMP Server uninitialized\n");
}

////////////////////////////////////////////////

// Returns true if there is a valid socket connection to the AMP client
bool Server::IsValidConnection() const
{
    return (SocketThreadMgr->IsValidConnection());
}

// State accessor
// Checks for paused or disabled state
bool Server::IsState(ServerStateType state) const
{
    Lock::Locker locker(&CurrentStateLock);
    return ((CurrentState.StateFlags & state) != 0);
}

// State accessor
// Sets the specified state (paused, disabled, etc)
void Server::SetState(ServerStateType state, bool stateValue, bool sendState)
{
    if (stateValue != IsState(state))
    {
        ToggleAmpState(state, sendState);
    }
}

// State accessor
// Checks for level of function profiling
AmpProfileLevel Server::GetProfileLevel() const
{
    Lock::Locker locker(&CurrentStateLock);
    return static_cast<AmpProfileLevel>(CurrentState.ProfileLevel);
}

// Toggles the specified state, allowing multiple states to be set or unset at once
void Server::ToggleAmpState(UInt32 toggleState, bool sendState)
{
    if (toggleState != 0)
    {
        Lock::Locker locker(&CurrentStateLock);
        SetAmpState(CurrentState.StateFlags ^ toggleState, sendState);
    }
}

// Sets the specified state, allowing multiple states to be set or unset at once
void Server::SetAmpState(UInt32 newState, bool sendState)
{
    Lock::Locker locker(&CurrentStateLock);
    bool disabledChanged = (Amp_Disabled & newState) != (Amp_Disabled & CurrentState.StateFlags);

    if (MemReportLocked)
    {
        if (IsState(Amp_MemoryReports))
        {
            newState |= Amp_MemoryReports;
        }
        else
        {
            newState &= ~Amp_MemoryReports;
        }
    }

    CurrentState.StateFlags = newState;

    if (disabledChanged)
    {
        if (IsState(Amp_Disabled))
        {
            CloseConnection();
        }
        else
        {
            OpenConnection();
        }            
    }

    UpdateProfilingState();
    if (sendState)
    {
        SendCurrentState();
    }
}

// Sets the function profiling level. The higher the level, the more detail you get
// lock = true locks the profile level so that it cannot be changed by the AMP client
void Server::SetProfileLevel(AmpProfileLevel profileLevel, bool lock)
{
    if (!ProfileLevelLocked || lock)
    {
        Lock::Locker locker(&CurrentStateLock);
        CurrentState.ProfileLevel = profileLevel;
        if (lock)
        {
            ProfileLevelLocked = 1;
        }
        SetState(Amp_NoFunctionAggregation, profileLevel >= Amp_Profile_Level_High, false);
    }
    SendCurrentState();
}

// Turns on or off the detailed memory reporting
// lock = true locks the memory report level so that it cannot be changed by the AMP client
void Server::SetMemReports(bool memReports, bool lock)
{
    if (!MemReportLocked || lock)
    {
        SetState(Amp_MemoryReports, memReports, true);
        if (lock)
        {
            MemReportLocked = 1;
        }
    }
}


UInt32 Server::GetCurrentState() const
{
    Lock::Locker locker(&CurrentStateLock);
    return CurrentState.StateFlags;
}


void Server::SetConnectedApp(const char* playerTitle)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState.ConnectedApp != playerTitle)
    {
        CurrentState.ConnectedApp = playerTitle;
        SendCurrentState();
    }
}

void Server::SetAaMode(const char* aaMode)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState.AaMode != aaMode)
    {
        CurrentState.AaMode = aaMode;
        SendCurrentState();
    }
}

void Server::SetStrokeType(const char* strokeType)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState.StrokeType != strokeType)
    {
        CurrentState.StrokeType = strokeType;
        SendCurrentState();
    }
}

void Server::SetCurrentLocale(const char* locale)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState.CurrentLocale != locale)
    {
        CurrentState.CurrentLocale = locale;
        SendCurrentState();
    }
}

void Server::SetCurveTolerance(float tolerance)
{
    Lock::Locker locker(&CurrentStateLock);
    if (Alg::Abs(tolerance - CurrentState.CurveTolerance) > 0.001f)
    {
        CurrentState.CurveTolerance = tolerance;
        SendCurrentState();
    }
}

// Updates the state and sends it to AMP
// Useful because not all state components are exposed through individual accessors
// Also, allows changing many components at once, sending only one update message, 
// for example during initialization
void Server::UpdateState(const ServerState* state)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState != *state)
    {
        bool oldMemReport = ((CurrentState.StateFlags & Amp_MemoryReports) != 0);
        SInt32 oldProfileLevel = CurrentState.ProfileLevel;

        CurrentState = *state;

        // Revert memory reports if they are locked
        if (oldMemReport != IsMemReports() && MemReportLocked)
        {
            if (oldMemReport)
            {
                CurrentState.StateFlags |= Amp_MemoryReports;
            }
            else
            {
                CurrentState.StateFlags &= (~Amp_MemoryReports);
            }
        }

        // Revert profile level if it's locked
        if (GetProfileLevel() != oldProfileLevel && ProfileLevelLocked)
        {
            CurrentState.ProfileLevel = oldProfileLevel;
        }

        SendCurrentState();
    }
}


// Can change port so that AMP can distinguish between instances of GFx running on the same address
void Server::SetListeningPort(UInt32 port)
{
    if (Port != port)
    {
        Port = port;
        if (IsSocketCreated())
        {
            CloseConnection();
            OpenConnection();
        }
    }
}

// Set the port where UDP broadcast messages are sent and received
// Has no effect after server had already started
void Server::SetBroadcastPort(UInt32 port)
{
    BroadcastPort = port;
}

// Returns true if currently connected to AMP and sending stats
bool Server::GetProfilingState() const
{ 
    if (RecordingState != Amp_Server_Recording_Off)
    {
        return true;
    }
    
    if (!IsEnabled())
    {
        return false;
    }

    if (IsPaused())
    {
        return false;
    }

    if (!SocketThreadMgr->IsValidSocket())
    {
        return false;
    }

    if (!IsValidConnection())
    {
        return false;
    }

    return true;
}

// Returns the cached value of GetProfilingState
// The cached value gets updated when you call UpdateProfilingState
bool Server::IsProfiling() const
{ 
    return (Profiling != 0);
}

void Server::UpdateProfilingState()
{
    Profiling = GetProfilingState() ? 1 : 0;
}

// Wait until connected, or until the specified time interval has passed
// Used so that an application can start being profiled from the first frame
// since establishing connection can take a little while to establish
void Server::WaitForAmpConnection(unsigned maxDelayMilliseconds)
{
    ConnectedEvent.Wait(maxDelayMilliseconds);
}

// Sets the AMP capabilities supported by the current GFx application
// These capabilities, such as wireframe mode, are implemented outside of GFx
// and are application-specific
void Server::SetAppControlCaps(const MessageAppControl* caps)
{
    AppControlCaps = *SF_HEAP_AUTO_NEW(this) MessageAppControl(caps->GetFlags());
    AppControlCaps->SetLoadMovieFile(caps->GetLoadMovieFile());
}

// Starts/stops stat recording for display when not connected to AMP server
// Used mainly for platforms that do not support socket connections
void Server::ToggleInternalStatRecording()
{
    Lock::Locker lock(&RecordingStateLock);
    switch (RecordingState)
    {
        case Amp_Server_Recording_Off:
        case Amp_Server_Recording_Stopped:
        default:
            RecordingState = Amp_Server_Recording_On;
            break;
        case Amp_Server_Recording_On:
            RecordingState = Amp_Server_Recording_Stopped;
            break;
    }
}

// The server can wait for a connection before proceeding
// This is useful when you want to profile the startup
void Server::SetConnectionWaitTime(unsigned waitTimeMilliseconds)
{
    ConnectionWaitDelay = waitTimeMilliseconds;
}

// If AMP is the only system using sockets, it will initialize the library 
// and release it when it's done.
// SetInitSocketLib(false) means that AMP is going to be using a previously-initialized socket library
void Server::SetInitSocketLib(bool initSocketLib)
{
    InitSocketLib = initSocketLib;
}

// Set the AMP socket implementation factory
// Must be called before a connection has been opened
void Server::SetSocketImplFactory(SocketImplFactory* socketFactory)
{
    SocketThreadMgr->SetSocketImplFactory(socketFactory);
}

// Sets the AMP heap limit in bytes
// When this limit is exceeded, GFx is paused until any pending messages have been sent to AMP,
// thus reducing the heap memory
void Server::SetHeapLimit(UPInt memLimit)
{
    Memory::GetHeapByAddress(this)->SetLimit(memLimit);
}


// Called once per frame by the app
// It is called from GRenderer::EndFrame
// Updates all statistics
void Server::AdvanceFrame()
{
    UpdateProfilingState();

    // Wait for AMP thread to catch up
#if !defined(SF_OS_3DS) && !defined(SF_OS_WII) && !defined(SF_OS_WIIU)
    SendingEvent.Wait(1000);
    bool skipSend = false;
#else
    bool skipSend = !SendingEvent.IsSignaled();
    SF_DEBUG_MESSAGE(skipSend, "Discarding AMP data - send queue too large\n");
#endif

    // Don't bother updating if we have no client connected
    if (IsProfiling() && !skipSend)
    {
        SendFrameStats();
    }
    else
    {
        // Timing stats need to be cleared every frame when not recording
        // Otherwise times will accumulate

        CollectMovieData(NULL);
        ClearRendererData();
        CollectTaskData(NULL);
    }

    {
        Lock::Locker locker(&ToggleStateLock);
        if (ToggleState != 0)
        {
            ToggleAmpState(ToggleState, true);
            ToggleState = 0;
        }

        if (PendingForceState)
        {
            SetAmpState(ForceState, true);
            PendingForceState = false;
        }

        if (PendingProfileLevel != Amp_Profile_Level_Null)
        {
            SetProfileLevel(static_cast<AmpProfileLevel>(PendingProfileLevel), false);
            PendingProfileLevel = Amp_Profile_Level_Null;
        }
    }

    SF_GPA_ID_DESTROY(GpaDomain, GpaGroupId);
    GpaGroupId = SF_GPA_ID_MAKE((void*)GpaStringHandle, 0);
    SF_GPA_ID_CREATE(GpaDomain, GpaGroupId);
    SF_GPA_TASK_GROUP(GpaDomain, GpaGroupId, SF_GPA_ITT_NULL, GpaStringHandle);
}

// Called from movie advance thread
void Server::MovieAdvance(MovieImpl* movie)
{
    Lock::Locker lock(&ObjectsReportLock);
    if (ObjectsReportRequested == movie->GetAdvanceStats().GetViewHandle())
    {
        ObjectsLog objectsReportLog;
        Ptr<AmpMovieObjectDesc> root;
        movie->PrintObjectsReport(ObjectsReportFlags, &objectsReportLog, NULL, &root, Memory::GetHeapByAddress(this));
        SendMessage(SF_HEAP_AUTO_NEW(this) MessageObjectsReport(objectsReportLog.ToCStr(), root, ObjectsReportRequested));
        ObjectsReportRequested = 0;
    }
}

// Optional callback to handle app control messages
void Server::SetAppControlCallback(AppControlInterface* callback)
{
    AppControlCallback = callback;
}

// Called from the app to give a chance to handle new socket messages
// Returns true if a message was found and handled
bool Server::HandleNextMessage()
{
    return SocketThreadMgr->HandleNextReceivedMessage();
}

// Convenience method that wrapps the connection lock and NULL check
void Server::SendMessage(Message* msg)
{
    SocketThreadMgr->SendAmpMessage(msg);
}

// Sends a log message to AMP client
// called directly from the log object
void Server::SendLog(const char* message, int msgLength, LogMessageId msgType)
{
    // create time stamp for log message
    time_t rawTime = 0;
#ifndef SF_OS_3DS
    time(&rawTime);
#endif

    // Send a message with the appropriate category for filtering
    SendMessage(SF_HEAP_AUTO_NEW(this) MessageLog(String(message, msgLength), msgType, rawTime));
}

// Sends the current state to the AMP client so the UI can be properly updated
void Server::SendCurrentState()
{
    Lock::Locker locker(&CurrentStateLock);
    SocketThreadMgr->SetBroadcastInfo(CurrentState.ConnectedApp, CurrentState.ConnectedFile);
    Ptr<ViewStats> currentMovie = GetDebugPausedMovie();
    if (currentMovie)
    {
        CurrentState.CurrentFileId = currentMovie->GetActiveFile();
        CurrentState.CurrentLineNumber = currentMovie->GetActiveLine();
    }
    CurrentState.Platform = Message::GetLocalPlatform();
    SendMessage(SF_HEAP_AUTO_NEW(this) MessageCurrentState(&CurrentState));
}

// Sends the supported AMP capabilities (for example, wireframe mode) to the AMP client
void Server::SendAppControlCaps()
{
    SendMessage(SF_HEAP_AUTO_NEW(this) MessageAppControl(AppControlCaps->GetFlags()));
}

// Handles an app control request message by forwarding it to the callback object
bool Server::HandleAppControl(const MessageAppControl* msg)
{
    Lock::Locker locker(&ToggleStateLock);
    ToggleState = 0;
    if (msg->IsToggleAmpRecording())
    {
        ToggleState |= Amp_Paused;
    }
    if (msg->IsToggleInstructionProfile())
    {
        if (IsInstructionProfiling())
        {
            PendingProfileLevel = Amp_Profile_Level_Low;
        }
        else
        {
            PendingProfileLevel = Amp_Profile_Level_High;
        }
    }
    if (msg->IsToggleMemReport())
    {
        ToggleState |= Amp_MemoryReports;
    }
    if (msg->GetProfileLevel() != Amp_Profile_Level_Null)
    {
        PendingProfileLevel = msg->GetProfileLevel();
    }

    if (msg->IsDebugPause())
    {
        Lock::Locker lock(&MovieLock);
        Ptr<ViewStats> pausedMovie = GetDebugPausedMovie();
        if (pausedMovie)
        {
            pausedMovie->DebugGo();
        }
        else
        {
            if (MovieStats.GetSize() > 0)
            {
                MovieStats[0]->AdvanceTimings->DebugPause();
            }
        }
    }

    {
        Lock::Locker lock(&MovieLock);
        Ptr<ViewStats> pausedMovie = GetDebugPausedMovie();
        if (pausedMovie)
        {
            if (msg->IsDebugNextMovie())
            {
                for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
                {
                    if (MovieStats[i]->AdvanceTimings == pausedMovie)
                    {
                        UPInt next = (i == MovieStats.GetSize() - 1 ? 0 : i + 1);
                        MovieStats[next]->AdvanceTimings->DebugPause();
                        pausedMovie->DebugGo();
                    }
                }
            }
            else if (msg->IsDebugStep())
            {
                pausedMovie->DebugStep(0);
            }
            else if (msg->IsDebugStepIn())
            {
                pausedMovie->DebugStep(1);
            }
            else if (msg->IsDebugStepOut())
            {
                pausedMovie->DebugStep(-1);
            }
        }
    }

    if (AppControlCallback != NULL)
    {
        // Forward to custom handler
        AppControlCallback->HandleAmpRequest(msg);
    }

    return true;
}

bool Server::HandleInitState(const MessageInitState* message)
{
    Lock::Locker lock(&ToggleStateLock);
    ForceState = message->GetInitStateFlags();
    PendingForceState = true;
    PendingProfileLevel = message->GetInitProfileLevel();
    return true;
}

// The server has no knowledge of the SWD format
// but the SWD may be located on the server computer and not the client
// Therefore the server first tries to load the SWD locally 
// If it succeeds, it sends the data, without parsing it, to the client
// If no SWD is found, then it sends the file name for the requested SWD handle
bool Server::HandleSwdRequest(const MessageSwdRequest* msg)
{
    // Get the first file opener found
    Ptr<FileOpenerBase> fileOpener;
    LoaderLock.DoLock();
    for (UPInt i = 0; i < Loaders.GetSize(); ++i)
    {
        fileOpener = Loaders[i]->GetFileOpener();
        if (fileOpener)
        {
            break;
        }
    }
    LoaderLock.Unlock();

    if (fileOpener)
    {
        Lock::Locker locker(&SwfLock);

        UInt32 handle = msg->GetHandle();
        SwdMap::Iterator it = HandleToSwdIdMap.Find(handle);
        if (it != HandleToSwdIdMap.End())
        {
            String filename = GetSwdFilename(handle);
            if (!filename.IsEmpty() && msg->IsRequestContents())
            {
                // Convert .swf into .swd
                String swdFilename = filename;
                UPInt iLength = swdFilename.GetLength();
                if (iLength > 4)
                {
                    String extension = swdFilename.Substring(iLength - 4, iLength);
                    if (extension == ".swf" || extension == ".gfx")
                    {
                        swdFilename = swdFilename.Substring(0, iLength - 4);
                    }
                }
                swdFilename += ".swd";

                // Try to read the SWD
                bool sent = false;

                Ptr<File> swdFile = *fileOpener->OpenFile(swdFilename);
                if (swdFile && swdFile->GetLength() > 0)
                {
                    Array<UByte> fileData(swdFile->GetLength());
                    if (swdFile->Read(&fileData[0], (int)fileData.GetSize()) == swdFile->GetLength())
                    {
                        // File read. Send the data to the client
                        SendMessage(SF_HEAP_AUTO_NEW(this) MessageSwdFile(handle, 
                            &fileData[0], (UInt32)fileData.GetSize(), swdFilename));
                        sent = true;
                    }
                    else
                    {
                        SF_ASSERT(false); // corrupt SWD
                    }
                    swdFile->Close();
                }

                if (!sent)
                {
                    // Send just the SWD file name so the client can try to load it
                    SendMessage(SF_HEAP_AUTO_NEW(this) MessageSwdFile(handle, NULL, 0, swdFilename));
                }
            }
        }
    }

    return true;
}

bool Server::HandleSourceRequest(const MessageSourceRequest* msg)
{
    // Get the first file opener found
    Ptr<FileOpenerBase> fileOpener;
    LoaderLock.DoLock();
    for (UPInt i = 0; i < Loaders.GetSize(); ++i)
    {
        fileOpener = Loaders[i]->GetFileOpener();
        if (fileOpener)
        {
            break;
        }
    }
    LoaderLock.Unlock();

    if (fileOpener)
    {
        Lock::Locker locker(&SourceFileLock);

        UInt64 handle = msg->GetFileHandle();
        SourceFileMap::Iterator it = HandleToSourceFileMap.Find(handle);
        if (it != HandleToSourceFileMap.End())
        {
            String filename = GetSourceFilename(handle);
            if (!filename.IsEmpty() && msg->IsRequestContents())
            {
                // Try to read the file
                bool sent = false;

                Ptr<File> sourceFile = *fileOpener->OpenFile(filename);
                if (sourceFile && sourceFile->GetLength() > 0)
                {
                    Array<UByte> fileData(sourceFile->GetLength());
                    if (sourceFile->Read(&fileData[0], (int)fileData.GetSize()) == sourceFile->GetLength())
                    {
                        // File read. Send the data to the client
                        SendMessage(SF_HEAP_AUTO_NEW(this) MessageSourceFile(handle, 
                            &fileData[0], (UInt32)fileData.GetSize(), filename));
                        sent = true;
                        SF_DEBUG_MESSAGE3(true, "AMP - Sending (%d, %d) source file \"%s\"\n", static_cast<UInt32>(handle >> 32), static_cast<UInt32>(handle), filename.ToCStr());
                    }
                    else
                    {
                        SF_ASSERT(false); // corrupt file
                    }
                    sourceFile->Close();
                }

                if (!sent)
                {
                    // Send just the source file name so the client can try to load it
                    SendMessage(SF_HEAP_AUTO_NEW(this) MessageSourceFile(handle, NULL, 0, filename));
                    SF_DEBUG_MESSAGE1(true, "AMP - Not sending source file \"%s\"\n", filename.ToCStr());
                }
            }
        }
    }

    return true;
}

bool Server::HandleObjectsReportRequest(const MessageObjectsReportRequest* msg)
{
    Lock::Locker locker(&ObjectsReportLock);
    ObjectsReportRequested = msg->GetMovieHandle();
    ObjectsReportFlags = 0;
    if (msg->IsShortFilenames())
    {
        ObjectsReportFlags |= Movie::Report_ShortFileNames;
    }
    if (msg->IsNoCircularReferences())
    {
        ObjectsReportFlags |= Movie::Report_NoCircularReferences;
    }
    if (msg->IsSuppressOverallStats())
    {
        ObjectsReportFlags |= Movie::Report_SuppressOverallStats;
    }
    if (msg->IsAddressesForAnonymObjsOnly())
    {
        ObjectsReportFlags |= Movie::Report_AddressesForAnonymObjsOnly;
    }
    if (msg->IsSuppressMovieDefsStats())
    {
        ObjectsReportFlags |= Movie::Report_SuppressMovieDefsStats;
    }
    if (msg->IsNoEllipsis())
    {
        ObjectsReportFlags |= Movie::Report_NoEllipsis;
    }

    return true;
}

class AmpFileWriter : public ImageFileWriter_Mixin<AmpFileWriter>
{
public:
    AmpFileWriter() : Version(0) { }
    void SetVersion(UInt32 version) { Version = version; }
    virtual bool Write(File* file, const ImageData& imageData, const ImageWriteArgs* args = 0) const
    {
        SF_UNUSED(args);
        imageData.Write(*file, Version);
        return true;
    }

    // Instance singleton.
    static AmpFileWriter Instance;
private:
    UInt32 Version;
};

AmpFileWriter AmpFileWriter::Instance;

MessageImageData* Server::GetImageData(UInt32 imageId) const
{
    MessageImageData* dataMsg = SF_HEAP_AUTO_NEW(this) MessageImageData(imageId);
    Ptr<AmpStream> imageFile = *SF_HEAP_AUTO_NEW(this) AmpStream();

    Lock::Locker lock(&ImageLock);

    for (UPInt i = 0; i < Images.GetSize(); ++i)
    {
        ImageResource* imgRes = Images[i];
        if (imgRes->GetImageId() == imageId)
        {
            Ptr<ImageBase> srcImage = imgRes->GetImage();
            if (srcImage)
            {
                if (!ImageData::IsFormatCompressed(srcImage->GetFormat()))
                {
                    AmpFileWriter::Instance.SetVersion(SocketThreadMgr->GetMsgVersion());
                    if (AmpFileWriter::WriteImage(imageFile, srcImage->GetAsImage()))
                    {
                        if (imageFile)
                        {
                            dataMsg->SetImageData(imageFile);
                            dataMsg->SetPngFormat(false);
                        }
                        return dataMsg;
                    }
                }
            }
        }
    }

    return dataMsg;
}


bool Server::HandleImageRequest(const MessageImageRequest* msg)
{
    MessageImageData* dataMsg = GetImageData(msg->GetImageId());

    if (dataMsg->GetImageData() != NULL || SocketThreadMgr->GetMsgVersion() >= 23)
    {
        SocketThreadMgr->SendAmpMessage(dataMsg);
    }
    return true;
}

bool Server::HandleFontRequest(const MessageFontRequest* msg)
{
    MessageFontData* dataMsg;
    dataMsg = SF_HEAP_AUTO_NEW(this) MessageFontData(msg->GetFontId());

    Ptr<AmpStream> imageFile = *SF_HEAP_AUTO_NEW(this) AmpStream();
    Render::GlyphCache* glyphCache = CurrentRenderer->GetImpl()->GetGlyphCache();

    int numTextures = glyphCache->GetTextureData(imageFile, SocketThreadMgr->GetMsgVersion());
    dataMsg->SetNumTextures(static_cast<UInt32>(numTextures));
    if (numTextures > 0)
    {
        dataMsg->SetImageData(imageFile);
    }
    SendMessage(dataMsg);

    return true;
}


// Adds a movie to the list of movies to be profiled
// Called from GFxMovieRoot constructor
void Server::AddMovie(MovieImpl* movie)
{
    // Don't profile movies on the debug heaps (such as the AMP HUD)
    if (((movie->GetMovieHeap()->GetFlags() & MemoryHeap::Heap_UserDebug) == 0))
    {
        // Open the socket connection, if needed
        if (!IsSocketCreated())
        {
            OpenConnection();
        }

        Lock::Locker locker(&MovieLock);
        Movies.PushBack(movie);

        // Add the stats immediately
        MovieStats.PushBack(*SF_HEAP_AUTO_NEW(this) ViewProfile(movie));
    }
}

// Removes a movie from the list of movies to be profiled
// Called from GFxMovieRoot destructor
void Server::RemoveMovie(MovieImpl* movie)
{
    if (((movie->GetMovieHeap()->GetFlags() & MemoryHeap::Heap_UserDebug) == 0))
    {
        Lock::Locker locker(&MovieLock);
        for (UPInt i = 0; i < Movies.GetSize(); ++i)
        {
            if (Movies[i] == movie)
            {
                Movies.RemoveAt(i);
                // Don't remove the stats yet
                // wait until they have reported
                break;
            }
        }

        // If there are no more movies left to profile, close the socket connection
        if (Movies.GetSize() == 0)
        {
            CloseConnection();        
        }
    }
}

// This method is used to find an instance of Movie by using memory heap
// in order to use proper AmpStats structure.
// note: the returned movie is AddRef-ed!
bool Server::FindMovieByHeap(MemoryHeap* heap, MovieImpl** movie)
{
    SF_ASSERT(movie);
    Lock::Locker locker(&MovieLock);
    for (UPInt i = 0, n = Movies.GetSize(); i < n; ++i)
    {
        if (Movies[i] && Movies[i]->GetHeap() == heap)
        {
            Movies[i]->AddRef();
            *movie = Movies[i];
            return true;
        }
    }
    return false;
}

// Adds an image to the list of images reported by AMP
// Called from Image constructor
void Server::AddImage(ImageResource* image)
{
    // Don't profile movies on the debug heaps (such as the AMP HUD)
    if (((Memory::GetHeapByAddress(image)->GetFlags() & MemoryHeap::Heap_UserDebug) == 0))
    {
        Lock::Locker locker(&ImageLock);
        Images.PushBack(image);
    }
}

// Removes a movie from the list of images reported by AMP
// Called from Image destructor
void Server::RemoveImage(ImageResource* image)
{
    if (((Memory::GetHeapByAddress(image)->GetFlags() & MemoryHeap::Heap_UserDebug) == 0))
    {
        Lock::Locker locker(&ImageLock);
        for (UPInt i = 0; i < Images.GetSize(); ++i)
        {
            if (Images[i] == image)
            {
                Images.RemoveAt(i);
                break;
            }
        }
    }
}

// Adds a loader to the list of active loaders
// Called from the GFxLoader constructor
void Server::AddLoader(Loader* loader)
{
    Lock::Locker locker(&LoaderLock);
    Loaders.PushBack(loader);
}

// Removes a loader from the list of active loaders 
// Called from the GFxLoader destructor
void Server::RemoveLoader(Loader* loader)
{
    Lock::Locker locker(&LoaderLock);
    for (UPInt i = 0; i < Loaders.GetSize(); ++i)
    {
        if (Loaders[i] == loader)
        {
            Loaders.RemoveAt(i);
            break;
        }
    }
}

void Server::AddLoadProcess(LoadProcess* loadProcess)
{
    Lock::Locker locker(&LoadProcessLock);
    LoadProcesses.PushBack(loadProcess);
    RenderProfile* renderProf = SF_HEAP_AUTO_NEW(this) RenderProfile();
    renderProf->DisplayTimings = loadProcess->LoadProcessStats;
    TaskStats.PushBack(*renderProf);
}

void Server::RemoveLoadProcess(LoadProcess* loadProcess)
{
    Lock::Locker locker(&LoadProcessLock);
    for (UPInt i = 0; i < LoadProcesses.GetSize(); ++i)
    {
        if (LoadProcesses[i] == loadProcess)
        {
            LoadProcesses.RemoveAt(i);
            break;
        }
    }
}

void Server::AddSound(UPInt soundMem)
{
    SoundMemory += soundMem;
}

void Server::RemoveSound(UPInt soundMem)
{
    SoundMemory -= soundMem;
}

void Server::AddStrokes(UInt32 numStrokes)
{
    NumStrokes += numStrokes;
}

void Server::RemoveStrokes(UInt32 numStrokes)
{
    NumStrokes -= numStrokes;
}

void Server::IncrementFontFailures()
{
    ++FontFailures;
}

void Server::IncrementFontOptRead()
{
    ++FontOptRead;
}

void Server::DecrementFontOptRead()
{
    SF_ASSERT(FontOptRead > 0);
    --FontOptRead;
}

// Set the renderer to be profiled (only one supported)
void Server::SetRenderer(Render::Renderer2D* renderer)
{
    CurrentRenderer = renderer;
}


// AMP server generates unique handles for each SWF
// The 16-byte SWD debug ID is not used because it is not appropriate as a hash key
// AMP keeps a map of SWD handles to actual SWD ID and filename
UInt32 Server::GetNextSwdHandle() const
{
    static UInt32 lastSwdHandle = NativeCodeSwdHandle;
    Lock::Locker locker(&SwfLock);
    ++lastSwdHandle;
    return lastSwdHandle;
}

// Called whenever a Debug ID tag is found when reading a SWF
// Creates a new entry in the SWD map
void Server::AddSwf(UInt32 swdHandle, const char* swdId, const char* filename)
{
    SwdInfo* pInfo = SF_HEAP_AUTO_NEW(this) SwdInfo();
    pInfo->SwdId = swdId;
    pInfo->Filename = filename;

    Lock::Locker locker(&SwfLock);
    HandleToSwdIdMap.Set(swdHandle, *pInfo);
}

// Retrieves the SWD debug ID for a given handle
String Server::GetSwdId(UInt32 handle) const
{
    Lock::Locker locker(&SwfLock);
    SwdMap::ConstIterator it = HandleToSwdIdMap.Find(handle);
    if (it == HandleToSwdIdMap.End())
    {
        return "";
    }
    return it->Second->SwdId;
}

// Retrieves the SWF filename for a given handle
String Server::GetSwdFilename(UInt32 handle) const
{
    Lock::Locker locker(&SwfLock);
    SwdMap::ConstIterator it = HandleToSwdIdMap.Find(handle);
    if (it == HandleToSwdIdMap.End())
    {
        return "";
    }
    return it->Second->Filename;
}

// Retrieves the source filename for a given handle
String Server::GetSourceFilename(UInt64 handle) const
{
    Lock::Locker locker(&SourceFileLock);
    SourceFileMap::ConstIterator it = HandleToSourceFileMap.Find(handle);
    if (it == HandleToSourceFileMap.End())
    {
        return "";
    }
    return it->Second->Filename;
}

void Server::AddSourceFile(UInt64 fileHandle, const char* fileName)
{
    Lock::Locker locker(&SourceFileLock);
    Ptr<SourceFileInfo> fileInfo = *SF_HEAP_AUTO_NEW(this) SourceFileInfo();
    fileInfo->Filename = fileName;
    HandleToSourceFileMap.Set(fileHandle, fileInfo);
}

AmpStats* Server::GetDisplayStats()
{
    return RenderStats->DisplayTimings;
}

SF_GPA_ITT_ID Server::GetGpaGroupId()
{
    return GpaGroupId;
}

SF_GPA_ITT_DOMAIN* Server::GetGpaDomain()
{
    return GpaDomain;
}


// Constructor
// Called only from Server::Init
Server::Server() :
    ToggleState(0),
    ForceState(0),
    PendingForceState(false),
    PendingProfileLevel(Amp_Profile_Level_Null),
    Port(7534),
    BroadcastPort(GFX_AMP_BROADCAST_PORT),
    SocketThreadMgr(NULL),
    CurrentRenderer(NULL),
    ConnectionWaitDelay(0),
    InitSocketLib(true),
    Profiling(0),
    SoundMemory(0),
    NumStrokes(0),
    FontFailures(0),
    FontOptRead(0),
    MemReportLocked(0),
    ProfileLevelLocked(0),
    ObjectsReportRequested(0),
    ObjectsReportFlags(0),
    AppControlCallback(NULL),
    RecordingState(Amp_Server_Recording_Off)

{
    ReportHeap = Memory::GetHeapByAddress(this)->CreateHeap("Memory Report", MemoryHeap::Heap_UserDebug);
    SendCallback = *SF_HEAP_AUTO_NEW(this) SendThreadCallback();
    StatusCallback = *SF_HEAP_AUTO_NEW(this) StatusChangedCallback(&ConnectedEvent);
    AppControlCaps = *SF_HEAP_AUTO_NEW(this) MessageAppControl();
    RenderStats = *SF_HEAP_AUTO_NEW(this) RenderProfile();

    SendingEvent.SetEvent();

    Ptr<MessageTypeRegistry> msgRegistry = *SF_HEAP_AUTO_NEW(this) MessageTypeRegistry();
    msgRegistry->AddMessageType<MessageAppControl>(*SF_HEAP_AUTO_NEW(this) AppControlMsgHandler(this), true);
    msgRegistry->AddMessageType<MessageInitState>(*SF_HEAP_AUTO_NEW(this) InitStateMsgHandler(this), true);
    msgRegistry->AddMessageType<MessageSwdRequest>(*SF_HEAP_AUTO_NEW(this) SwdRequestMsgHandler(this));
    msgRegistry->AddMessageType<MessageSourceRequest>(*SF_HEAP_AUTO_NEW(this) SourceRequestMsgHandler(this));
    msgRegistry->AddMessageType<MessageObjectsReportRequest>(*SF_HEAP_AUTO_NEW(this) ObjectsReportRequestMsgHandler(this));
    msgRegistry->AddMessageType<MessageImageRequest>(*SF_HEAP_AUTO_NEW(this) ImageRequestMsgHandler(this));
    msgRegistry->AddMessageType<MessageFontRequest>(*SF_HEAP_AUTO_NEW(this) FontRequestMsgHandler(this));

    SocketThreadMgr = *SF_HEAP_AUTO_NEW(this) ThreadMgr(true, SendCallback, 
                        StatusCallback, &SendingEvent, NULL, Socket::GetAmpDefaultFactory(), msgRegistry);

    GpaDomain = SF_GPA_ITT_DOMAIN_CREATEA("Scaleform.GFX4");
    GpaStringHandle = SF_GPA_ITT_STRING_HANDLE_CREATEA("GFX4");
    GpaGroupId = SF_GPA_ID_MAKE((void*)GpaStringHandle, 0);
    SF_GPA_ID_CREATE(GpaDomain, GpaGroupId);
    SF_GPA_TASK_GROUP(GpaDomain, GpaGroupId, SF_GPA_ITT_NULL, GpaStringHandle);
}

// Destructor
Server::~Server()
{
    SocketThreadMgr = NULL;
    ReportHeap->Release();
}

// Start the socket threads
// Returns true if successful
bool Server::OpenConnection()
{
    if (!IsState(Amp_Disabled))
    {
        if (!SocketThreadMgr->InitAmp(NULL, Port, BroadcastPort, NULL))
        {
            return false;
        }

        UpdateProfilingState();

        WaitForAmpConnection(ConnectionWaitDelay);
    }

    return true;
}

// Stops the socket threads
void Server::CloseConnection()
{
    MovieLock.DoLock();
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieStats[i]->AdvanceTimings->DebugGo();
    }
    MovieLock.Unlock();

    SocketThreadMgr->UninitAmp();
    UpdateProfilingState();
}

// Returns true if we are listening for a connection
bool Server::IsSocketCreated() const
{
    return SocketThreadMgr->IsRunning();
}

// Collects all the memory data per frame
// Populates the ProfileFrame object, which is then sent to the client
void Server::CollectMemoryData(ProfileFrame* frameProfile)
{
    UInt32 MemoryComponents = 0;
    UInt32 systemImageMemory = 0;
    UInt32 graphicsImageMemory = 0;
    FontResourceMap fontMap;

    {
        Lock::Locker lock(&ImageLock);

        CollectFontData(&fontMap);

        // Images
        for (UPInt i = 0; i < Images.GetSize(); ++i)
        {
            ImageResource* imgRes = Images[i];

            Render::ImageBase* img = imgRes->GetImage();
            if (img != NULL)
            {
                int memRegion;
                Ptr<ImageInfo> imgInfo = *SF_HEAP_AUTO_NEW(this) ImageInfo();
                imgInfo->Id = img->GetImageId();
                imgInfo->Bytes = static_cast<UInt32>(img->GetBytes(&memRegion));
                switch (memRegion)
                {
                case 0:
                    frameProfile->ImageMemory += imgInfo->Bytes;
                    break;
                case 1:
                    frameProfile->ImageGraphicsMemory += imgInfo->Bytes;
                    break;
                default:
                    imgInfo->External = true;
                    break;
                }

                String imgName;
                String fileURL(imgRes->GetFileURL());
                if (!fileURL.IsEmpty())
                { 
                    imgName += fileURL + " ";
                }
                if (imgRes->GetResourceUse() == Resource::Use_Gradient)
                {
                    imgName += "Gradient";
                }
                else if (imgRes->GetResourceUse() == Resource::Use_Bitmap)
                {
                    imgName += "Bitmap";
                }

                UInt32 imageWidth = img->GetSize().Width;
                UInt32 imageHeight = img->GetSize().Height;

                if (img->GetImageType() == ImageBase::Type_SubImage)
                {
                    imgInfo->AtlasTop = img->GetRect().y1;
                    imgInfo->AtlasBottom = img->GetRect().y2;
                    imgInfo->AtlasLeft = img->GetRect().x1;
                    imgInfo->AtlasRight = img->GetRect().x2;
                    imgInfo->AtlasId = img->GetBaseImageId();
                    imageWidth = imgInfo->AtlasRight - imgInfo->AtlasLeft;
                    imageHeight = imgInfo->AtlasBottom - imgInfo->AtlasTop;
                }

                String imgFormat;
                switch (img->GetFormat())
                {
                case Render::Image_R8G8B8A8:
                    imgFormat = "R8G8B8A8";
                    break;
                case Render::Image_B8G8R8A8:
                    imgFormat = "B8G8R8A8";
                    break;
                case Render::Image_R8G8B8:
                    imgFormat = "R8G8B8";
                    break;
                case Render::Image_B8G8R8:
                    imgFormat = "B8G8R8";
                    break;
                case Render::Image_A8:
                    imgFormat = "A8";
                    break;
                case Render::Image_DXT1:
                    imgFormat = "DXT1";
                    break;
                case Render::Image_DXT3:
                    imgFormat = "DXT3";
                    break;
                case Render::Image_DXT5:
                    imgFormat = "DXT5";
                    break;
                case Render::Image_BC7:
                    imgFormat = "BC7";
                    break;
                default:
                    break;
                }

                Format(imgInfo->Name, "{1}x{2} {0} ({3})", 
                    imgName.ToCStr(), 
                    imageWidth, 
                    imageHeight,
                    imgFormat);

                frameProfile->ImageList.PushBack(imgInfo);
            }
        }
    }

    if (IsMemReports())
    {
        Memory::pGlobalHeap->MemReport(frameProfile->MemoryByStatId, MemoryHeap::MemReportHeapDetailed);
        frameProfile->TotalMemory = frameProfile->MemoryByStatId->GetValue("Scaleform Footprint") - frameProfile->MemoryByStatId->GetValue("Debug Data");
        frameProfile->VideoMemory = frameProfile->MemoryByStatId->SumValues("Video Heaps");
        frameProfile->MovieDataMemory = frameProfile->MemoryByStatId->GetValue("Movie Data Heaps");
        frameProfile->MovieDataMemory = frameProfile->MemoryByStatId->GetValue("Movie Data Heaps");
        frameProfile->FontMemory = frameProfile->MemoryByStatId->SumValues("Font");
        frameProfile->MovieViewMemory = frameProfile->MemoryByStatId->GetValue("Movie View Heaps");
        frameProfile->AS3Memory = frameProfile->MemoryByStatId->GetValue("AS3 VM");
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
        frameProfile->SoundMemory = frameProfile->MemoryByStatId->SumValues("Sound");
#else
        frameProfile->SoundMemory = static_cast<UInt32>(SoundMemory);
        if (frameProfile->SoundMemory != 0)
        {
            UInt32 lastMemId = frameProfile->MemoryByStatId->GetMaxId();
            MemItem* globalHeapItem = frameProfile->MemoryByStatId->SearchForName("Global Heap");
            if (globalHeapItem != NULL)
            {
                globalHeapItem->AddChild(++lastMemId, "Font Cache", frameProfile->FontCacheMemory);
                globalHeapItem->AddChild(++lastMemId, "Mesh Cache", frameProfile->MeshCacheMemory);
                globalHeapItem->AddChild(++lastMemId, "Sound", frameProfile->SoundMemory);
            }
        }
#endif
    }
    else
    {
        MemoryHeap::RootStats stats;
        Memory::pGlobalHeap->GetRootStats(&stats);
        frameProfile->TotalMemory = (UInt32)(stats.SysMemFootprint - stats.UserDebugFootprint - stats.DebugInfoFootprint);

        Ptr<MemItem> memSummary = *SF_HEAP_AUTO_NEW(this) MemItem(0);
        Memory::pGlobalHeap->MemReport(memSummary, MemoryHeap::MemReportBrief);
        UInt32 lastMemId = memSummary->GetMaxId();
        memSummary->ID = ++lastMemId;
        frameProfile->MemoryByStatId->Children.PushBack(memSummary);
        memSummary->AddChild(++lastMemId, "Font Cache", frameProfile->FontCacheMemory);
        memSummary->AddChild(++lastMemId, "Mesh Cache", frameProfile->MeshCacheMemory);

        for (UPInt i = 0; i < memSummary->Children.GetSize(); ++i)
        {
            MemItem* childItem = memSummary->Children[i];
            if (childItem->Name == "Video")
            {
                frameProfile->VideoMemory = childItem->Value;
            }
            else if (childItem->Name == "Sound")
            {
                frameProfile->SoundMemory = childItem->Value;
            }
            else if (childItem->Name == "Movie Data")
            {
                frameProfile->MovieDataMemory = childItem->Value;
            }
            else if (childItem->Name == "Movie View")
            {
                frameProfile->MovieViewMemory = childItem->Value;
            }
        }
#ifndef SF_MEMORY_ENABLE_DEBUG_INFO
        frameProfile->SoundMemory = static_cast<UInt32>(SoundMemory);
#endif
        memSummary->AddChild(++lastMemId, "Sound", frameProfile->SoundMemory);

        // Don't send too many images
        if (frameProfile->ImageList.GetSize() > 10)
        {
            frameProfile->Images->Name = "Total Image Memory";
            frameProfile->Images->SetValue(systemImageMemory + graphicsImageMemory);
            String imagesString;
            Format(imagesString, "{0} Images", frameProfile->ImageList.GetSize());
            frameProfile->Images->AddChild(++lastMemId, imagesString);
            frameProfile->ImageList.Clear();
        }   
    }

    MemoryComponents += frameProfile->ImageMemory;
    MemoryComponents += frameProfile->VideoMemory;
    MemoryComponents += frameProfile->SoundMemory;
    MemoryComponents += frameProfile->MovieDataMemory;
    MemoryComponents += frameProfile->MovieViewMemory;
    MemoryComponents += frameProfile->MeshCacheMemory;
    MemoryComponents += frameProfile->FontCacheMemory;

    const MemItem* unused = frameProfile->MemoryByStatId->SearchForName("Unused Memory");
    if (unused != NULL)
    {
        MemoryComponents += unused->Value;
    }

    frameProfile->OtherMemory = Alg::Max(frameProfile->TotalMemory, MemoryComponents) - MemoryComponents;

    // Fonts
    UInt32 lastItemId = frameProfile->MemoryByStatId->GetMaxId();
    frameProfile->Fonts->AddChild(++lastItemId, "Font Cache", frameProfile->FontCacheMemory);

    FontResourceMap::Iterator itFont;
    for (itFont = fontMap.Begin(); itFont != fontMap.End(); ++itFont)
    {
        Ptr<MemItem> fontMovie = *SF_HEAP_AUTO_NEW(this) MemItem(++lastItemId);
        fontMovie->Name = itFont->First;
        fontMovie->StartExpanded = true;
        for (UPInt j = 0; j < itFont->Second.GetSize(); ++j)
        {
            fontMovie->AddChild(++lastItemId, itFont->Second[j]);
        }
        frameProfile->Fonts->Children.PushBack(fontMovie);
    }

    frameProfile->Fonts->StartExpanded = true;
}

// Collects the font information from the MovieDefs
void Server::CollectFontData(FontResourceMap* fontMap) const
{
    {
        Lock::Locker lock(&MovieLock);

        // Get all the unique movieDefs from the registered Movies
        Array<MovieDef*> movieDefs;
        for (UPInt i = 0; i < Movies.GetSize(); ++i)
        {
            MovieDef* movieDef = Movies[i]->GetMovieDef();
            if (movieDef != NULL)
            {
                bool movieDefFound = false;
                for (UPInt j  = 0; j < movieDefs.GetSize(); ++j)
                {
                    if (movieDefs[j] == movieDef)
                    {
                        movieDefFound = true;
                        break;
                    }
                }
                if (!movieDefFound)
                {
                    movieDefs.PushBack(movieDef);
                }
            }
        }

        // Get the images for each movieDef
        for (UPInt i = 0; i < movieDefs.GetSize(); ++i)
        {
            FontVisitor fontVisitor;
            movieDefs[i]->VisitResources(&fontVisitor, MovieDef::ResVisit_Fonts);

            String movieName(movieDefs[i]->GetFileURL());
            FontResourceMap::Iterator itFont = fontMap->FindCaseInsensitive(movieName);
            if (itFont == fontMap->End())
            {
                fontMap->SetCaseInsensitive(movieName, fontVisitor.Fonts);
            }
            else
            {
                itFont->Second.Append(fontVisitor.Fonts);
            }
        }
    }
}


// Collects all the movie data per frame
// Populates the ProfileFrame object, which is then sent to the client
// Clears all the movie data when done
// Clearing needs to happen even when stats are not being reported
// to avoid them accumulating and therefore being wrong for the first frame
void Server::CollectMovieData(ProfileFrame* frameProfile)
{
    Array< Ptr<ViewProfile> > movieCopy;
    {
        // Make a copy before collecting stats to avoid deadlocks (in case ViewStats is locked)
        Lock::Locker lock(&MovieLock);
        for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
        {
            movieCopy.PushBack(MovieStats[i]);
        }

        // Re-syncs the movie stats with the loaded movies
        // after the statistics have been reported
        MovieStats.Clear();
        for (UPInt i = 0; i < Movies.GetSize(); ++i)
        {
            MovieStats.PushBack(*SF_HEAP_AUTO_NEW(this) ViewProfile(Movies[i]));
        }
    }

    if (frameProfile != NULL)
    {
        frameProfile->MovieStats.Resize(movieCopy.GetSize());
    }

    for (UPInt i = 0; i < movieCopy.GetSize(); ++i)
    {
        if (frameProfile != NULL)
        {
            movieCopy[i]->CollectStats(frameProfile, i);
        }
        movieCopy[i]->ClearStats();
    }
}

// Populates the ProfileFrame with renderer stats
void Server::CollectRendererData(ProfileFrame* frameProfile)
{
    RenderStats->CollectStats(frameProfile);

    Render::HAL::Stats renderStats;
    CurrentRenderer->GetHAL()->GetStats(&renderStats, false);
    CollectRendererStats(frameProfile, renderStats);
    CollectMeshCacheStats(frameProfile);
    CollectRendererExternalMemory(frameProfile);

    Render::GlyphCache* glyphCache = CurrentRenderer->GetImpl()->GetGlyphCache();
    frameProfile->RasterizedGlyphCount = glyphCache->GetNumRasterizedGlyphs();
    frameProfile->FontTextureCount = glyphCache->GetNumTextures();
    frameProfile->FontTotalArea = static_cast<UInt32>(glyphCache->ComputeTotalArea());
    frameProfile->FontUsedArea = static_cast<UInt32>(glyphCache->ComputeUsedArea());
    if (frameProfile->FontTotalArea != 0)
    {
        frameProfile->FontFill = 100 * frameProfile->FontUsedArea / frameProfile->FontTotalArea;
    }
    frameProfile->FontCacheMemory = static_cast<UInt32>(glyphCache->GetBytes());
    frameProfile->FontFail = FontFailures;
    frameProfile->FontOptRead = FontOptRead;

    frameProfile->MeshThrashing = CurrentRenderer->GetHAL()->GetMeshCache().Thrashing;

    frameProfile->GradientFillCount = static_cast<UInt32>(CurrentRenderer->GetImpl()->GetPrimitiveFillManager().GetNumGradients());
    frameProfile->StrokeCount = NumStrokes;

    ClearRendererData();
}

void Server::CollectRendererStats(ProfileFrame* frameProfile, const Render::HAL::Stats& stats)
{
    frameProfile->MeshCount += stats.Meshes;
    frameProfile->TriangleCount += stats.Triangles;
    frameProfile->DrawPrimitiveCount += stats.Primitives;
    frameProfile->MaskCount += stats.Masks;
    frameProfile->FilterCount += stats.Filters;
}

void Server::CollectMeshCacheStats(ProfileFrame* frameProfile)
{
    Render::MeshCache::Stats meshStats;
    CurrentRenderer->GetHAL()->GetMeshCache().GetStats(&meshStats);

    for (int i = 0; i < Render::MeshCache::MeshBuffer_StatCount; ++i)
    {
        UInt32 usedMemory = static_cast<UInt32>(meshStats.TotalSize[i] - meshStats.UsedSize[i]);
        UInt32 unusedMemory = static_cast<UInt32>(meshStats.UsedSize[i]);
        if (i & Render::MeshCache::MeshBuffer_GpuMem)
        {
            frameProfile->MeshCacheGraphicsMemory += usedMemory;
            frameProfile->MeshCacheGraphicsUnusedMemory += unusedMemory;
        }
        else
        {
            frameProfile->MeshCacheMemory += usedMemory;
            frameProfile->MeshCacheUnusedMemory += unusedMemory;
        }
    }
}

void Server::CollectRendererExternalMemory(ProfileFrame* frameProfile)
{
    CurrentRenderer->GetImpl()->GetExternalMemory(frameProfile->MemoryByStatId);
}


// Clears all the renderer statistics
// Clearing needs to happen even when stats are not being reported
// to avoid them accumulating and therefore being wrong for the first frame
void Server::ClearRendererData()
{
    RenderStats->ClearStats();
    Render::HAL::Stats stats;
    CurrentRenderer->GetHAL()->GetStats(&stats, true);
    CurrentRenderer->GetHAL()->GetMeshCache().Thrashing = 0;
    FontFailures = 0;
}

void Server::CollectTaskData(ProfileFrame* frameProfile)
{
    Array< Ptr<RenderProfile> > tasksCopy;
    {
        // Make a copy before collecting stats to avoid deadlocks (in case ViewStats is locked)
        Lock::Locker lock(&LoadProcessLock);
        for (UPInt i = 0; i < TaskStats.GetSize(); ++i)
        {
            tasksCopy.PushBack(TaskStats[i]);
        }

        // Remove TaskStats of LoadProcesses that have finished
        TaskStats.Clear();
        for (UPInt i = 0; i < LoadProcesses.GetSize(); ++i)
        {
            RenderProfile* renderProf = SF_HEAP_AUTO_NEW(this) RenderProfile();
            renderProf->DisplayTimings = LoadProcesses[i]->LoadProcessStats;
            TaskStats.PushBack(*renderProf);
        }
    }

    for (UPInt i = 0; i < tasksCopy.GetSize(); ++i)
    {
        if (frameProfile != NULL)
        {
            tasksCopy[i]->CollectStats(frameProfile);
        }
        tasksCopy[i]->ClearStats();
    }
}

// Sends all the statistics for the current frame to the AMP client
void Server::SendFrameStats()
{
    bool printStats = false;
    {
        Lock::Locker recordingLock(&RecordingStateLock);
        switch (RecordingState)
        {
            case Amp_Server_Recording_Off:
            default:
                break;
            case Amp_Server_Recording_On:
                // Don't send anything until recording is toggled off
                return;
            case Amp_Server_Recording_Stopped:
                printStats = true;
                RecordingState = Amp_Server_Recording_Off;
                break;                    
        }
    }

    ProfileFrame* frameProfile = SF_HEAP_NEW(ReportHeap) ProfileFrame();

    static UInt64 lastTick = 0;
    static UInt32 frameCounter = 0;
    static UInt32 lastFps = 0;
    static UInt64 firstTick = Timer::GetProfileTicks();
    frameProfile->TimeStamp = Timer::GetProfileTicks() - firstTick;
    UInt64 iDeltaTicks = frameProfile->TimeStamp - lastTick;
    ++frameCounter;
    if (iDeltaTicks > 1000000)
    {
        lastFps = static_cast<UInt32>(frameCounter * 1000000 / iDeltaTicks);
        frameCounter = 0;
        lastTick = frameProfile->TimeStamp;
    }
    frameProfile->FramesPerSecond = lastFps;
    frameProfile->ProfilingLevel = GetProfileLevel();
    frameProfile->DetailedMemReport = IsState(Amp_MemoryReports);

    // Send the active SWD handles so the client knows to load the appropriate SWD
    SwfLock.DoLock();
    for (SwdMap::Iterator it = HandleToSwdIdMap.Begin(); it != HandleToSwdIdMap.End(); ++it)
    {
        frameProfile->SwdHandles.PushBack(it->First);
    }
    SwfLock.Unlock();

    // Send the active files so the client knows to load them
    SourceFileLock.DoLock();
    for (SourceFileMap::Iterator it = HandleToSourceFileMap.Begin(); it != HandleToSourceFileMap.End(); ++it)
    {
        frameProfile->FileHandles.PushBack(it->First);
    }
    SourceFileLock.Unlock();
    
//     Uncomment for marker testing
//     if (frameProfile->TimeStamp % 10 == 0 && MovieStats.GetSize() > 0)
//     {
//         MovieStats[0]->AdvanceTimings->AddMarker("test");
//     }

    CollectMovieData(frameProfile);
    CollectRendererData(frameProfile);
    CollectTaskData(frameProfile);
    CollectMemoryData(frameProfile);
    
    if (printStats)
    {
        Ptr<Log> loaderLog;
        Lock::Locker lock(&LoaderLock);
        for (UPInt i = 0; i < Loaders.GetSize(); ++i)
        {
            loaderLog = Loaders[i]->GetLog();
            if (loaderLog)
            {
                break;
            }
        }
        if (loaderLog)
        {
            frameProfile->Print(loaderLog);
        }
        frameProfile->Release();
    }
    else
    {
        SendMessage(SF_HEAP_AUTO_NEW(this) MessageProfileFrame(*frameProfile));
    }
}

Ptr<ViewStats> Server::GetDebugPausedMovie() const
{
    Lock::Locker lock(&MovieLock);
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        if (MovieStats[i]->AdvanceTimings->IsDebugPaused())
        {
            return MovieStats[i]->AdvanceTimings;
        }
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////////

Server::ViewProfile::ViewProfile(MovieImpl* movie) : 
    AdvanceTimings(&movie->GetAdvanceStats())
{
}

void Server::ViewProfile::CollectStats(ProfileFrame* frameProfile, UPInt index)
{
    MovieProfile* stats = SF_HEAP_AUTO_NEW(this) MovieProfile(); 

    AdvanceTimings->CollectTimingStats(frameProfile);
    AdvanceTimings->CollectAmpInstructionStats(stats);
    AdvanceTimings->CollectAmpFunctionStats(stats);
    AdvanceTimings->CollectAmpSourceLineStats(stats);
    AdvanceTimings->CollectMarkers(stats);
    AdvanceTimings->CollectGcStats(frameProfile);

    stats->ViewHandle = AdvanceTimings->GetViewHandle();
    stats->MinFrame = stats->MaxFrame = AdvanceTimings->GetCurrentFrame();
    stats->ViewName = AdvanceTimings->GetName();
    stats->Version = AdvanceTimings->GetVersion();
    stats->Width = AdvanceTimings->GetWidth();
    stats->Height = AdvanceTimings->GetHeight();
    stats->FrameRate = AdvanceTimings->GetFrameRate();
    stats->FrameCount = AdvanceTimings->GetFrameCount();

    frameProfile->MovieStats[index] = *stats;        
}

void Server::ViewProfile::ClearStats()
{
    AdvanceTimings->ClearAmpFunctionStats();
    AdvanceTimings->ClearAmpInstructionStats();
    AdvanceTimings->ClearAmpSourceLineStats();
    AdvanceTimings->ClearMarkers();
    AdvanceTimings->ClearGcStats();
}

//////////////////////////////////////////////////////////////////////////////

Server::RenderProfile::RenderProfile()
{
    DisplayTimings = *SF_HEAP_AUTO_NEW(this) ViewStats();
}

void Server::RenderProfile::CollectStats(ProfileFrame* frameProfile)
{
    Ptr<MovieProfile> stats = *SF_HEAP_AUTO_NEW(this) MovieProfile(); 

    DisplayTimings->CollectTimingStats(frameProfile);
    DisplayTimings->CollectAmpFunctionStats(stats);

    frameProfile->DisplayFunctionStats->Merge(*stats->FunctionTreeStats);
    frameProfile->DisplayStats->Merge(*stats->FunctionStats);        
}

void Server::RenderProfile::ClearStats()
{
    DisplayTimings->ClearAmpFunctionStats();
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // SF_AMP_SERVER


