/**************************************************************************

PublicHeader:   AMP
Filename    :   Amp_ProfileFrame.h
Content     :   Profile information sent to AMP
Created     :   February 2010
Authors     :   Alex Mantzaris, Ben Mowery

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef GFX_AMP_PROFILE_FRAME
#define GFX_AMP_PROFILE_FRAME

#include "GFxConfig.h"
#ifdef SF_ENABLE_STATS

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_Log.h"
#include "Kernel/SF_MemItem.h"
#include "GFx/GFx_PlayerStats.h"


namespace Scaleform {
class File;
namespace GFx {
namespace AMP {

// This struct contains the stats for each ActionScript instruction for a given movie
//
// The stats are organized in an array with one element per ActionScript buffer
// The buffer stats include execution times for each instruction, but also the buffer length,
// which is used by the client to map source code lines to execution time
//
// Each buffer is uniquely identified by a SWD handle (unique to each SWF file),
// and the offset into the SWF in bytes.
// 
struct MovieInstructionStats : public RefCountBase<MovieInstructionStats, StatAmp_ProfileFrame>
{
    struct InstructionTimePair
    {
        UInt32      Offset;  // Byte offset from beginning of buffer
        UInt64      Time;    // Microseconds
    };

    // ActionScript buffer data collected during execution
    struct ScriptBufferStats : RefCountBase<ScriptBufferStats, StatAmp_ProfileFrame>
    {
        UInt32                          SwdHandle;      // uniquely defines the SWF
        UInt32                          BufferOffset;   // Byte offset from beginning of SWF
        UInt32                          BufferLength;
        ArrayLH<InstructionTimePair>    InstructionTimesArray;

        // Serialization
        void                            Read(File& str, UInt32 version);
        void                            Write(File& str, UInt32 version) const;
    };

    // Array of pointers, so all data can be allocated on the same heap
    ArrayLH< Ptr<ScriptBufferStats> > BufferStatsArray;

    // operators
    MovieInstructionStats& operator/=(unsigned numFrames);
    MovieInstructionStats& operator*=(unsigned num);
    void        Merge(const MovieInstructionStats& other);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;
};


// Struct for mapping a byte offset to a function name and length
struct FunctionDesc : public RefCountBase<FunctionDesc, StatAmp_ProfileFrame>
{
    StringLH    Name;
    unsigned    Length;
    UInt64      FileId;
    UInt32      FileLine;
    UInt32      ASVersion;
};
typedef HashLH<UInt64, Ptr<FunctionDesc> > FunctionDescMap;

typedef HashLH<UInt64, String> SourceFileDescMap;

// Struct for a function call graph item
class FuncTreeItem : public RefCountBase<FuncTreeItem, Stat_Default_Mem>
{
public:
    UInt64                          FunctionId;     // SWF handle and offset
    UInt64                          BeginTime;      // microseconds
    UInt64                          EndTime;        // microseconds
    UInt32                          TreeItemId;
    ArrayLH< Ptr<FuncTreeItem> >    Children;

    void                GetAllFunctions(HashSet<UInt64>* functionIds) const;
    const FuncTreeItem* GetTreeItem(UInt32 treeItemId) const;
    void                ResetTreeIds(FuncTreeItem* other);
    int                 GetNumItems();

    template<typename T>
    void Visit(T& visitor)
    {
        visitor(this);
        for (UPInt i = 0; i < Children.GetSize(); ++i)
        {
            Children[i]->Visit(visitor);
        }
    }

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;
};

// This struct contains the execution times and numbers for each ActionScript function 
// for a given movie
//
// The stats are organized in an array with one element per ActionScript function
// and a hash map from function ID to function name
//
struct MovieFunctionStats;

struct MovieFunctionTreeStats : public RefCountBase<MovieFunctionTreeStats, Stat_Default_Mem>
{
    StringLH                            ViewName;
    ArrayLH< Ptr<FuncTreeItem> >        FunctionRoots;
    FunctionDescMap                     FunctionInfo;

    bool        IsEmpty() const;

    // operators
    void        Merge(const MovieFunctionTreeStats& other);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;

    MovieFunctionStats* Accumulate(bool includeActionscipt) const;
    void                Print(Log* log) const;
};



// This struct contains the execution times and numbers for each ActionScript function 
// for a given movie
//
// The stats are organized in an array with one element per ActionScript function
// and a hash map from function ID to function name
//
struct MovieFunctionStats : public RefCountBase<MovieFunctionStats, StatAmp_ProfileFrame>
{
    struct FuncStats
    {
        UInt64      FunctionId;     // SWF handle and offset
        UInt64      CallerId;       // Caller SWF handle and offset
        UInt32      TimesCalled;
        UInt64      TotalTime;      // microseconds
    };

    ArrayLH<FuncStats>              FunctionTimings;
    FunctionDescMap                 FunctionInfo;

    // operators
    MovieFunctionStats& operator/=(unsigned numFrames);
    MovieFunctionStats& operator*=(unsigned numFrames);
    void        Merge(const MovieFunctionStats& other);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;

    // For debugging
    void        DebugPrint() const;
    void        Print(Log* log) const;
};

// This struct contains the execution times and numbers for each ActionScript line 
// for a given movie
//
// The stats are organized in an array with one element per ActionScript line
// and a hash map from file ID to filename
//
struct MovieSourceLineStats : public RefCountBase<MovieSourceLineStats, StatAmp_ProfileFrame>
{
    struct SourceStats
    {
        UInt64      FileId;         // SWF handle and file ID
        UInt32      LineNumber;
        UInt64      TotalTime;      // microseconds
    };

    ArrayLH<SourceStats>                SourceLineTimings;
    SourceFileDescMap                   SourceFileInfo;

    // operators
    MovieSourceLineStats& operator/=(unsigned numFrames);
    MovieSourceLineStats& operator*=(unsigned numFrames);
    void        Merge(const MovieSourceLineStats& other);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;
};

// This struct contains all the profile statistics for a view
// That includes function statistics and instruction statistics
// It is part of the profile sent to AMP every frame
struct MovieProfile : public RefCountBase<MovieProfile, StatAmp_ProfileFrame>
{
    struct MarkerInfo : public RefCountBase<FunctionDesc, StatAmp_ProfileFrame>
    {
        StringLH        Name;
        UInt32          Number;
    };

    UInt32              ViewHandle;
    UInt32              MinFrame;
    UInt32              MaxFrame;
    StringLH            ViewName;
    UInt32              Version;
    float               Width;
    float               Height;
    float               FrameRate;
    UInt32              FrameCount;
    ArrayLH< Ptr<MarkerInfo> >   Markers;

    Ptr<MovieInstructionStats>  InstructionStats;
    Ptr<MovieFunctionStats>     FunctionStats;
    Ptr<MovieSourceLineStats>   SourceLineStats;
    Ptr<MovieFunctionTreeStats> FunctionTreeStats;

    // Initialization
    MovieProfile();
    
    // operators
    void            Merge(const MovieProfile& rhs);
    MovieProfile&   operator=(const MovieProfile& rhs);
    MovieProfile&   operator/=(unsigned numFrames);
    MovieProfile&   operator*=(unsigned num);

    // Serialization
    void            Read(File& str, UInt32 version);
    void            Write(File& str, UInt32 version) const;
    void            Print(Log* log) const;
};

// Image information
struct ImageInfo : public RefCountBase<ImageInfo, StatAmp_ProfileFrame>
{
    UInt32      Id;
    StringLH    Name;
    UInt32      Bytes;
    bool        External;
    UInt32      AtlasId;
    UInt32      AtlasTop;
    UInt32      AtlasBottom;
    UInt32      AtlasLeft;
    UInt32      AtlasRight;  

    ImageInfo() : Id(0), Bytes(0), External(false), 
        AtlasId(0), AtlasTop(0), AtlasBottom(0), AtlasLeft(0), AtlasRight(0) { }

    bool operator<(const ImageInfo& rhs) const
    {
        if (Bytes == rhs.Bytes)
        {
            return Id < rhs.Id;
        }
        return Bytes > rhs.Bytes;
    }

    // Serialization
    void            Read(File& str, UInt32 version);
    void            Write(File& str, UInt32 version) const;
};

// The data in this class consists of all the metrics reported by AMP
// and displayed by the client in various ways (graphs, tables, etc)
//
// The AMP server updates one such object every frame and sends it to the client
// as part of a MessageProfileFrame
// 
class ProfileFrame : public RefCountBase<ProfileFrame, StatAmp_ProfileFrame>
{
public:
    // Frame timestamp (microseconds)
    UInt64  TimeStamp;

    UInt32  FramesPerSecond;
    UInt32  ProfilingLevel;
    bool    DetailedMemReport;

    // CPU graph
    UInt32  AdvanceTime;
    UInt32      ActionTime;
    UInt32      TimelineTime;
    UInt32      InputTime;
    UInt32          MouseTime;
    UInt32      GcCollectTime;
    UInt32          GcMarkInCycleTime;
    UInt32          GcScanInUseTime;
    UInt32          GcFreeGarbageTime;
    UInt32          GcFinalizeTime;
    UInt32          GcDelayedCleanupTime;
    UInt32  GetVariableTime;
    UInt32  SetVariableTime;
    UInt32  InvokeTime;
    UInt32  DisplayTime;
    UInt32      PresentTime;
    UInt32      FontThrashingTime;
    UInt32      GradientGenTime;
    UInt32      FontMissTime;
    UInt32  UserTime;
    UInt32  UserCalls;

    // Rendering graph
    UInt32  LineCount;
    UInt32  MaskCount;
    UInt32  FilterCount;
    UInt32  MeshCount;
    UInt32  TriangleCount;
    UInt32  DrawPrimitiveCount;
    UInt32  StrokeCount;
    UInt32  GradientFillCount;
    UInt32  MeshThrashing;
    UInt32  RasterizedGlyphCount;
    UInt32  FontTextureCount;
    UInt32  NumFontCacheTextureUpdates;
    UInt32  FontThrashing;
    UInt32  FontFill;
    UInt32  FontFail;
    UInt32  FontMisses;
    UInt32  FontTotalArea;
    UInt32  FontUsedArea;
    UInt32  FontOptRead;

    // Memory graph
    UInt32  TotalMemory;
    UInt32  ImageMemory;
    UInt32  ImageGraphicsMemory;
    UInt32  MovieDataMemory;
    UInt32  FontMemory;
    UInt32  MovieViewMemory;
    UInt32  AS3Memory;
    UInt32  MeshCacheMemory;
    UInt32  MeshCacheGraphicsMemory;
    UInt32  MeshCacheUnusedMemory;
    UInt32  MeshCacheGraphicsUnusedMemory;
    UInt32  FontCacheMemory;
    UInt32  VideoMemory;
    UInt32  SoundMemory;
    UInt32  OtherMemory;

    // Garbage collector
    UInt32  GcRootsNumber;
    UInt32  GcFreedRootsNumber;

    ArrayLH< Ptr<MovieProfile> >    MovieStats;
    Ptr<MovieFunctionStats>         DisplayStats;
    Ptr<MovieFunctionTreeStats>     DisplayFunctionStats;
    ArrayLH<UInt32>                 SwdHandles;
    ArrayLH<UInt64>                 FileHandles;

    Ptr<MemItem>                    MemoryByStatId;
    Ptr<MemItem>                    Images;
    Ptr<MemItem>                    Fonts;

    ArrayLH< Ptr<ImageInfo> >       ImageList;

    // Initialization
    ProfileFrame();

    // Operators for multiple frame reporting
    ProfileFrame& operator+=(const ProfileFrame& rhs);
    ProfileFrame& operator/=(unsigned numFrames);
    ProfileFrame& operator*=(unsigned num);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;

    // Debug
    void        Print(Log* log) const;
    int         GetNumFunctionTreeStats() const;

    void        AccumulateTreeFunctionStats(bool includeActionScript);
};

// This struct holds the current state of AMP
// For feedback to the AMP client
//
class ServerState : public RefCountBase<ServerState, StatAmp_ProfileFrame>
{
public:
    UInt32                  StateFlags;
    SInt32                  ProfileLevel;
    StringLH                ConnectedApp;
    StringLH                ConnectedFile;
    StringLH                AaMode;
    StringLH                StrokeType;
    StringLH                CurrentLocale;
    ArrayLH<String>         Locales;
    float                   CurveTolerance;
    float                   CurveToleranceMin;
    float                   CurveToleranceMax;
    float                   CurveToleranceStep;
    UInt64                  CurrentFileId;
    UInt32                  CurrentLineNumber;
    UInt32                  Platform;

    ServerState();
    ServerState& operator=(const ServerState& rhs);
    bool operator!=(const ServerState& rhs) const;

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif

#endif