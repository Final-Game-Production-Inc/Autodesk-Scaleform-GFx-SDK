/**************************************************************************

PublicHeader:   AMP
Filename    :   Amp_Server.h
Content     :   Class encapsulating communication with AMP
                Embedded in application to be profiled
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INCLUDE_FX_AMP_SERVER_H
#define INCLUDE_FX_AMP_SERVER_H

#include "GFxConfig.h"
#if defined(SF_AMP_SERVER) || defined (SF_AMP_CLIENT)

#include "Amp_Interfaces.h"
#include "Kernel/SF_AmpInterface.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_ImageResource.h"
#include "Render/Renderer2D.h"
#include "Render/Render_HAL.h"
#include "GFx/AMP/Amp_Message.h"

namespace Scaleform {

namespace GFx {

class SocketImplFactory;

namespace AMP {

class ThreadMgr;
class SendThreadCallback;
class StatusChangedCallback;
class ViewStats;
class AmpStream;
class ObjectsLog;

// AMP server states
enum ServerStateType
{
    Amp_Default =                   0x00000000,
    Amp_Paused =                    0x00000001,
    Amp_Disabled =                  0x00000002,
    Amp_RenderOverdraw =            0x00000008,
    Amp_InstructionSampling =       0x00000010,
    Amp_MemoryReports =             0x00000020,
    Amp_NoFunctionAggregation =     0x00000040,
    Amp_App_Paused =                0x00001000,
    Amp_App_Wireframe =             0x00002000,
    Amp_App_FastForward =           0x00004000,
    Amp_RenderBatch =               0x00008000,
    Amp_RenderBlending =            0x00010000,
    Amp_RenderTextureDensity =      0x00020000,
};

#define GFX_AMP_BROADCAST_PORT 7533

// Server encapsulates the communication of a GFx application with an AMP client
// It is a singleton so that it can be easily accessed throughout GFx
// The object has several functions:
// - it keeps track of statistics per frame
// - it manages the socket connection with the client
// - it handles received messages
// The server needs to be thread-safe, as it can be accessed from anywhere in GFx
class Server : public RefCountBase<Server, StatAmp_Server>, public AmpServer
{
public:
    // Singleton initialization and uninitialization
    // Called from System
    static void SF_STDCALL  Init();
    static void SF_STDCALL  Uninit();

    // Connection status
    virtual bool    OpenConnection();
    virtual void    CloseConnection();
    virtual bool    IsValidConnection() const;
    virtual UInt32  GetCurrentState() const;
    virtual void    SetConnectedApp(const char* playerTitle);
    virtual void    SetAaMode(const char* aaMode);
    virtual void    SetStrokeType(const char* strokeType);
    virtual void    SetCurrentLocale(const char* locale);
    virtual void    SetCurveTolerance(float tolerance);
    virtual void    UpdateState(const ServerState* state);
    virtual bool    IsEnabled() const               { return !IsState(Amp_Disabled); }
    virtual bool    IsPaused() const                { return IsState(Amp_Paused); }
    virtual bool    IsProfiling() const;
    virtual bool    IsInstructionProfiling() const  { return (GetProfileLevel() >= Amp_Profile_Level_High); }
    virtual bool    IsInstructionSampling() const   { return IsState(Amp_InstructionSampling); }
    virtual bool    IsFunctionAggregation() const   { return !IsState(Amp_NoFunctionAggregation); }
    virtual AmpProfileLevel GetProfileLevel() const;
    virtual void    SetProfileLevel(AmpProfileLevel profileLevel, bool lock);
    virtual void    SetMemReports(bool memReports, bool lock);
    virtual void    SetAppControlCaps(const MessageAppControl* caps);
    virtual void    ToggleInternalStatRecording();

    // Configuration options
    virtual void    SetListeningPort(UInt32 port);
    virtual void    SetBroadcastPort(UInt32 port);
    virtual void    SetConnectionWaitTime(unsigned waitTimeMilliseconds);
    virtual void    SetHeapLimit(UPInt memLimit);
    virtual void    SetInitSocketLib(bool initSocketLib);
    virtual void    SetSocketImplFactory(SocketImplFactory* socketFactory);

    // AdvanceFrame needs to be called once per frame
    // It is called from GRenderer::EndFrame
    virtual void    AdvanceFrame();

    // Called from movie advance thread
    virtual void    MovieAdvance(MovieImpl* movie);

    // Custom callback that handles application-specific messages
    virtual void    SetAppControlCallback(AppControlInterface* callback);

    // Message handler
    virtual bool    HandleNextMessage();

    // Specific messages
    virtual void    SendLog(const char* message, int messageLength, LogMessageId msgType);
    virtual void    SendCurrentState();
    virtual void    SendAppControlCaps();

    // Message handling methods
    bool            HandleAppControl(const MessageAppControl* message);
    bool            HandleInitState(const MessageInitState* message);
    bool            HandleSwdRequest(const MessageSwdRequest* message);
    bool            HandleSourceRequest(const MessageSourceRequest* message);
    bool            HandleObjectsReportRequest(const MessageObjectsReportRequest* message);
    bool            HandleImageRequest(const MessageImageRequest* message);
    bool            HandleFontRequest(const MessageFontRequest* message);

    // AMP keeps track of active Movie Views
    virtual void    AddMovie(MovieImpl* movie);
    virtual void    RemoveMovie(MovieImpl* movie);
    // note: the returned movie is AddRef-ed!
    virtual bool    FindMovieByHeap(MemoryHeap* heap, MovieImpl** movie);

    // AMP keeps track of images
    virtual void    AddImage(GFx::ImageResource* image);
    virtual void    RemoveImage(GFx::ImageResource* image);

    // AMP keeps track of active Loaders (for renderer access)
    virtual void    AddLoader(Loader* loader);
    virtual void    RemoveLoader(Loader* loader);

    // Keeps track of task stats to report loading times, etc
    virtual void    AddLoadProcess(GFx::LoadProcess* loadProcess);
    virtual void    RemoveLoadProcess(GFx::LoadProcess* loadProcess);

    // AMP keeps track of sound memory (when SF_MEMORY_ENABLE_DEBUG_INFO is not defined)
    virtual void    AddSound(UPInt soundMem);
    virtual void    RemoveSound(UPInt soundMem);

    // AMP keeps track of some renderer stats
    virtual void    AddStrokes(UInt32 numStrokes);
    virtual void    RemoveStrokes(UInt32 numStrokes);
    virtual void    IncrementFontFailures();
    virtual void    IncrementFontOptRead();
    virtual void    DecrementFontOptRead();

    // Set the renderer to be profiled (only one supported)
    virtual void    SetRenderer(Render::Renderer2D* renderer);

    // AMP server generates unique handles for each SWF
    // The 16-byte SWD debug ID is not used because it is not appropriate as a hash key
    // AMP keeps a map of SWD handles to actual SWD ID and filename
    virtual UInt32  GetNextSwdHandle() const;
    virtual void    AddSwf(UInt32 swdHandle, const char* swdId, const char* filename);
    virtual String  GetSwdId(UInt32 handle) const;
    virtual String  GetSwdFilename(UInt32 handle) const;

    // ActionScript source code file name for a given handle
    virtual void    AddSourceFile(UInt64 fileHandle, const char* fileName);

    // Renderer statistics
    virtual AmpStats*   GetDisplayStats();

    // GPA integration
    virtual SF_GPA_ITT_ID        GetGpaGroupId();
    virtual SF_GPA_ITT_DOMAIN*   GetGpaDomain();

private:

    // Struct that holds loaded SWF information
    struct SwdInfo : public RefCountBase<SwdInfo, StatAmp_Server>
    {
        StringLH    SwdId;
        StringLH    Filename;
    };
    typedef HashLH<UInt32, Ptr<SwdInfo> > SwdMap;

    // Struct that holds AS file information
    struct SourceFileInfo : public RefCountBase<SourceFileInfo, StatAmp_Server>
    {
        StringLH Filename;
    };
    typedef HashLH<UInt64, Ptr<SourceFileInfo> > SourceFileMap;

    struct ViewProfile : public RefCountBase<ViewProfile, StatAmp_Server>
    {
        ViewProfile(MovieImpl* movie);
        void CollectStats(ProfileFrame* frameProfile, UPInt index);
        void ClearStats();
        Ptr<ViewStats>      AdvanceTimings;
    };

    struct RenderProfile : public RefCountBase<RenderProfile, StatAmp_Server>
    {
        RenderProfile();
        void CollectStats(ProfileFrame* frameProfile);
        void ClearStats();
        Ptr<ViewStats>      DisplayTimings;
    };

    // Member variables
    ServerState                     CurrentState;   // Paused, Disabled, etc
    mutable Lock                    CurrentStateLock;
    UInt32                          ToggleState;   // Paused, Disabled, etc
    UInt32                          ForceState;   // Paused, Disabled, etc
    bool                            PendingForceState;
    SInt32                          PendingProfileLevel;
    mutable Lock                    ToggleStateLock;
    UInt32                          Port;           // For socket connection to client
    UInt32                          BroadcastPort;  // For broadcasting IP address to AMP
    Ptr<ThreadMgr>                  SocketThreadMgr;      // Socket threads
    ArrayLH<MovieImpl*>             Movies;
    ArrayLH< Ptr<ViewProfile> >     MovieStats;
    mutable Lock                    MovieLock;
    ArrayLH<ImageResource*>         Images;
    mutable Lock                    ImageLock;
    ArrayLH<LoadProcess*>           LoadProcesses;
    mutable Lock                    LoadProcessLock;
    ArrayLH< Ptr<RenderProfile> >   TaskStats;
    ArrayLH<Loader*>                Loaders;
    Render::Renderer2D*             CurrentRenderer;
    Ptr<RenderProfile>              RenderStats;
    mutable Lock                    LoaderLock;
    SwdMap                          HandleToSwdIdMap; // Map of unique handle to SWF information
    mutable Lock                    SwfLock;
    SourceFileMap                   HandleToSourceFileMap; 
    mutable Lock                    SourceFileLock;
    Scaleform::Event                ConnectedEvent; // Used to suspend GFx until connected to AMP
    Scaleform::Event                SendingEvent; // Used to suspend GFx until message queue is empty
    unsigned                        ConnectionWaitDelay;  // milliseconds
    bool                            InitSocketLib;  // Initialize socket library?
    mutable AtomicInt<UInt32>       Profiling;
    AtomicInt<UPInt>                SoundMemory;
    AtomicInt<UInt32>               NumStrokes;
    AtomicInt<UInt32>               FontFailures;
    AtomicInt<UInt32>               FontOptRead;
    AtomicInt<UInt32>               MemReportLocked;
    AtomicInt<UInt32>               ProfileLevelLocked;

    // For objects report
    Lock                            ObjectsReportLock;
    UInt32                          ObjectsReportRequested;
    UInt32                          ObjectsReportFlags;

    // Task group domain and ID for GPA integration
    SF_GPA_ITT_DOMAIN*              GpaDomain;
    SF_GPA_ITT_STRING_HANDLE*       GpaStringHandle;
    SF_GPA_ITT_ID                   GpaGroupId;

    // Handles app requests from AMP client
    AppControlInterface*            AppControlCallback;
    // Callback from Send Thread
    Ptr<SendThreadCallback>         SendCallback;
    // Callback for connection status change
    Ptr<StatusChangedCallback>      StatusCallback;

    // Caps so that Client knows what app control functionality is supported
    Ptr<MessageAppControl>          AppControlCaps;

    MemoryHeap*                     ReportHeap;
    
    // Recording statistics for server-side display
    // Can record statistics for display when not connected to AMP client
    enum ServerRecordingState
    {
        Amp_Server_Recording_Off,
        Amp_Server_Recording_On,
        Amp_Server_Recording_Stopped,
    };
    ServerRecordingState    RecordingState;
    mutable Lock            RecordingStateLock;

    // private constructor 
    // Create singleton with Server::Init
    Server();
    ~Server();

    bool        IsMemReports() const            { return IsState(Amp_MemoryReports); }
    void        WaitForAmpConnection(unsigned maxDelayMilliseconds = SF_WAIT_INFINITE);
    bool        IsState(ServerStateType state) const;
    void        SetState(ServerStateType state, bool stateValue, bool sendState);
    void        ToggleAmpState(UInt32 toggleState, bool sendState);
    void        SetAmpState(UInt32 newState, bool sendState);
    String      GetSourceFilename(UInt64 handle) const;
    bool        GetProfilingState() const;
    void        UpdateProfilingState();
    void        SendMessage(Message* msg);

    // Internal helper methods
    bool        IsSocketCreated() const;

    void        CollectMemoryData(ProfileFrame* frameProfile);
    void        CollectMovieData(ProfileFrame* frameProfile);
    void        CollectRendererData(ProfileFrame* frameProfile);
    void        ClearRendererData();
    void        CollectTaskData(ProfileFrame* frameProfile);
    void        SendFrameStats();
    void        CollectRendererStats(ProfileFrame* frameProfile, const Render::HAL::Stats& stats);
    void        CollectMeshCacheStats(ProfileFrame* frameProfile);
    void        CollectRendererExternalMemory(ProfileFrame* frameProfile);
    
    MessageImageData* GetImageData(UInt32 imageId) const;

    typedef StringHash< Array<String> > FontResourceMap;
    void        CollectFontData(FontResourceMap* fontMap) const;

    Ptr<ViewStats>  GetDebugPausedMovie() const;
};


} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // SF_AMP_SERVER

#endif  // INCLUDE_FX_AMP_SERVER_H

