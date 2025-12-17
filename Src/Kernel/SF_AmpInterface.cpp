/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_AmpInterface.cpp
Content     :   AMP Interface
Created     :   2010
Authors     :   Alex Mantzaris

Notes       :   Interface to the Analyzer for Memory and Performance

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_AmpInterface.h"
#include "SF_HeapNew.h"
#include "SF_File.h"

namespace Scaleform {

// Serialization
void AmpMovieObjectDesc::Read(File& str)
{
    Description.Clear();
    UInt32 stringLength = str.ReadUInt32();
    for (UInt32 i = 0; i < stringLength; ++i)
    {
        Description.AppendChar(str.ReadSByte());
    }
    UInt32 numChildren = str.ReadUInt32();
    Children.Resize(numChildren);
    for (UInt32 i = 0; i < numChildren; ++i)
    {
        AmpMovieObjectDesc* child = SF_HEAP_AUTO_NEW(this) AmpMovieObjectDesc();
        child->Read(str);
        Children[i] = *child;
    }
}

// Serialization
void AmpMovieObjectDesc::Write(File& str) const
{
    str.WriteUInt32(static_cast<UInt32>(Description.GetLength()));
    for (UPInt i = 0; i < Description.GetLength(); ++i)
    {
        str.WriteSByte(Description[i]);
    }
    UInt32 numChildren = static_cast<UInt32>(Children.GetSize());
    str.WriteUInt32(numChildren);
    for (UInt32 i = 0; i < numChildren; ++i)
    {
        Children[i]->Write(str);
    }
}


class DefaultAmpServer : public AmpServer
{
public:
    virtual void    AdvanceFrame() { }
    virtual void    MovieAdvance(GFx::MovieImpl* movie) { SF_UNUSED(movie); }
    virtual void    SetAppControlCallback(GFx::AMP::AppControlInterface* callback) { SF_UNUSED(callback); }
    virtual bool    OpenConnection() { return false; }
    virtual void    CloseConnection() { }
    virtual bool    IsValidConnection() const { return false; }
    virtual UInt32  GetCurrentState() const { return 0; }
    virtual void    SetConnectedApp(const char* playerTitle) { SF_UNUSED(playerTitle); }
    virtual void    SetAaMode(const char* aaMode) { SF_UNUSED(aaMode); }
    virtual void    SetStrokeType(const char* strokeType) { SF_UNUSED(strokeType); }
    virtual void    SetCurrentLocale(const char* locale) { SF_UNUSED(locale); }
    virtual void    SetCurveTolerance(float tolerance) { SF_UNUSED(tolerance); }
    virtual void    UpdateState(const GFx::AMP::ServerState* state) { SF_UNUSED(state); }
    virtual bool    IsEnabled() const { return false; }
    virtual bool    IsPaused() const { return false; }
    virtual bool    IsProfiling() const { return false; }
    virtual bool    IsInstructionSampling() const { return false; }
    virtual bool    IsFunctionAggregation() const { return false; }
    virtual bool    IsInstructionProfiling() const { return false; }
    virtual AmpProfileLevel GetProfileLevel() const { return Amp_Profile_Level_Null; }      
    virtual void    SetProfileLevel(AmpProfileLevel profileLevel, bool lock) { SF_UNUSED2(profileLevel, lock); }   
    virtual void    SetMemReports(bool memReports, bool lock) { SF_UNUSED2(memReports, lock); }
    virtual void    SetAppControlCaps(const GFx::AMP::MessageAppControl* caps){ SF_UNUSED(caps); }
    virtual void    ToggleInternalStatRecording() {} ;
    virtual bool    HandleNextMessage() { return false; }
    virtual void    SendLog(const char* message, int messageLength, LogMessageId msgType) { SF_UNUSED3(message, messageLength, msgType); }
    virtual void    SendCurrentState() { }
    virtual void    SendAppControlCaps() { }
    virtual void    SetListeningPort(UInt32 port) { SF_UNUSED(port); }
    virtual void    SetBroadcastPort(UInt32 port) { SF_UNUSED(port); }
    virtual void    SetConnectionWaitTime(unsigned waitTimeMilliseconds) { SF_UNUSED(waitTimeMilliseconds); }
    virtual void    SetHeapLimit(UPInt memLimit) { SF_UNUSED(memLimit); }
    virtual void    SetInitSocketLib(bool initSocketLib) { SF_UNUSED(initSocketLib); }
    virtual void    SetSocketImplFactory(GFx::SocketImplFactory* socketFactory) { SF_UNUSED(socketFactory); }
    virtual void    AddMovie(GFx::MovieImpl* movie) { SF_UNUSED(movie); }
    virtual void    RemoveMovie(GFx::MovieImpl* movie) { SF_UNUSED(movie); }
    virtual bool    FindMovieByHeap(MemoryHeap*, GFx::MovieImpl**) { return false; }
    virtual void    AddImage(GFx::ImageResource* image) { SF_UNUSED(image); }
    virtual void    RemoveImage(GFx::ImageResource* image) { SF_UNUSED(image); }
    virtual void    AddLoader(GFx::Loader* loader) { SF_UNUSED(loader); }
    virtual void    RemoveLoader(GFx::Loader* loader) { SF_UNUSED(loader); }
    virtual void    AddLoadProcess(GFx::LoadProcess* loadProcess) { SF_UNUSED(loadProcess); }
    virtual void    RemoveLoadProcess(GFx::LoadProcess* loadProcess) { SF_UNUSED(loadProcess); }
    virtual void    AddSound(UPInt soundMem)  { SF_UNUSED(soundMem); }
    virtual void    RemoveSound(UPInt soundMem) { SF_UNUSED(soundMem); }
    virtual void    AddStrokes(UInt32 numStrokes) { SF_UNUSED(numStrokes); }
    virtual void    RemoveStrokes(UInt32 numStrokes) { SF_UNUSED(numStrokes); }
    virtual void    IncrementFontFailures() { }
    virtual void    IncrementFontOptRead() { }
    virtual void    DecrementFontOptRead() { }
    virtual void    SetRenderer(Render::Renderer2D* renderer) { SF_UNUSED(renderer); }
    virtual UInt32  GetNextSwdHandle() const { return 0; }
    virtual void    AddSwf(UInt32 swdHandle, const char* swdId, const char* filename) { SF_UNUSED3(swdHandle, swdId, filename); }
    virtual String  GetSwdId(UInt32 handle) const { SF_UNUSED(handle); return ""; }
    virtual String  GetSwdFilename(UInt32 handle) const { SF_UNUSED(handle); return ""; }
    virtual AmpStats* GetDisplayStats() { return NULL; }
    virtual SF_GPA_ITT_ID GetGpaGroupId() { return SF_GPA_ITT_NULL; }
    virtual SF_GPA_ITT_DOMAIN* GetGpaDomain() { return NULL; }
    virtual void    AddSourceFile(UInt64 fileHandle, const char* fileName) { SF_UNUSED2(fileHandle, fileName); }
};

#ifdef SF_AMP_SERVER

// Static
AmpServer* AmpServer::AmpServerSingleton = NULL;

// Static
AmpServer& AmpServer::GetInstance()
{
    // Use no-op DefaultAmpServer is amp wasn't initialized. 
    if (!AmpServerSingleton)
        Init();
    SF_ASSERT(AmpServerSingleton != NULL);
    return *AmpServerSingleton;
}

// Static
void AmpServer::Init()
{
    static DefaultAmpServer ampSingleton;
    AmpServerSingleton = &ampSingleton; 
}

// Static
void AmpServer::Uninit()
{
    AmpServerSingleton = NULL;
}

#endif  // SF_AMP_SERVER

}  // namespace Scaleform

