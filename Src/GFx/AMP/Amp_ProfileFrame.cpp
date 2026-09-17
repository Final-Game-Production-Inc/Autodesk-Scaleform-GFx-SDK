#include "Amp_ProfileFrame.h"

#ifdef SF_ENABLE_STATS

#include "Kernel/SF_MemItem.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MemItem.h"
#include "Render/Render_MeshCache.h"
#include "Amp_ViewStats.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// utility function
void readString(File& file, String* str)
{
    UInt32 stringLength = file.ReadUInt32();
    for (UInt32 i = 0; i < stringLength; ++i)
    {
        str->AppendChar(file.ReadSByte());
    }
}

// utility function
void writeString(File& file, const String& str)
{
    file.WriteUInt32(static_cast<UInt32>(str.GetLength()));
    for (UPInt i = 0; i < str.GetLength(); ++i)
    {
        file.WriteSByte(str[i]);
    }
}


// Constructor
ProfileFrame::ProfileFrame() :
    TimeStamp(0),
    FramesPerSecond(0),
    ProfilingLevel(0),
    DetailedMemReport(false),
    AdvanceTime(0),
    ActionTime(0),
    TimelineTime(0),
    InputTime(0),
    MouseTime(0),
    GcCollectTime(0),
    GcMarkInCycleTime(0),
    GcScanInUseTime(0),
    GcFreeGarbageTime(0),
    GcFinalizeTime(0),
    GcDelayedCleanupTime(0),
    GetVariableTime(0),
    SetVariableTime(0),
    InvokeTime(0),
    DisplayTime(0),
    PresentTime(0),
    FontThrashingTime(0),
    GradientGenTime(0),
    FontMissTime(0),
    UserTime(0),
    UserCalls(0),
    LineCount(0),
    MaskCount(0),
    FilterCount(0),
    MeshCount(0),
    TriangleCount(0),
    DrawPrimitiveCount(0),
    StrokeCount(0),
    GradientFillCount(0),
    MeshThrashing(0),
    RasterizedGlyphCount(0),
    FontTextureCount(0),
    NumFontCacheTextureUpdates(0),
    FontThrashing(0),
    FontFill(0),
    FontFail(0),
    FontMisses(0),
    FontTotalArea(0),
    FontUsedArea(0),
    FontOptRead(0),
    TotalMemory(0),
    ImageMemory(0),
    ImageGraphicsMemory(0),
    MovieDataMemory(0),
    FontMemory(0),
    MovieViewMemory(0),
    AS3Memory(0),
    MeshCacheMemory(0),
    MeshCacheGraphicsMemory(0),
    MeshCacheUnusedMemory(0),
    MeshCacheGraphicsUnusedMemory(0),
    FontCacheMemory(0),
    VideoMemory(0),
    SoundMemory(0),
    OtherMemory(0),
    GcRootsNumber(0),
    GcFreedRootsNumber(0)
{
    MemoryByStatId = *SF_HEAP_AUTO_NEW(this) MemItem(0);
    Images = *SF_HEAP_AUTO_NEW(this) MemItem(0);
    Fonts = *SF_HEAP_AUTO_NEW(this) MemItem(0);
    DisplayStats = *SF_HEAP_AUTO_NEW(this) MovieFunctionStats();
    DisplayFunctionStats = *SF_HEAP_AUTO_NEW(this) MovieFunctionTreeStats();
}


ProfileFrame& ProfileFrame::operator+=(const ProfileFrame& rhs)
{
    TimeStamp += rhs.TimeStamp;
    FramesPerSecond += rhs.FramesPerSecond;
    ProfilingLevel = Alg::Max(ProfilingLevel, rhs.ProfilingLevel);
    DetailedMemReport = (DetailedMemReport || rhs.DetailedMemReport);
    AdvanceTime += rhs.AdvanceTime;
    TimelineTime += rhs.TimelineTime;
    ActionTime += rhs.ActionTime;
    InputTime += rhs.InputTime;
    MouseTime += rhs.MouseTime;
    GcCollectTime += rhs.GcCollectTime;
    GcMarkInCycleTime += rhs.GcMarkInCycleTime;
    GcScanInUseTime += rhs.GcScanInUseTime;
    GcFreeGarbageTime += rhs.GcFreeGarbageTime;
    GcFinalizeTime += rhs.GcFinalizeTime;
    GcDelayedCleanupTime += rhs.GcDelayedCleanupTime;
    GetVariableTime += rhs.GetVariableTime;
    SetVariableTime += rhs.SetVariableTime;
    InvokeTime += rhs.InvokeTime;
    DisplayTime += rhs.DisplayTime;
    PresentTime += rhs.PresentTime;
    FontThrashingTime += rhs.FontThrashingTime;
    GradientGenTime += rhs.GradientGenTime;
    FontMissTime += rhs.FontMissTime;
    UserTime += rhs.UserTime;
    UserCalls += rhs.UserCalls;
    LineCount += rhs.LineCount;
    MaskCount += rhs.MaskCount;
    FilterCount += rhs.FilterCount;
    MeshCount += rhs.MeshCount;
    TriangleCount += rhs.TriangleCount;
    DrawPrimitiveCount += rhs.DrawPrimitiveCount;
    StrokeCount += rhs.StrokeCount;
    GradientFillCount += rhs.GradientFillCount;
    MeshThrashing += rhs.MeshThrashing;
    RasterizedGlyphCount += rhs.RasterizedGlyphCount;
    FontTextureCount += rhs.FontTextureCount;
    NumFontCacheTextureUpdates += rhs.NumFontCacheTextureUpdates;
    FontThrashing += rhs.FontThrashing;
    FontFill += rhs.FontFill;
    FontFail += rhs.FontFail;
    FontMisses += rhs.FontMisses;
    FontTotalArea += rhs.FontTotalArea;
    FontUsedArea += rhs.FontUsedArea;
    FontOptRead = Alg::Max(rhs.FontOptRead, FontOptRead);
    TotalMemory += rhs.TotalMemory;
    ImageMemory += rhs.ImageMemory;
    ImageGraphicsMemory += rhs.ImageGraphicsMemory;
    MovieDataMemory += rhs.MovieDataMemory;
    FontMemory += rhs.FontMemory;
    MovieViewMemory += rhs.MovieViewMemory;
    AS3Memory += rhs.AS3Memory;
    MeshCacheMemory += rhs.MeshCacheMemory;
    MeshCacheGraphicsMemory += rhs.MeshCacheGraphicsMemory;
    MeshCacheUnusedMemory += rhs.MeshCacheUnusedMemory;
    MeshCacheGraphicsUnusedMemory += rhs.MeshCacheGraphicsUnusedMemory;
    FontCacheMemory += rhs.FontCacheMemory;
    VideoMemory += rhs.VideoMemory;
    SoundMemory += rhs.SoundMemory;
    OtherMemory += rhs.OtherMemory;
    GcRootsNumber += rhs.GcRootsNumber;
    GcFreedRootsNumber += rhs.GcFreedRootsNumber;

    for (UPInt i = 0; i < rhs.MovieStats.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < MovieStats.GetSize(); ++j)
        {
            if (MovieStats[j]->ViewHandle == rhs.MovieStats[i]->ViewHandle)
            {
                MovieStats[j]->Merge(*rhs.MovieStats[i]);
                found = true;
                break;
            }
        }
        
        if (!found)
        {
            MovieProfile* newStats = SF_HEAP_AUTO_NEW(this) MovieProfile();
            *newStats = *rhs.MovieStats[i];
            MovieStats.PushBack(*newStats);
        }
    }
    DisplayStats->Merge(*rhs.DisplayStats);
    DisplayFunctionStats->Merge(*rhs.DisplayFunctionStats);

    for (UPInt i = 0; i < rhs.SwdHandles.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < SwdHandles.GetSize(); ++j)
        {
            if (SwdHandles[j] == rhs.SwdHandles[i])
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            SwdHandles.PushBack(rhs.SwdHandles[i]);
        }
    }

    for (UPInt i = 0; i < rhs.FileHandles.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < FileHandles.GetSize(); ++j)
        {
            if (FileHandles[j] == rhs.FileHandles[i])
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            FileHandles.PushBack(rhs.FileHandles[i]);
        }
    }

    MemoryByStatId->Merge(*rhs.MemoryByStatId);
    Images->Merge(*rhs.Images);
    Fonts->Merge(*rhs.Fonts);

    for (UPInt i = 0; i < rhs.ImageList.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < ImageList.GetSize(); ++j)
        {
            if (ImageList[j]->Id == rhs.ImageList[i]->Id)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            ImageList.PushBack(rhs.ImageList[i]);
        }
    }

    return *this;
}

ProfileFrame& ProfileFrame::operator/=(unsigned numFrames)
{
    TimeStamp /= numFrames;
    FramesPerSecond /= numFrames;
    AdvanceTime /= numFrames;
    TimelineTime /= numFrames;
    ActionTime /= numFrames;
    InputTime /= numFrames;
    MouseTime /= numFrames;
    GcCollectTime /= numFrames;
    GcMarkInCycleTime /= numFrames;
    GcScanInUseTime /= numFrames;
    GcFreeGarbageTime /= numFrames;
    GcFinalizeTime /= numFrames;
    GcDelayedCleanupTime /= numFrames;
    GetVariableTime /= numFrames;
    SetVariableTime /= numFrames;
    InvokeTime /= numFrames;
    DisplayTime /= numFrames;
    PresentTime /= numFrames;
    FontThrashingTime /= numFrames;
    GradientGenTime /= numFrames;
    FontMissTime /= numFrames;
    UserTime /= numFrames;
    UserCalls /= numFrames;
    LineCount /= numFrames;
    MaskCount /= numFrames;
    FilterCount /= numFrames;
    MeshCount /= numFrames;
    TriangleCount /= numFrames;
    DrawPrimitiveCount /= numFrames;
    StrokeCount /= numFrames;
    GradientFillCount /= numFrames;
    MeshThrashing /= numFrames;
    RasterizedGlyphCount /= numFrames;
    FontTextureCount /= numFrames;
    NumFontCacheTextureUpdates /= numFrames;
    FontThrashing /= numFrames;
    FontFill /= numFrames;
    FontFail /= numFrames;
    FontMisses /= numFrames;
    FontTotalArea /= numFrames;
    FontUsedArea /= numFrames;
    TotalMemory /= numFrames;
    ImageMemory /= numFrames;
    ImageGraphicsMemory /= numFrames;
    MovieDataMemory /= numFrames;
    FontMemory /= numFrames;
    MovieViewMemory /= numFrames;
    AS3Memory /= numFrames;
    MeshCacheMemory /= numFrames;
    MeshCacheGraphicsMemory /= numFrames;
    MeshCacheUnusedMemory /= numFrames;
    MeshCacheGraphicsUnusedMemory /= numFrames;
    FontCacheMemory /= numFrames;
    VideoMemory /= numFrames;
    SoundMemory /= numFrames;
    OtherMemory /= numFrames;
    GcRootsNumber /= numFrames;
    GcFreedRootsNumber /= numFrames;

    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        *MovieStats[i] /= numFrames;
    }
    *DisplayStats /= numFrames;

    *MemoryByStatId /= numFrames;
    *Images /= numFrames;
    *Fonts /= numFrames;

    return *this;
}


ProfileFrame& ProfileFrame::operator*=(unsigned num)
{
    TimeStamp *= num;
    FramesPerSecond *= num;
    AdvanceTime *= num;
    TimelineTime *= num;
    ActionTime *= num;
    InputTime *= num;
    MouseTime *= num;
    GcCollectTime *= num;
    GcMarkInCycleTime *= num;
    GcScanInUseTime *= num;
    GcFreeGarbageTime *= num;
    GcFinalizeTime *= num;
    GcDelayedCleanupTime *= num;
    GetVariableTime *= num;
    SetVariableTime *= num;
    InvokeTime *= num;
    DisplayTime *= num;
    PresentTime *= num;
    FontThrashingTime *= num;
    GradientGenTime *= num;
    FontMissTime *= num;
    UserTime *= num;
    UserCalls *= num;
    LineCount *= num;
    MaskCount *= num;
    FilterCount *= num;
    MeshCount *= num;
    TriangleCount *= num;
    DrawPrimitiveCount *= num;
    StrokeCount *= num;
    GradientFillCount *= num;
    MeshThrashing *= num;
    RasterizedGlyphCount *= num;
    FontTextureCount *= num;
    NumFontCacheTextureUpdates *= num;
    FontThrashing *= num;
    FontFill *= num;
    FontFail *= num;
    FontMisses *= num;
    FontTotalArea *= num;
    FontUsedArea *= num;
    TotalMemory *= num;
    ImageMemory *= num;
    ImageGraphicsMemory *= num;
    MovieDataMemory *= num;
    FontMemory *= num;
    MovieViewMemory *= num;
    AS3Memory *= num;
    MeshCacheMemory *= num;
    MeshCacheGraphicsMemory *= num;
    MeshCacheUnusedMemory *= num;
    MeshCacheGraphicsUnusedMemory *= num;
    FontCacheMemory *= num;
    VideoMemory *= num;
    SoundMemory *= num;
    OtherMemory *= num;
    GcRootsNumber *= num;
    GcFreedRootsNumber *= num;

    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        *MovieStats[i] *= num;
    }
    *DisplayStats *= num;

    *MemoryByStatId *= num;
    *Images *= num;
    *Fonts *= num;

    return *this;
}


// Serialization
void ProfileFrame::Read(File& str, UInt32 version)
{
    TimeStamp = str.ReadUInt64();
    FramesPerSecond = str.ReadUInt32();
    if (version >= 33)
    {
        ProfilingLevel = str.ReadUInt32();
        DetailedMemReport = (str.ReadUByte() != 0);
    }
    AdvanceTime = str.ReadUInt32();
    TimelineTime = str.ReadUInt32();
    ActionTime = str.ReadUInt32();
    if (version < 21)
    {
        str.ReadUInt32();
    }
    InputTime = str.ReadUInt32();
    MouseTime = str.ReadUInt32();
    if (version >= 32)
    {
        GcCollectTime = str.ReadUInt32();
        GcMarkInCycleTime = str.ReadUInt32();
        GcScanInUseTime = str.ReadUInt32();
        GcFreeGarbageTime = str.ReadUInt32();
        GcFinalizeTime = str.ReadUInt32();
        GcDelayedCleanupTime = str.ReadUInt32();
    }
    GetVariableTime = str.ReadUInt32();
    SetVariableTime = str.ReadUInt32();
    InvokeTime = str.ReadUInt32();
    DisplayTime = str.ReadUInt32();
    if (version >= 30)
    {
        PresentTime = str.ReadUInt32();
    }
    FontThrashingTime = str.ReadUInt32();
    GradientGenTime = str.ReadUInt32();
    if (version >= 42)
    {
        FontMissTime = str.ReadUInt32();
    }
    UserTime = str.ReadUInt32();
    if (version >= 37)
    {
        UserCalls = str.ReadUInt32();
    }
    LineCount = str.ReadUInt32();
    MaskCount = str.ReadUInt32();
    FilterCount = str.ReadUInt32();
    if (version >= 16)
    {
        MeshCount = str.ReadUInt32();
    }
    TriangleCount = str.ReadUInt32();
    DrawPrimitiveCount = str.ReadUInt32();
    StrokeCount = str.ReadUInt32();
    GradientFillCount = str.ReadUInt32();
    MeshThrashing = str.ReadUInt32();
    RasterizedGlyphCount = str.ReadUInt32();
    FontTextureCount = str.ReadUInt32();
    NumFontCacheTextureUpdates = str.ReadUInt32();
    if (version >= 14)
    {
        FontThrashing = str.ReadUInt32();
        FontFill = str.ReadUInt32();
        FontFail = str.ReadUInt32();
        if (version >= 24)
        {
            FontMisses = str.ReadUInt32();
        }
        if (version >= 27)
        {
            FontTotalArea = str.ReadUInt32();
            FontUsedArea = str.ReadUInt32();
        }
        if (version >= 34)
        {
            FontOptRead = str.ReadUInt32();
        }
    }
    TotalMemory = str.ReadUInt32();
    ImageMemory = str.ReadUInt32();
    if (version >= 29)
    {
        ImageGraphicsMemory = str.ReadUInt32();
    }
    MovieDataMemory = str.ReadUInt32();
    if (version >= 40)
    {
        FontMemory = str.ReadUInt32();
    }
    MovieViewMemory = str.ReadUInt32();
    if (version >= 35)
    {
        AS3Memory = str.ReadUInt32();
    }
    MeshCacheMemory = str.ReadUInt32();
    if (version >= 28)
    {
        MeshCacheGraphicsMemory = str.ReadUInt32();
        MeshCacheUnusedMemory = str.ReadUInt32();
        MeshCacheGraphicsUnusedMemory = str.ReadUInt32();
    }
    FontCacheMemory = str.ReadUInt32();
    VideoMemory = str.ReadUInt32();
    SoundMemory = str.ReadUInt32();
    OtherMemory = str.ReadUInt32();
    if (version >= 32)
    {
        GcRootsNumber = str.ReadUInt32();
        GcFreedRootsNumber = str.ReadUInt32();
    }

    MovieStats.Resize(str.ReadUInt32());
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieStats[i] = *SF_HEAP_AUTO_NEW(this) MovieProfile();
        MovieStats[i]->Read(str, version);
    }
    if (version >= 15)
    {
        DisplayStats->Read(str, version);
    }
    if (version >= 25)
    {
        DisplayFunctionStats->Read(str, version);
    }
    SwdHandles.Resize(str.ReadUInt32());
    for (UPInt i = 0; i < SwdHandles.GetSize(); ++i)
    {
        SwdHandles[i] = str.ReadUInt32();
    }
    if (version >= 9)
    {
        FileHandles.Resize(str.ReadUInt32());
        for (UPInt i = 0; i < FileHandles.GetSize(); ++i)
        {
            FileHandles[i] = str.ReadUInt64();
        }
    }
    MemoryByStatId->Read(str, version);
    if (version <= 18)
    {
        Ptr<MemItem> tmp = *SF_HEAP_AUTO_NEW(this) MemItem(0);
        tmp->Read(str, version);
    }
    if (version >= 3)
    {
        Images->Read(str, version);
    }
    if (version >= 7)
    {
        Fonts->Read(str, version);
    }
    if (version >= 17)
    {
        ImageList.Resize(str.ReadUInt32());
        for (UPInt i = 0; i < ImageList.GetSize(); ++i)
        {
            ImageList[i] = *SF_HEAP_AUTO_NEW(this) ImageInfo();
            ImageList[i]->Read(str, version);
        }
    }

    if (version < 8)
    {
        str.ReadUInt32();
        str.ReadUInt32();
    }
}

// Serialization
void ProfileFrame::Write(File& str, UInt32 version) const
{
    str.WriteUInt64(TimeStamp);
    str.WriteUInt32(FramesPerSecond);
    if (version >= 33)
    {
        str.WriteUInt32(ProfilingLevel);
        str.WriteUByte(DetailedMemReport ? 1 : 0);
    }
    str.WriteUInt32(AdvanceTime);
    str.WriteUInt32(TimelineTime);
    str.WriteUInt32(ActionTime);
    if (version < 21)
    {
        str.WriteUInt32(0);
    }
    str.WriteUInt32(InputTime);
    str.WriteUInt32(MouseTime);
    if (version >= 32)
    {
        str.WriteUInt32(GcCollectTime);
        str.WriteUInt32(GcMarkInCycleTime);
        str.WriteUInt32(GcScanInUseTime);
        str.WriteUInt32(GcFreeGarbageTime);
        str.WriteUInt32(GcFinalizeTime);
        str.WriteUInt32(GcDelayedCleanupTime);
    }
    str.WriteUInt32(GetVariableTime);
    str.WriteUInt32(SetVariableTime);
    str.WriteUInt32(InvokeTime);
    str.WriteUInt32(DisplayTime);
    if (version >= 30)
    {
        str.WriteUInt32(PresentTime);
    }
    str.WriteUInt32(FontThrashingTime);
    str.WriteUInt32(GradientGenTime);
    if (version >= 42)
    {
        str.WriteUInt32(FontMissTime);
    }
    str.WriteUInt32(UserTime);
    if (version >= 37)
    {
        str.WriteUInt32(UserCalls);
    }
    str.WriteUInt32(LineCount);
    str.WriteUInt32(MaskCount);
    str.WriteUInt32(FilterCount);
    if (version >= 16)
    {
        str.WriteUInt32(MeshCount);
    }
    str.WriteUInt32(TriangleCount);
    str.WriteUInt32(DrawPrimitiveCount);
    str.WriteUInt32(StrokeCount);
    str.WriteUInt32(GradientFillCount);
    str.WriteUInt32(MeshThrashing);
    str.WriteUInt32(RasterizedGlyphCount);
    str.WriteUInt32(FontTextureCount);
    str.WriteUInt32(NumFontCacheTextureUpdates);
    if (version >= 14)
    {
        str.WriteUInt32(FontThrashing);
        str.WriteUInt32(FontFill);
        str.WriteUInt32(FontFail);
        if (version >= 24)
        {
            str.WriteUInt32(FontMisses);
        }
        if (version >= 27)
        {
            str.WriteUInt32(FontTotalArea);
            str.WriteUInt32(FontUsedArea);
        }
        if (version >= 34)
        {
            str.WriteUInt32(FontOptRead);
        }
    }
    str.WriteUInt32(TotalMemory);
    str.WriteUInt32(ImageMemory);
    if (version >= 29)
    {
        str.WriteUInt32(ImageGraphicsMemory);
    }
    str.WriteUInt32(MovieDataMemory);
    if (version >= 40)
    {
        str.WriteUInt32(FontMemory);
    }
    str.WriteUInt32(MovieViewMemory);
    if (version >= 35)
    {
        str.WriteUInt32(AS3Memory);
    }
    str.WriteUInt32(MeshCacheMemory);
    if (version >= 28)
    {
        str.WriteUInt32(MeshCacheGraphicsMemory);
        str.WriteUInt32(MeshCacheUnusedMemory);
        str.WriteUInt32(MeshCacheGraphicsUnusedMemory);
    }
    str.WriteUInt32(FontCacheMemory);
    str.WriteUInt32(VideoMemory);
    str.WriteUInt32(SoundMemory);
    str.WriteUInt32(OtherMemory);
    if (version >= 32)
    {
        str.WriteUInt32(GcRootsNumber);
        str.WriteUInt32(GcFreedRootsNumber);
    }

    str.WriteUInt32(static_cast<UInt32>(MovieStats.GetSize()));
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieStats[i]->Write(str, version);
    }
    if (version >= 15)
    {
        DisplayStats->Write(str, version);
    }
    if (version >= 25)
    {
        DisplayFunctionStats->Write(str, version);
    }    
    str.WriteUInt32(static_cast<UInt32>(SwdHandles.GetSize()));
    for (UPInt i = 0; i < SwdHandles.GetSize(); ++i)
    {
        str.WriteUInt32(SwdHandles[i]);
    }
    if (version >= 9)
    {
        str.WriteUInt32(static_cast<UInt32>(FileHandles.GetSize()));
        for (UPInt i = 0; i < FileHandles.GetSize(); ++i)
        {
            str.WriteUInt64(FileHandles[i]);
        }
    }
    MemoryByStatId->Write(str, version);
    if (version <= 18)
    {
        Ptr<MemItem> tmp = *SF_HEAP_AUTO_NEW(this) MemItem(0);
        tmp->Write(str, version);
    }
    if (version >= 3)
    {
        Images->Write(str, version);
    }
    if (version >= 7)
    {
        Fonts->Write(str, version);
    }
    if (version >= 17)
    {
        str.WriteUInt32(static_cast<UInt32>(ImageList.GetSize()));
        for (UInt32 i = 0; i < ImageList.GetSize(); ++i)
        {
            ImageList[i]->Write(str, version);
        }
    }

    if (version < 8)
    {
        str.WriteUInt32(0);
        str.WriteUInt32(0);
    }
}

void ProfileFrame::Print(Log* log) const
{
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieStats[i]->Print(log);
    }
    log->LogMessage("========== RENDERER FUNCTIONS ===============\n");
    DisplayStats->Print(log);
    DisplayFunctionStats->Print(log);
}


int ProfileFrame::GetNumFunctionTreeStats() const
{
    int numFunctionTreeStats = 0;
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieFunctionTreeStats* treeStats = MovieStats[i]->FunctionTreeStats;
        if (treeStats != NULL)
        {
            for (UPInt j = 0; j < treeStats->FunctionRoots.GetSize(); ++j)
            {
                numFunctionTreeStats += treeStats->FunctionRoots[j]->GetNumItems();
            }
        }
    }
    if (DisplayFunctionStats)
    {
        for (UPInt j = 0; j < DisplayFunctionStats->FunctionRoots.GetSize(); ++j)
        {
            numFunctionTreeStats += DisplayFunctionStats->FunctionRoots[j]->GetNumItems();
        }
    }
    return numFunctionTreeStats;
}

void ProfileFrame::AccumulateTreeFunctionStats(bool includeActionScript)
{
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieFunctionTreeStats* treeStats = MovieStats[i]->FunctionTreeStats;
        if (treeStats != NULL)
        {
            MovieStats[i]->FunctionStats = *treeStats->Accumulate(includeActionScript);
        }
    }
    if (DisplayFunctionStats)
    {
        DisplayStats = *DisplayFunctionStats->Accumulate(includeActionScript);
    }
}

//////////////////////////////////////////////////////////

// Constructor
MovieProfile::MovieProfile() : 
    ViewHandle(0),
    MinFrame(0),
    MaxFrame(0)
{   
    InstructionStats = *SF_HEAP_AUTO_NEW(this) MovieInstructionStats();
    FunctionStats = *SF_HEAP_AUTO_NEW(this) MovieFunctionStats();
    SourceLineStats = *SF_HEAP_AUTO_NEW(this) MovieSourceLineStats();
    FunctionTreeStats = *SF_HEAP_AUTO_NEW(this) MovieFunctionTreeStats();
}


void MovieProfile::Merge(const MovieProfile& rhs)
{
    SF_ASSERT(ViewHandle == rhs.ViewHandle);
    SF_ASSERT(ViewName == rhs.ViewName);
    MinFrame = Alg::Min(MinFrame, rhs.MinFrame);
    MaxFrame = Alg::Max(MaxFrame, rhs.MaxFrame);

    for (UPInt i = 0; i < rhs.Markers.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < Markers.GetSize(); ++j)
        {
            if (rhs.Markers[i]->Name == Markers[j]->Name)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Markers.PushBack(Markers[i]);
        }
    }

    InstructionStats->Merge(*rhs.InstructionStats);
    FunctionStats->Merge(*rhs.FunctionStats);
    SourceLineStats->Merge(*rhs.SourceLineStats);
    FunctionTreeStats->Merge(*rhs.FunctionTreeStats);
}

MovieProfile& MovieProfile::operator=(const MovieProfile& rhs)
{
    ViewHandle = rhs.ViewHandle;
    MinFrame = rhs.MinFrame;
    MaxFrame = rhs.MaxFrame;
    ViewName = rhs.ViewName;
    Version = rhs.Version;
    Width = rhs.Width;
    Height = rhs.Height;
    FrameRate = rhs.FrameRate;
    FrameCount = rhs.FrameCount;
    Markers = rhs.Markers;
    *InstructionStats = *rhs.InstructionStats;
    *FunctionStats = *rhs.FunctionStats;
    *SourceLineStats = *rhs.SourceLineStats;
    *FunctionTreeStats = *rhs.FunctionTreeStats;
    return *this;
}

MovieProfile& MovieProfile::operator/=(unsigned numFrames)
{
    *InstructionStats /= numFrames;
    *FunctionStats /= numFrames;
    *SourceLineStats /= numFrames;
    return *this;
}

MovieProfile& MovieProfile::operator*=(unsigned num)
{
    *InstructionStats *= num;
    *FunctionStats *= num;
    *SourceLineStats *= num;
    return *this;
}

// Serialization
void MovieProfile::Read(File& str, UInt32 version)
{
    ViewHandle = str.ReadUInt32();
    MinFrame = str.ReadUInt32();
    MaxFrame = str.ReadUInt32();
    if (version >= 4)
    {
        readString(str, &ViewName);
        Version = str.ReadUInt32();
        Width = str.ReadFloat();
        Height = str.ReadFloat();
        FrameRate = str.ReadFloat();
        FrameCount = str.ReadUInt32();
    }
    if (version >= 6)
    {
        UInt32 numMarkers = str.ReadUInt32();
        Markers.Resize(numMarkers);
        for (UInt32 i = 0; i < numMarkers; ++i)
        {
            Markers[i] = *SF_HEAP_AUTO_NEW(this) MarkerInfo();
            if (version >= 11)
            {
                 readString(str, &Markers[i]->Name);
            }
            else
            {
                Markers[i]->Name = "Marker";
            }
            Markers[i]->Number = str.ReadUInt32();
        }
    }
    SF_ASSERT(InstructionStats);
    InstructionStats->Read(str, version);
    SF_ASSERT(FunctionStats);
    FunctionStats->Read(str, version);
    SF_ASSERT(SourceLineStats);
    SourceLineStats->Read(str, version);
    if (version >= 25)
    {
        SF_ASSERT(SourceLineStats);
        FunctionTreeStats->Read(str, version);
    }
}

// Serialization
void MovieProfile::Write(File& str, UInt32 version) const
{
    str.WriteUInt32(ViewHandle);
    str.WriteUInt32(MinFrame);
    str.WriteUInt32(MaxFrame);
    if (version >= 4)
    {
        writeString(str, ViewName);
        str.WriteUInt32(Version);
        str.WriteFloat(Width);
        str.WriteFloat(Height);
        str.WriteFloat(FrameRate);
        str.WriteUInt32(FrameCount);
    }
    if (version >= 6)
    {
        str.WriteUInt32(static_cast<UInt32>(Markers.GetSize()));
        for (UPInt i = 0; i < Markers.GetSize(); ++i)
        {
            if (version >= 11)
            {
                writeString(str, Markers[i]->Name);
            }
            str.WriteUInt32(Markers[i]->Number);
        }
    }
    SF_ASSERT(InstructionStats);
    InstructionStats->Write(str, version);
    SF_ASSERT(FunctionStats);
    FunctionStats->Write(str, version);
    SF_ASSERT(SourceLineStats);
    SourceLineStats->Write(str, version);
    if (version >= 25)
    {
        SF_ASSERT(FunctionTreeStats);
        FunctionTreeStats->Write(str, version);
    }
}


void MovieProfile::Print(Log* log) const
{
    log->LogMessage("========== MOVIE VIEW FUNCTIONS FOR %s ======\n", ViewName.ToCStr());
    FunctionStats->Print(log);
    FunctionTreeStats->Print(log);
}


/////////////////////////////////////////////////////

void ImageInfo::Read(File& str, UInt32 version)
{
    SF_UNUSED(version);
    Id = str.ReadUInt32();
    readString(str, &Name);
    if (version < 29)
    {
        String dummy;
        readString(str, &dummy);
    }
    Bytes = str.ReadUInt32();
    External = (str.ReadUByte() != 0);
    AtlasId = str.ReadUInt32();
    AtlasTop = str.ReadUInt32();
    AtlasBottom = str.ReadUInt32();
    AtlasLeft = str.ReadUInt32();
    AtlasRight = str.ReadUInt32();
}

void ImageInfo::Write(File& str, UInt32 version) const
{
    SF_UNUSED(version);
    str.WriteUInt32(Id);
    writeString(str, Name);
    if (version < 29)
    {
        writeString(str, "");
    }
    str.WriteUInt32(Bytes);
    str.WriteUByte(External ? 1 : 0);
    str.WriteUInt32(AtlasId);
    str.WriteUInt32(AtlasTop);
    str.WriteUInt32(AtlasBottom);
    str.WriteUInt32(AtlasLeft);
    str.WriteUInt32(AtlasRight);
}

/////////////////////////////////////////////////////

MovieInstructionStats& MovieInstructionStats::operator/=(unsigned numFrames)
{
    for (UPInt i = 0; i < BufferStatsArray.GetSize(); ++i)
    {
        ArrayLH<InstructionTimePair>& times = BufferStatsArray[i]->InstructionTimesArray;
        for (UPInt j = 0; j < times.GetSize(); ++j)
        {
            times[j].Time /= numFrames;
        }
    }
    return *this;
}

MovieInstructionStats& MovieInstructionStats::operator*=(unsigned num)
{
    for (UPInt i = 0; i < BufferStatsArray.GetSize(); ++i)
    {
        ArrayLH<InstructionTimePair>& times = BufferStatsArray[i]->InstructionTimesArray;
        for (UPInt j = 0; j < times.GetSize(); ++j)
        {
            times[j].Time *= num;
        }
    }
    return *this;
}


void MovieInstructionStats::Merge(const MovieInstructionStats& other)
{
    for (UPInt i = 0; i < other.BufferStatsArray.GetSize(); ++i)
    {
        bool buffersMerged = false;
        for (UPInt j = 0; j < BufferStatsArray.GetSize(); ++j)
        {
            if (BufferStatsArray[j]->SwdHandle == other.BufferStatsArray[i]->SwdHandle
                && BufferStatsArray[j]->BufferOffset == other.BufferStatsArray[i]->BufferOffset)
            {
                SF_ASSERT(BufferStatsArray[j]->BufferLength == other.BufferStatsArray[i]->BufferLength);
                ArrayLH<InstructionTimePair>& thisArray = BufferStatsArray[j]->InstructionTimesArray;
                const ArrayLH<InstructionTimePair>& otherArray = BufferStatsArray[i]->InstructionTimesArray;
                for (UPInt k = 0; k < otherArray.GetSize(); ++k)
                {
                    bool instructionsMerged = false;
                    for (UPInt l = 0; l < thisArray.GetSize(); ++l)
                    {
                        if (thisArray[l].Offset == otherArray[k].Offset)
                        {
                            thisArray[l].Time += otherArray[k].Time;
                            instructionsMerged = true;
                            break;
                        }
                    }

                    if (!instructionsMerged)
                    {
                        thisArray.PushBack(otherArray[k]);
                    }
                }
                buffersMerged = true;
                break;
            }
        }

        if (!buffersMerged)
        {
            ScriptBufferStats* newBufferStats = SF_HEAP_AUTO_NEW(this) ScriptBufferStats();
            *newBufferStats = *other.BufferStatsArray[i];
            BufferStatsArray.PushBack(*newBufferStats);
        }
    }
}


// Serialization
void MovieInstructionStats::Read(File& str, UInt32 version)
{
    UInt32 iSize = str.ReadUInt32();
    BufferStatsArray.Resize(iSize);
    for (UPInt i = 0; i < BufferStatsArray.GetSize(); ++i)
    {
        BufferStatsArray[i] = *SF_HEAP_AUTO_NEW(this) ScriptBufferStats();
        BufferStatsArray[i]->Read(str, version);
    }
}

// Serialization
void MovieInstructionStats::Write(File& str, UInt32 version) const
{
    str.WriteUInt32(static_cast<UInt32>(BufferStatsArray.GetSize()));
    for (UPInt i = 0; i < BufferStatsArray.GetSize(); ++i)
    {
        BufferStatsArray[i]->Write(str, version);
    }
}

///////////////////////////////////////////////////////////

// Serialization
void MovieInstructionStats::ScriptBufferStats::Read(File& str, UInt32 version)
{
    SF_UNUSED(version);
    SwdHandle = str.ReadUInt32();
    BufferOffset = str.ReadUInt32();
    BufferLength = str.ReadUInt32();
    UInt32 iSize = str.ReadUInt32();
    InstructionTimesArray.Resize(iSize);
    for (UPInt i = 0; i < InstructionTimesArray.GetSize(); ++i)
    {
        InstructionTimesArray[i].Offset = str.ReadUInt32();
        InstructionTimesArray[i].Time = str.ReadUInt64();
    }
}

// Serialization
void MovieInstructionStats::ScriptBufferStats::Write(File& str, UInt32 version) const
{
    SF_UNUSED(version);
    str.WriteUInt32(SwdHandle);
    str.WriteUInt32(BufferOffset);
    str.WriteUInt32(BufferLength);
    str.WriteUInt32(static_cast<UInt32>(InstructionTimesArray.GetSize()));
    for (UPInt i = 0; i < InstructionTimesArray.GetSize(); ++i)
    {
        str.WriteUInt32(InstructionTimesArray[i].Offset);
        str.WriteUInt64(InstructionTimesArray[i].Time);
    }
}

///////////////////////////////////////////////////////////////

void FuncTreeItem::GetAllFunctions(HashSet<UInt64>* functionIds) const
{
    functionIds->Set(FunctionId);
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        Children[i]->GetAllFunctions(functionIds);
    }
}

const FuncTreeItem* FuncTreeItem::GetTreeItem(UInt32 treeItemId) const
{
    if (treeItemId == TreeItemId)
    {
        return this;
    }
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        const FuncTreeItem* childItem = Children[i]->GetTreeItem(treeItemId);
        if (childItem != NULL)
        {
            return childItem;
        }
    }
    return NULL;
}

class MaxIdVisitor
{
public:
    MaxIdVisitor() : MaxId(0) { }
    void operator()(FuncTreeItem* treeItem)
    {
        MaxId = Alg::Max(MaxId, treeItem->TreeItemId);
    }
    UInt32 MaxId;
};

class NumItemsVisitor
{
public:
    NumItemsVisitor() : NumItems(0) { }
    void operator()(FuncTreeItem* treeItem)
    {
        SF_UNUSED(treeItem);
        ++NumItems;
    }
    int NumItems;
};

class OffsetIdVisitor
{
public:
    OffsetIdVisitor(UInt32 offsetId) : OffsetId(offsetId) { }
    void operator()(FuncTreeItem* treeItem)
    {
        treeItem->TreeItemId += OffsetId;
    }
private:
    UInt32 OffsetId;
};

void  FuncTreeItem::ResetTreeIds(FuncTreeItem* other)
{
    MaxIdVisitor maxIdVisitor;
    other->Visit(maxIdVisitor);
    OffsetIdVisitor offsetVisitor(maxIdVisitor.MaxId);

    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        Children[i]->Visit(offsetVisitor);
    }
}

int FuncTreeItem::GetNumItems()
{
    NumItemsVisitor visitor;
    Visit(visitor);
    return visitor.NumItems;
}

void FuncTreeItem::Read(File& str, UInt32 version)
{
    FunctionId = str.ReadUInt64();
    BeginTime = str.ReadUInt64();
    EndTime = str.ReadUInt64();
    TreeItemId = str.ReadUInt32();
    Children.Resize(str.ReadUInt32());
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        Children[i] = *SF_HEAP_AUTO_NEW(this) FuncTreeItem();
        Children[i]->Read(str, version);
    }
}

void FuncTreeItem::Write(File& str, UInt32 version) const
{
    str.WriteUInt64(FunctionId);
    str.WriteUInt64(BeginTime);
    str.WriteUInt64(EndTime);
    str.WriteUInt32(TreeItemId);
    str.WriteUInt32(static_cast<UInt32>(Children.GetSize()));
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        Children[i]->Write(str, version);
    }
}

///////////////////////////////////////////////////////////////

MovieFunctionStats& MovieFunctionStats::operator/=(unsigned numFrames)
{
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        FunctionTimings[i].TimesCalled /= numFrames;
        FunctionTimings[i].TotalTime /= numFrames;        
    }
    return *this;
}

MovieFunctionStats& MovieFunctionStats::operator*=(unsigned num)
{
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        FunctionTimings[i].TimesCalled *= num;
        FunctionTimings[i].TotalTime *= num;        
    }
    return *this;
}

void MovieFunctionStats::Merge(const MovieFunctionStats& other)
{
    for (UPInt i = 0; i < other.FunctionTimings.GetSize(); ++i)
    {
        bool merged = false;
        for (UPInt j = 0; j < FunctionTimings.GetSize(); ++j)
        {
            if (FunctionTimings[j].FunctionId == other.FunctionTimings[i].FunctionId
                && FunctionTimings[j].CallerId == other.FunctionTimings[i].CallerId)
            {
                FunctionTimings[j].TimesCalled += other.FunctionTimings[i].TimesCalled;
                FunctionTimings[j].TotalTime += other.FunctionTimings[i].TotalTime;
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            FunctionTimings.PushBack(other.FunctionTimings[i]);
        }
    }

    FunctionDescMap::ConstIterator it;
    for (it = other.FunctionInfo.Begin(); it != other.FunctionInfo.End(); ++it)
    {
        FunctionInfo.Set(it->First, it->Second);
    }
}


// Serialization
void MovieFunctionStats::Read(File& str, UInt32 version)
{
    SF_UNUSED(version);
    UInt32 size = str.ReadUInt32();
    FunctionTimings.Resize(size);
    for (UInt32 i = 0; i < size; i++)
    {
        FunctionTimings[i].FunctionId = str.ReadUInt64();
        FunctionTimings[i].CallerId = str.ReadUInt64();
        FunctionTimings[i].TimesCalled = str.ReadUInt32();
        FunctionTimings[i].TotalTime = str.ReadUInt64();
    }
    
    size = str.ReadUInt32();
    for (UInt32 i = 0; i < size; i++)
    {
        UInt64 iKey = str.ReadUInt64();
        FunctionDesc* pDesc = SF_HEAP_AUTO_NEW(this) FunctionDesc();
        readString(str, &pDesc->Name);
        pDesc->Length = str.ReadUInt32();
        if (version >= 9)
        {
            pDesc->FileId = str.ReadUInt64();
            pDesc->FileLine = str.ReadUInt32();
            if (version >= 13)
            {
                pDesc->ASVersion = str.ReadUInt32();
            }
        }
        FunctionInfo.Set(iKey, *pDesc);
    }
}

// Serialization
void MovieFunctionStats::Write(File& str, UInt32 version) const
{
    SF_UNUSED(version);
    str.WriteUInt32(static_cast<UInt32>(FunctionTimings.GetSize()));
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        str.WriteUInt64(FunctionTimings[i].FunctionId);
        str.WriteUInt64(FunctionTimings[i].CallerId);
        str.WriteUInt32(FunctionTimings[i].TimesCalled);
        str.WriteUInt64(FunctionTimings[i].TotalTime);
    }

    str.WriteUInt32(static_cast<UInt32>(FunctionInfo.GetSize()));
    FunctionDescMap::ConstIterator descIt;
    for (descIt = FunctionInfo.Begin(); descIt != FunctionInfo.End(); ++descIt)
    {
        str.WriteUInt64(descIt->First);
        writeString(str, descIt->Second->Name);
        str.WriteUInt32(descIt->Second->Length);
        if (version >= 9)
        {
            str.WriteUInt64(descIt->Second->FileId);
            str.WriteUInt32(descIt->Second->FileLine);
            if (version >= 13)
            {
                str.WriteUInt32(descIt->Second->ASVersion);
            }
        }
    }
}

// Debug message for visualizing the call graph
void MovieFunctionStats::DebugPrint() const
{
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        String entry;
        String name;
        FunctionDescMap::ConstIterator descIt;
        descIt = FunctionInfo.Find(FunctionTimings[i].FunctionId);
        if (descIt != FunctionInfo.End())
        {
            name = descIt->Second->Name;
        }
        String parentName;
        descIt = FunctionInfo.Find(FunctionTimings[i].CallerId);
        if (descIt != FunctionInfo.End())
        {
            parentName = descIt->Second->Name;
        }
        Format(entry, "{0} ({1}) from {2} ({3}): {4} times\n", name.ToCStr(), FunctionTimings[i].FunctionId, parentName.ToCStr(), FunctionTimings[i].CallerId, FunctionTimings[i].TimesCalled);
        SF_DEBUG_MESSAGE1(1, "%s", entry.ToCStr());
    }
    SF_DEBUG_MESSAGE(1, "------------------------------");
}

bool funcStatsLess(const MovieFunctionStats::FuncStats& lhs, const MovieFunctionStats::FuncStats& rhs)
{
    return lhs.TotalTime > rhs.TotalTime;
}

void MovieFunctionStats::Print(Log* log) const
{
    Hash<UInt64, FuncStats> funcMap;
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        Hash<UInt64, FuncStats>::Iterator it = funcMap.Find(FunctionTimings[i].FunctionId);
        if (it != funcMap.End())
        {
            ++it->Second.TimesCalled;
            it->Second.TotalTime += FunctionTimings[i].TotalTime;
        }
        else
        {
            funcMap.Add(FunctionTimings[i].FunctionId, FunctionTimings[i]);
        }
    }

    Array<FuncStats> sortedList;
    for (Hash<UInt64, FuncStats>::Iterator it = funcMap.Begin(); it != funcMap.End(); ++it)
    {
        FuncStats newItem;
        newItem.FunctionId = it->First;
        newItem.CallerId = it->Second.CallerId;
        newItem.TotalTime = it->Second.TotalTime;
        newItem.TimesCalled = it->Second.TimesCalled;
        sortedList.PushBack(newItem);
    }
    Alg::QuickSort(sortedList, funcStatsLess);

    for (UPInt i = 0; i < sortedList.GetSize(); ++i)
    {
        String entry;
        String name;
        FunctionDescMap::ConstIterator descIt;
        descIt = FunctionInfo.Find(sortedList[i].FunctionId);
        if (descIt != FunctionInfo.End())
        {
            name = descIt->Second->Name;
        }
        Format(entry, "{0}: {1} ms ({2} times)\n", name.ToCStr(), sortedList[i].TotalTime / 1000, sortedList[i].TimesCalled);
        log->LogMessage("%s", entry.ToCStr());
    }
}



///////////////////////////////////////////////////////////////

bool MovieFunctionTreeStats::IsEmpty() const
{
    return FunctionRoots.GetSize() == 0;
}

///////////////////////////////////////////////////////////////

void MovieFunctionTreeStats::Merge(const MovieFunctionTreeStats& other)
{
    FunctionRoots.Append(other.FunctionRoots);
    for (UPInt i = 1; i < FunctionRoots.GetSize(); ++i)
    {
        FunctionRoots[i]->ResetTreeIds(FunctionRoots[i - 1]);
    }

    FunctionDescMap::ConstIterator it;
    for (it = other.FunctionInfo.Begin(); it != other.FunctionInfo.End(); ++it)
    {
        FunctionInfo.Set(it->First, it->Second);
    }
}


// Serialization
void MovieFunctionTreeStats::Read(File& str, UInt32 version)
{
    SF_UNUSED(version);

    readString(str, &ViewName);

    FunctionRoots.Resize(str.ReadUInt32());
    for (UInt32 i = 0; i < FunctionRoots.GetSize(); ++i)
    {
        FunctionRoots[i] = *SF_HEAP_AUTO_NEW(this) FuncTreeItem();
        FunctionRoots[i]->Read(str, version);
    }

    UInt32 size = str.ReadUInt32();
    for (UInt32 i = 0; i < size; i++)
    {
        UInt64 iKey = str.ReadUInt64();
        FunctionDesc* pDesc = SF_HEAP_AUTO_NEW(this) FunctionDesc();
        readString(str, &pDesc->Name);
        pDesc->Length = str.ReadUInt32();
        pDesc->FileId = str.ReadUInt64();
        pDesc->FileLine = str.ReadUInt32();
        pDesc->ASVersion = str.ReadUInt32();
        FunctionInfo.Set(iKey, *pDesc);
    }
}

// Serialization
void MovieFunctionTreeStats::Write(File& str, UInt32 version) const
{
    SF_UNUSED(version);

    writeString(str, ViewName);

    str.WriteUInt32(static_cast<UInt32>(FunctionRoots.GetSize()));
    for (UPInt i = 0; i < FunctionRoots.GetSize(); ++i)
    {
        FunctionRoots[i]->Write(str, version);
    }

    str.WriteUInt32(static_cast<UInt32>(FunctionInfo.GetSize()));
    FunctionDescMap::ConstIterator descIt;
    for (descIt = FunctionInfo.Begin(); descIt != FunctionInfo.End(); ++descIt)
    {
        str.WriteUInt64(descIt->First);
        writeString(str, descIt->Second->Name);
        str.WriteUInt32(descIt->Second->Length);
        str.WriteUInt64(descIt->Second->FileId);
        str.WriteUInt32(descIt->Second->FileLine);
        str.WriteUInt32(descIt->Second->ASVersion);
    }
}

struct FunctionTreeVisitor
{
    FunctionTreeVisitor(bool includeActionscipt) : IncludeActionScript(includeActionscipt) { }
    void operator()(const FuncTreeItem* root)
    {
        if (IncludeActionScript || (root->FunctionId >> 32) == GFx::AMP::NativeCodeSwdHandle)
        {
            Hash<UInt64, MovieFunctionStats::FuncStats>::Iterator it = FuncMap.Find(root->FunctionId);
            if (it != FuncMap.End())
            {
                ++it->Second.TimesCalled;
                it->Second.TotalTime += root->EndTime - root->BeginTime;
            }
            else
            {
                MovieFunctionStats::FuncStats funcStats;
                funcStats.FunctionId = root->FunctionId;
                funcStats.CallerId = 0;
                funcStats.TimesCalled = 1;
                funcStats.TotalTime = root->EndTime - root->BeginTime;
                FuncMap.Add(root->FunctionId, funcStats);
            }
        }
    }
    Hash<UInt64, MovieFunctionStats::FuncStats> FuncMap;
    bool IncludeActionScript;
};

MovieFunctionStats* MovieFunctionTreeStats::Accumulate(bool includeActionscipt) const
{
    MovieFunctionStats* accumulatedStats = SF_HEAP_AUTO_NEW(this) MovieFunctionStats();
    accumulatedStats->FunctionInfo = FunctionInfo;

    FunctionTreeVisitor treeVisitor(includeActionscipt);
    for (UPInt i = 0; i < FunctionRoots.GetSize(); ++i)
    {
        FunctionRoots[i]->Visit(treeVisitor);
    }

    for (Hash<UInt64, MovieFunctionStats::FuncStats>::Iterator it = treeVisitor.FuncMap.Begin(); it != treeVisitor.FuncMap.End(); ++it)
    {
        accumulatedStats->FunctionTimings.PushBack(it->Second);
    }

    return accumulatedStats;
}


void MovieFunctionTreeStats::Print(Log* log) const
{
    Ptr<MovieFunctionStats> accumulatedStats = *Accumulate(true);

    for (UPInt i = 0; i < accumulatedStats->FunctionTimings.GetSize(); ++i)
    {
        String entry;
        String name;
        FunctionDescMap::ConstIterator descIt;
        descIt = FunctionInfo.Find(accumulatedStats->FunctionTimings[i].FunctionId);
        if (descIt != FunctionInfo.End())
        {
            name = descIt->Second->Name;
        }
        if (!name.IsEmpty())
        {
            Format(entry, "{0}: {1} ms ({2} times)\n", name.ToCStr(), accumulatedStats->FunctionTimings[i].TotalTime / 1000, accumulatedStats->FunctionTimings[i].TimesCalled);
            log->LogMessage("%s", entry.ToCStr());
        }
    }
}



///////////////////////////////////////////////////////////////

MovieSourceLineStats& MovieSourceLineStats::operator/=(unsigned numFrames)
{
    for (UPInt i = 0; i < SourceLineTimings.GetSize(); ++i)
    {
        SourceLineTimings[i].TotalTime /= numFrames;        
    }
    return *this;
}

MovieSourceLineStats& MovieSourceLineStats::operator*=(unsigned num)
{
    for (UPInt i = 0; i < SourceLineTimings.GetSize(); ++i)
    {
        SourceLineTimings[i].TotalTime *= num;        
    }
    return *this;
}

void MovieSourceLineStats::Merge(const MovieSourceLineStats& other)
{
    for (UPInt i = 0; i < other.SourceLineTimings.GetSize(); ++i)
    {
        bool merged = false;
        for (UPInt j = 0; j < SourceLineTimings.GetSize(); ++j)
        {
            if (SourceLineTimings[j].FileId == other.SourceLineTimings[i].FileId
                && SourceLineTimings[j].LineNumber == other.SourceLineTimings[i].LineNumber)
            {
                SourceLineTimings[j].TotalTime += other.SourceLineTimings[i].TotalTime;
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            SourceLineTimings.PushBack(other.SourceLineTimings[i]);
        }
    }

    SourceFileDescMap::ConstIterator it;
    for (it = other.SourceFileInfo.Begin(); it != other.SourceFileInfo.End(); ++it)
    {
        SourceFileInfo.Set(it->First, it->Second);
    }
}


// Serialization
void MovieSourceLineStats::Read(File& str, UInt32 version)
{
    if (version >= 9)
    {
        UInt32 size = str.ReadUInt32();
        SourceLineTimings.Resize(size);
        for (UInt32 i = 0; i < size; i++)
        {
            SourceLineTimings[i].FileId = str.ReadUInt64();
            SourceLineTimings[i].LineNumber = str.ReadUInt32();
            SourceLineTimings[i].TotalTime = str.ReadUInt64();
        }

        size = str.ReadUInt32();
        for (UInt32 i = 0; i < size; i++)
        {
            UInt64 iKey = str.ReadUInt64();
            String desc;
            readString(str, &desc);
            SourceFileInfo.Set(iKey, desc);
        }
    }
}

// Serialization
void MovieSourceLineStats::Write(File& str, UInt32 version) const
{
    if (version >= 9)
    {
        str.WriteUInt32(static_cast<UInt32>(SourceLineTimings.GetSize()));
        for (UPInt i = 0; i < SourceLineTimings.GetSize(); ++i)
        {
            str.WriteUInt64(SourceLineTimings[i].FileId);
            str.WriteUInt32(SourceLineTimings[i].LineNumber);
            str.WriteUInt64(SourceLineTimings[i].TotalTime);
        }

        str.WriteUInt32(static_cast<UInt32>(SourceFileInfo.GetSize()));
        SourceFileDescMap::ConstIterator descIt;
        for (descIt = SourceFileInfo.Begin(); descIt != SourceFileInfo.End(); ++descIt)
        {
            str.WriteUInt64(descIt->First);
            writeString(str, descIt->Second);
        }
    }
}

///////////////////////////////////////

ServerState::ServerState() :
    StateFlags(0),
    ProfileLevel(0),
    CurveTolerance(0.0f), 
    CurveToleranceMin(0.0f),
    CurveToleranceMax(0.0f),
    CurveToleranceStep(0.0f),
    CurrentFileId(0),
    CurrentLineNumber(0),
    Platform(0)
{
}

ServerState& ServerState::operator=(const ServerState& rhs)
{
    StateFlags = rhs.StateFlags;
    ProfileLevel = rhs.ProfileLevel;
    ConnectedApp = rhs.ConnectedApp;
    ConnectedFile = rhs.ConnectedFile;
    AaMode = rhs.AaMode;
    StrokeType = rhs.StrokeType;
    CurrentLocale = rhs.CurrentLocale;
    Locales.Resize(rhs.Locales.GetSize());
    for (UPInt i = 0; i < Locales.GetSize(); ++i)
    {
        Locales[i] = rhs.Locales[i];
    }
    CurveTolerance = rhs.CurveTolerance;
    CurveToleranceMin = rhs.CurveToleranceMin;
    CurveToleranceMax = rhs.CurveToleranceMax;
    CurveToleranceStep = rhs.CurveToleranceStep;
    CurrentFileId = rhs.CurrentFileId;
    CurrentLineNumber = rhs.CurrentLineNumber;
    Platform = rhs.Platform;
    return *this;
}

bool ServerState::operator!=(const ServerState& rhs) const
{
    if (StateFlags != rhs.StateFlags) return true;
    if (ProfileLevel != rhs.ProfileLevel) return true;
    if (ConnectedApp != rhs.ConnectedApp) return true;
    if (ConnectedFile != rhs.ConnectedFile) return true;
    if (AaMode != rhs.AaMode) return true;
    if (StrokeType != rhs.StrokeType) return true;
    if (CurrentLocale != rhs.CurrentLocale) return true;
    if (Locales.GetSize() != rhs.Locales.GetSize()) return true;
    for (UPInt i = 0; i < Locales.GetSize(); ++i)
    {
        if (Locales[i] != rhs.Locales[i]) return true;
    }
    if (Alg::Abs(CurveTolerance - rhs.CurveTolerance) > 0.0001) return true;
    if (Alg::Abs(CurveToleranceMin - rhs.CurveToleranceMin) > 0.0001) return true;
    if (Alg::Abs(CurveToleranceMax - rhs.CurveToleranceMax) > 0.0001) return true;
    if (Alg::Abs(CurveToleranceStep - rhs.CurveToleranceStep) > 0.0001) return true;
    if (CurrentFileId != rhs.CurrentFileId) return true;
    if (CurrentLineNumber != rhs.CurrentLineNumber) return true;
    if (Platform != rhs.Platform) return true;

    return false;
}


// Serialization
void ServerState::Read(File& str, UInt32 version)
{
    StateFlags = str.ReadUInt32();
    if (version >= 20)
    {
        ProfileLevel = str.ReadSInt32();
    }
    readString(str, &ConnectedApp);
    if (version >= 5)
    {
        readString(str, &ConnectedFile);
    }
    readString(str, &AaMode);
    readString(str, &StrokeType);
    readString(str, &CurrentLocale);
    UInt32 size = str.ReadUInt32();
    Locales.Resize(size);
    for (UInt32 i = 0; i < size; ++i)
    {
        readString(str, &Locales[i]);
    }
    CurveTolerance = str.ReadFloat();
    CurveToleranceMin = str.ReadFloat();
    CurveToleranceMax = str.ReadFloat();
    CurveToleranceStep = str.ReadFloat();
    if (version >= 10)
    {
        CurrentFileId = str.ReadUInt64();
        CurrentLineNumber = str.ReadUInt32();
    }
    if (version >= 36)
    {
        Platform = str.ReadUInt32();
    }
}

// Serialization
void ServerState::Write(File& str, UInt32 version) const
{
    str.WriteUInt32(StateFlags);
    if (version >= 20)
    {
        str.WriteSInt32(ProfileLevel);
    }
    writeString(str, ConnectedApp);
    if (version >= 5)
    {
        writeString(str, ConnectedFile);
    }
    writeString(str, AaMode);
    writeString(str, StrokeType);
    writeString(str, CurrentLocale);
    str.WriteUInt32(static_cast<UInt32>(Locales.GetSize()));
    for (UPInt i = 0; i < Locales.GetSize(); ++i)
    {
        writeString(str, Locales[i]);
    }
    str.WriteFloat(CurveTolerance);
    str.WriteFloat(CurveToleranceMin);
    str.WriteFloat(CurveToleranceMax);
    str.WriteFloat(CurveToleranceStep);
    if (version >= 10)
    {
        str.WriteUInt64(CurrentFileId);
        str.WriteUInt32(CurrentLineNumber);
    }
    if (version >= 36)
    {
        str.WriteUInt32(Platform);
    }
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif
