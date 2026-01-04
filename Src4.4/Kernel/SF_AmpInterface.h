/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_AmpInterface.h
Content     :   AMP Interface
Created     :   2010
Authors     :   Alex Mantzaris

Notes       :   Interface to the Analyzer for Memory and Performance

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_AmpInterface_H
#define INC_SF_Kernel_AmpInterface_H

#include "SF_RefCount.h"
#include "SF_String.h"
#include "SF_Log.h"

#if defined(SF_PROFILE_GPA)
    #include <ittnotify.h>
    #define SF_GPA_ITT_DOMAIN                       __itt_domain
    #define SF_GPA_ITT_DOMAIN_CREATEA(Name)         __itt_domain_createA(Name)
    #define SF_GPA_ITT_STRING_HANDLE                __itt_string_handle
    #define SF_GPA_ITT_STRING_HANDLE_CREATEA(Name)  __itt_string_handle_createA(Name)
    #define SF_GPA_ITT_NULL                         __itt_null
    #define SF_GPA_ITT_ID                           __itt_id
    #define SF_GPA_ID_MAKE(Ptr, X)                  __itt_id_make(Ptr, X)
    #define SF_GPA_ID_CREATE(Domain, ID)            __itt_id_create(Domain, ID)
    #define SF_GPA_ID_DESTROY(Domain, ID)           __itt_id_destroy(Domain, ID)
    #define SF_GPA_TASK_GROUP(Domain, GroupID, ParentID, Name)  __itt_task_group(Domain, GroupID, ParentID, Name)
#else
    #define SF_GPA_ITT_DOMAIN void
    #define SF_GPA_ITT_DOMAIN_CREATEA(Name) NULL
    #define SF_GPA_ITT_STRING_HANDLE void
    #define SF_GPA_ITT_STRING_HANDLE_CREATEA(Name) NULL
    #define SF_GPA_ITT_NULL 0
    #define SF_GPA_ITT_ID unsigned int
    #define SF_GPA_ID_MAKE(Ptr, X) 0
    #define SF_GPA_ID_CREATE(Domain, ID)
    #define SF_GPA_ID_DESTROY(Domain, ID)
    #define SF_GPA_TASK_GROUP( Domain, GroupID, ParentID, Name )
#endif

namespace Scaleform {
    class File;

namespace Render { 
    class HAL; 
}

namespace GFx { 
    class Loader;
    class MovieImpl;
    class ImageResource;
    class LoadProcess;
    class MovieDef;
    class SocketImplFactory;
    namespace AMP {
        class AppControlInterface;
        class ServerState;
        class MessageAppControl;
    }
}

enum AmpNativeFunctionId
{
    Amp_Native_Function_Id_Invalid = -1,

    Amp_Native_Function_Id_Advance,
    Amp_Native_Function_Id_AdvanceFrame,
    Amp_Native_Function_Id_DoActions,
    Amp_Native_Function_Id_ProcessInput,
    Amp_Native_Function_Id_ProcessMouse,

    Amp_Native_Function_Id_GcCollect,
    Amp_Native_Function_Id_GcMarkInCycle,
    Amp_Native_Function_Id_GcScanInUse,
    Amp_Native_Function_Id_GcFreeGarbage,
    Amp_Native_Function_Id_GcFinalize,
    Amp_Native_Function_Id_GcDelayedCleanup,

    Amp_Native_Function_Id_Display,
    Amp_Native_Function_Id_NextCapture,
    Amp_Native_Function_Id_Draw,
    Amp_Native_Function_Id_Present,
    Amp_Native_Function_Id_Tessellate,
    Amp_Native_Function_Id_GlyphTextureMapper_Create,
    Amp_Native_Function_Id_GradientFill,
    Amp_Native_Function_Id_GlyphCache_RasterizeGlyph,
    Amp_Native_Function_Id_GlyphCache_RasterizeShadow,
    Amp_Native_Function_Id_GlyphCache_EvictText,

    Amp_Native_Function_Id_Begin_ObjectInterface,
    Amp_Native_Function_Id_GetVariable,
    Amp_Native_Function_Id_GetVariableArray,
    Amp_Native_Function_Id_GetVariableArraySize,
    Amp_Native_Function_Id_SetVariable,
    Amp_Native_Function_Id_SetVariableArray,
    Amp_Native_Function_Id_SetVariableArraySize,
    Amp_Native_Function_Id_Invoke,
    Amp_Native_Function_Id_InvokeAlias,
    Amp_Native_Function_Id_InvokeArgs,
    Amp_Native_Function_Id_InvokeAliasArgs,
    Amp_Native_Function_Id_ObjectInterface_HasMember,
    Amp_Native_Function_Id_ObjectInterface_GetMember,
    Amp_Native_Function_Id_ObjectInterface_SetMember,
    Amp_Native_Function_Id_ObjectInterface_Invoke,
    Amp_Native_Function_Id_ObjectInterface_DeleteMember,
    Amp_Native_Function_Id_ObjectInterface_VisitMembers,
    Amp_Native_Function_Id_ObjectInterface_GetArraySize,
    Amp_Native_Function_Id_ObjectInterface_SetArraySize,
    Amp_Native_Function_Id_ObjectInterface_GetElement,
    Amp_Native_Function_Id_ObjectInterface_SetElement,
    Amp_Native_Function_Id_ObjectInterface_VisitElements,
    Amp_Native_Function_Id_ObjectInterface_PushBack,
    Amp_Native_Function_Id_ObjectInterface_PopBack,
    Amp_Native_Function_Id_ObjectInterface_RemoveElements,
    Amp_Native_Function_Id_ObjectInterface_GetDisplayMatrix,
    Amp_Native_Function_Id_ObjectInterface_SetDisplayMatrix,
    Amp_Native_Function_Id_ObjectInterface_GetMatrix3D,
    Amp_Native_Function_Id_ObjectInterface_SetMatrix3D,
    Amp_Native_Function_Id_ObjectInterface_IsDisplayObjectActive,
    Amp_Native_Function_Id_ObjectInterface_GetParent,
    Amp_Native_Function_Id_ObjectInterface_GetDisplayInfo,
    Amp_Native_Function_Id_ObjectInterface_SetDisplayInfo,
    Amp_Native_Function_Id_ObjectInterface_SetText,
    Amp_Native_Function_Id_ObjectInterface_GetText,
    Amp_Native_Function_Id_ObjectInterface_GotoAndPlay,
    Amp_Native_Function_Id_ObjectInterface_GetCxform,
    Amp_Native_Function_Id_ObjectInterface_SetCxform,
    Amp_Native_Function_Id_ObjectInterface_CreateEmptyMovieClip,
    Amp_Native_Function_Id_ObjectInterface_AttachMovie,
    Amp_Native_Function_Id_ObjectInterface_ToString,
    Amp_Native_Function_Id_ObjectInterface_GetWorldMatrix,
    Amp_Native_Function_Id_ObjectInterface_InvokeClosure,
	Amp_Native_Function_Id_ObjectInterface_IsInstanceOf,
    Amp_Native_Function_Id_ObjectInterface_IsByteArray,
    Amp_Native_Function_Id_ObjectInterface_GetByteArraySize,
	Amp_Native_Function_Id_ObjectInterface_SetByteArraySize,
    Amp_Native_Function_Id_ObjectInterface_ReadFromByteArray,
    Amp_Native_Function_Id_ObjectInterface_WriteToByteArray,
	Amp_Native_Function_Id_ObjectInterface_GetRawDataPtr,
    Amp_Native_Function_Id_End_ObjectInterface,

    Amp_Num_Native_Function_Ids
};

// AMP function profile levels
enum AmpProfileLevel
{
    Amp_Profile_Level_Null = -1,
    Amp_Profile_Level_Low,
    Amp_Profile_Level_Medium,
    Amp_Profile_Level_ActionScript = Amp_Profile_Level_Medium,
    Amp_Profile_Level_High,
};

class AmpStats;

// AmpInterface is the interface for AMP
// Having an interface in the Kernel eliminates the dependency on AMP
// an instance of a class implementing this interface is typically 
// created on application startup and passed into System or GFx::System constructor.
class AmpServer
{
public:
    virtual ~AmpServer() { }

    // AdvanceFrame needs to be called once per frame
    // It is called from GRenderer::EndFrame
    virtual void AdvanceFrame() = 0;

    // Called from movie advance thread
    virtual void MovieAdvance(GFx::MovieImpl* movie) = 0;

    static AmpServer& SF_STDCALL    GetInstance();
    static void SF_STDCALL          Init();
    static void SF_STDCALL          Uninit();

    // Custom callback that handles application-specific messages
    virtual void        SetAppControlCallback(GFx::AMP::AppControlInterface* callback) = 0;

    virtual bool        OpenConnection() = 0;
    virtual void        CloseConnection() = 0;
    virtual bool        IsValidConnection() const = 0;
    virtual UInt32      GetCurrentState() const = 0;
    virtual void        SetConnectedApp(const char* playerTitle) = 0;
    virtual void        SetAaMode(const char* aaMode) = 0;
    virtual void        SetStrokeType(const char* strokeType) = 0;
    virtual void        SetCurrentLocale(const char* locale) = 0;
    virtual void        SetCurveTolerance(float tolerance) = 0;
    virtual void        UpdateState(const GFx::AMP::ServerState* state) = 0;
    virtual bool        IsEnabled() const = 0;
    virtual bool        IsPaused() const = 0;
    virtual bool        IsProfiling() const = 0;
    virtual bool        IsInstructionProfiling() const = 0;
    virtual bool        IsInstructionSampling() const = 0;
    virtual bool        IsFunctionAggregation() const = 0;
    virtual AmpProfileLevel GetProfileLevel() const = 0;
    virtual void        SetProfileLevel(AmpProfileLevel profileLevel, bool lock) = 0;     
    virtual void        SetMemReports(bool memReports, bool lock) = 0;
    virtual void        SetAppControlCaps(const GFx::AMP::MessageAppControl* caps) = 0;
    virtual void        ToggleInternalStatRecording() = 0;

    // Configuration options
    virtual void        SetListeningPort(UInt32 port) = 0;
    virtual void        SetBroadcastPort(UInt32 port) = 0;
    virtual void        SetNoBroadcast() { SetBroadcastPort(0); } //  call before connection is opened
    virtual void        SetConnectionWaitTime(unsigned waitTimeMilliseconds) = 0;
    virtual void        SetHeapLimit(UPInt memLimit) = 0;
    virtual void        SetInitSocketLib(bool initSocketLib) = 0;
    virtual void        SetSocketImplFactory(GFx::SocketImplFactory* socketFactory) = 0;

    // Message handler
    virtual bool        HandleNextMessage() = 0;

    // Specific messages
    virtual void    SendLog(const char* message, int messageLength, LogMessageId msgType) = 0;
    virtual void    SendCurrentState() = 0;
    virtual void    SendAppControlCaps() = 0;

    // AMP keeps track of active Movie Views
    virtual void        AddMovie(GFx::MovieImpl* movie) = 0;
    virtual void        RemoveMovie(GFx::MovieImpl* movie) = 0;
    // note: the returned movie is AddRef-ed!
    virtual bool        FindMovieByHeap(MemoryHeap* heap, GFx::MovieImpl** movie) = 0;

    // AMP keeps track of images
    virtual void        AddImage(GFx::ImageResource* image) = 0;
    virtual void        RemoveImage(GFx::ImageResource* image) = 0;

    // AMP keeps track of active Loaders (for renderer access)
    virtual void        AddLoader(GFx::Loader* loader) = 0;
    virtual void        RemoveLoader(GFx::Loader* loader) = 0;

    // Keeps track of active load processes
    virtual void        AddLoadProcess(GFx::LoadProcess* loadProcess) = 0;
    virtual void        RemoveLoadProcess(GFx::LoadProcess* loadProcess) = 0;

    // AMP keeps track of sound memory (when SF_MEMORY_ENABLE_DEBUG_INFO is not defined)
    virtual void        AddSound(UPInt soundMem) = 0;
    virtual void        RemoveSound(UPInt soundMem) = 0;

    // AMP keeps track of some renderer stats
    virtual void        AddStrokes(UInt32 numStrokes) = 0;
    virtual void        RemoveStrokes(UInt32 numStrokes) = 0;
    virtual void        IncrementFontFailures() = 0;
    virtual void        IncrementFontOptRead() = 0;
    virtual void        DecrementFontOptRead() = 0;
    
    // AMP renderer is used to render overdraw
    virtual void        SetRenderer(Render::HAL* renderer) = 0;

    // AMP server generates unique handles for each SWF
    // The 16-byte SWD debug ID is not used because it is not appropriate as a hash key
    // AMP keeps a map of SWD handles to actual SWD ID and filename
    virtual UInt32      GetNextSwdHandle() const = 0;
    virtual void        AddSwf(UInt32 swdHandle, const char* swdId, const char* filename) = 0;
    virtual String      GetSwdId(UInt32 handle) const = 0;
    virtual String      GetSwdFilename(UInt32 handle) const = 0;

    virtual void        AddSourceFile(UInt64 fileHandle, const char* fileName) = 0;

    // Renderer statistics
    virtual AmpStats*   GetDisplayStats() = 0;

    // GPA integration
    virtual SF_GPA_ITT_ID        GetGpaGroupId() = 0;
    virtual SF_GPA_ITT_DOMAIN*   GetGpaDomain() = 0;

protected:
    static AmpServer* AmpServerSingleton;
};


class AmpStats : public RefCountBase<AmpStats, Stat_Default_Mem>
{
public:
    virtual ~AmpStats() { }

    virtual void    NativePushCallstack(const char* functionName, AmpNativeFunctionId functionId, UInt64 funcTime) = 0;
    virtual void    NativePopCallstack(UInt64 time) = 0;
    virtual void    AddGcRoots(UInt32 numRoots) = 0;
    virtual void    AddGcFreedRoots(UInt32 numFreedRoots) = 0;
    virtual void    GetStats(StatBag* bag, bool reset) = 0;
    virtual void    SetMovieDef(GFx::MovieDef* movieDef) = 0;
    virtual void    SetName(const char* pcName) = 0;
};


// Intel GPA
#if defined(SF_PROFILE_GPA)

class GPAScopedTask
{
public:
    SF_GPA_ITT_DOMAIN* m_pDomain;

    inline GPAScopedTask(const char* szFunctionName)
    {
        m_pDomain = AmpServer::GetInstance().GetGpaDomain();
        __itt_task_begin(m_pDomain, __itt_null, __itt_null, __itt_string_handle_createA(szFunctionName));
        __itt_relation_add_to_current(m_pDomain, __itt_relation_is_child_of, AmpServer::GetInstance().GetGpaGroupId());
    }

    inline ~GPAScopedTask()
    {
        __itt_task_end(m_pDomain);
    }
};

#else

class GPAScopedTask
{
public:
    inline GPAScopedTask(const char* szFunctionName)
    {
        SF_UNUSED(szFunctionName);
    }
};

#endif



// This class keeps track of function execution time and call stack
// Time starts counting in constructor and stops in destructor
// Updates the view stats object with the results
class AmpFunctionTimer
{
public:
    AmpFunctionTimer(AmpStats* ampStats, const char* functionName, AmpProfileLevel profileLevel = Amp_Profile_Level_Low, AmpNativeFunctionId functionId = Amp_Native_Function_Id_Invalid) : 
            StartTicks(0), Stats(ampStats), GpaTask(functionName)
    { 
        if (!AmpServer::GetInstance().IsProfiling()
            || AmpServer::GetInstance().GetProfileLevel() < profileLevel)
        {
            Stats = NULL;
        }
        if (Stats != NULL)
        {
            StartTicks = Timer::GetProfileTicks();
            Stats->NativePushCallstack(functionName, functionId, StartTicks);
        }
    }
    ~AmpFunctionTimer()
    {
        if (Stats != NULL)
        {
            Stats->NativePopCallstack(Timer::GetProfileTicks() - StartTicks);
        }
    }
private:
    UInt64              StartTicks;
    AmpStats*           Stats;
    GPAScopedTask       GpaTask;
};


class AmpMovieObjectDesc : public RefCountBase<AmpMovieObjectDesc, Stat_Default_Mem>
{
public:
    StringLH Description;
    ArrayLH<Ptr<AmpMovieObjectDesc> > Children;
    void Read(File& str);
    void Write(File& str) const;
};


// Some macros for frequently-used methods
#ifdef SF_AMP_SERVER
#define SF_AMP_SCOPE_TIMER_ID(ampStats, functionName, functionId)   AmpFunctionTimer _amp_timer_##functionId((ampStats), (functionName), Amp_Profile_Level_Low, (functionId))
#define SF_AMP_SCOPE_TIMER(ampStats, functionName, profileLevel)    AmpFunctionTimer _amp_timer_((ampStats), (functionName), (profileLevel))
#define SF_AMP_SCOPE_RENDER_TIMER_ID(functionName, functionId)      AmpFunctionTimer _amp_timer_##functionId(AmpServer::GetInstance().GetDisplayStats(), (functionName), Amp_Profile_Level_Low, (functionId))
#define SF_AMP_SCOPE_RENDER_TIMER(functionName, profileLevel)       AmpFunctionTimer _amp_timer_(AmpServer::GetInstance().GetDisplayStats(), (functionName), (profileLevel))
#define SF_AMP_CODE(x) x
#else
#define SF_AMP_SCOPE_TIMER_ID(ampStats, functionName, functionId)
#define SF_AMP_SCOPE_TIMER(ampStats, functionName, profileLevel)
#define SF_AMP_SCOPE_RENDER_TIMER_ID(functionName, functionId)
#define SF_AMP_SCOPE_RENDER_TIMER(functionName, profileLevel)
#define SF_AMP_CODE(x)
#endif




}  // namespace Scaleform

#endif  //  INC_SF_Kernel_AmpInterface_H

