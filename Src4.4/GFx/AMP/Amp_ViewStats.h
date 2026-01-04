/**************************************************************************

PublicHeader:   AMP
Filename    :   Amp_ViewStats.h
Content     :   Performance statistics for a MovieView
Created     :   February, 2010
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef GFX_AMP_SCRIPT_PROFILE_STATS_H
#define GFX_AMP_SCRIPT_PROFILE_STATS_H

#include "GFxConfig.h"
#ifdef SF_ENABLE_STATS

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_Random.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_StringHash.h"
#include "Kernel/SF_AmpInterface.h"
#include "Amp_ProfileFrame.h"
#include "GFx/GFx_PlayerStats.h"

namespace Scaleform {
    class String;

namespace GFx {
    class MovieDef;

namespace AMP {

// For mixing ActionScript and C++ call stack 
enum AmpHandleType
{ 
    NativeCodeSwdHandle = 1,
};



class ViewStats : public AmpStats
{
public:

    // Struct for holding function execution metrics
    struct AmpFunctionStats
    {
		UInt32 TimesCalled;
		UInt64 TotalTime;    // Microseconds
    };

    // This struct holds timing information for one GASActionBuffer
    // The times are stored in an array of size equal to the size of the buffer for fast access
    // Many entries will be zero, as instructions are typically several bytes long
    struct BufferInstructionTimes : public RefCountBase<BufferInstructionTimes, StatAmp_InstrBuffer>
    {
        BufferInstructionTimes(UInt32 size) : Times(size)
        {
            memset(&Times[0], 0, size * sizeof(Times[0])); // 
        }
        ArrayLH<UInt64> Times;
    };

    // Function executions are tracked per function called for each caller
    // This means a timings map will have a caller-callee pair as a key
    // ParentChildFunctionPair is this key
    struct ParentChildFunctionPair
    {
        // Make sure the member variables stay with no padding
        // Padding breaks the hash function that uses this struct as a key
        UInt64 CallerId;    // SWF handle and byte offset
        UInt64 FunctionId;  // SWF handle and byte offset
        bool operator==(const ParentChildFunctionPair& kRhs) const 
        { 
            return (CallerId == kRhs.CallerId && FunctionId == kRhs.FunctionId); 
        }
    };
    typedef HashLH<ParentChildFunctionPair, AmpFunctionStats> FunctionStatMap;

    // Struct for holding per-line timings
    struct FileLinePair
    {
        // Make sure the member variables stay with no padding
        // Padding breaks the hash function that uses this struct as a key
        UInt64 FileId;
        UInt32 LineNumber;
        bool operator==(const FileLinePair& rhs) const
        {
            return (FileId == rhs.FileId && LineNumber == rhs.LineNumber);
        }
    };
    typedef HashLH<FileLinePair, UInt64> SourceLineStatMap;

    // Constructor
    ViewStats();

    // virtual method overrides for native method profiling
    virtual void        NativePushCallstack(const char* functionName, AmpNativeFunctionId functionId, UInt64 funcTime);
    virtual void        NativePopCallstack(UInt64 time);

    // Methods for ActionScript profiling
    void                RegisterScriptFunction(UInt32 swdHandle, UInt32 swfOffset, const char* name, unsigned length, UInt32 asVersion, bool updateSource);
    void                RegisterSourceFile(UInt32 swdHandle, UInt32 index, const char* name);
    void                PushCallstack(UInt32 swdHandle, UInt32 swfOffset, UInt64 funcTime);
    void                PopCallstack(UInt32 swdHandle, UInt32 swfOffset, UInt64 funcTime);
    UInt32              GetActiveLine() const;
    void                SetActiveLine(UInt32 lineNumber);
    UInt64              GetActiveFile() const;
    void                SetActiveFile(UInt64 fileId);
    ArrayLH<UInt64>&    LockBufferInstructionTimes(UInt32 swdHandle, UInt32 swfBufferOffset, unsigned length);
    void                ReleaseBufferInstructionTimes();
    void                RecordSourceLineTime(UInt64 lineTime);
    
    // Methods for AMP stats collecting
    void                CollectAmpInstructionStats(MovieProfile* movieProfile);
    void                ClearAmpInstructionStats();
    void                CollectAmpFunctionStats(MovieProfile* movieProfile);
    void                ClearAmpFunctionStats();
    void                CollectAmpSourceLineStats(MovieProfile* movieProfile);
    void                ClearAmpSourceLineStats();
    void                CollectTimingStats(ProfileFrame* frameProfile);
    void                CollectMarkers(MovieProfile* movieProfile);
    void                ClearMarkers();
    void                CollectGcStats(ProfileFrame* frameProfile);
    void                ClearGcStats();
    void                CollectMemoryStats(ProfileFrame* frameProfile);
    void                GetStats(StatBag* bag, bool reset);

    void        UpdateStats(UInt64 functionId, UInt32 functionTime, UInt32 functionCalls, ProfileFrame* frameProfile);

    // View information
    void            SetName(const char* pcName);
    const String&   GetName() const;
    UInt32          GetViewHandle() const;
    void            SetMovieDef(MovieDef* movieDef);
    UInt32          GetVersion() const;
    float           GetWidth() const;
    float           GetHeight() const;
    float           GetFrameRate() const;
    UInt32          GetFrameCount() const;

    // Flash frame for this view
    UInt32      GetCurrentFrame() const;
    void        SetCurrentFrame(UInt32 frame);

    // For instruction profiling with random sampling
    UInt64      GetInstructionTime(UInt32 samplePeriod);

    // Frame markers
    void        AddMarker(const char* markerType);

    // GC stats
    virtual void AddGcRoots(UInt32 numRoots);
    virtual void AddGcFreedRoots(UInt32 numFreedRoots);

    // Debugger
    void        DebugStep(int depth);
    void        DebugGo();
    void        DebugPause();
    void        DebugWait();
    bool        IsDebugPaused() const;

private:

    // ActionScript function profiling
    FunctionStatMap                                             FunctionTimingMap;
    FunctionDescMap                                             FunctionInfoMap;
    StringHashLH<UInt32>                                        NativeFunctionIdMap;
    UInt32                                                      NextNativeFunctionId;

    typedef ArrayConstPolicy<0, 4, true> NeverShrinkPolicy;
    struct CallInfo 
    {
        CallInfo(FuncTreeItem* functionInfo = NULL, UInt64 fileId = 0, UInt32 lineNumber = 0) 
            : FunctionInfo(functionInfo), FileId(fileId), LineNumber(lineNumber) {}
        Ptr<FuncTreeItem>   FunctionInfo;
        UInt64              FileId;
        UInt32              LineNumber;
    };
    ArrayLH<CallInfo, StatAmp_Callstack, NeverShrinkPolicy>             Callstack;
    ArrayLH<Ptr<FuncTreeItem> >                                         FunctionRoots;
    UInt32                                                              NextTreeItemId;
    int                                                                 CallstackDepthPause;
    UInt64                                                              ActiveFileId;
    UInt32                                                              ActiveLineNumber;
    mutable Lock                                                        ActiveLock;                                    

    // ActionScript instruction timing
    typedef HashLH<UInt64, Ptr<BufferInstructionTimes> >        InstructionTimingMap;
    InstructionTimingMap                                        InstructionTimingsMap;
    mutable Mutex                                               InstructionTimingMutex;

    // ActionScript source line timing
    SourceLineStatMap                                           SourceLineTimingsMap;
    SourceFileDescMap                                           SourceLineInfoMap;

    // View information
    mutable Lock    ViewLock;
    UInt32          ViewHandle;   // Unique GFxMovieRoot ID so that AMP can keep track of each view
    StringLH        ViewName;  
    unsigned        CurrentFrame;
    UInt32          Version;
    float           Width;
    float           Height;
    float           FrameRate;
    UInt32          FrameCount;

    // Instruction sampling
    Alg::Random::Generator  RandomGen;
    UInt32                  SkipSamples;
    UInt64                  LastTimer;

    // Markers
    StringHashLH<UInt32>    Markers;

    // GC stats
    UInt32                  RootsNumber;
    UInt32                  FreedRootsNumber;

    Scaleform::Event        DebugEvent;

    // Helper method for key generation
    static UInt64 MakeSwdOffsetKey(UInt32 swdHandle, UInt32 iOffset)
    {
        UInt64 iKey = swdHandle;
        iKey <<= 32;
        iKey += iOffset;
        return iKey;
    }

    static bool IsFunctionAggregation()
    {
#ifdef SF_AMP_SERVER
        return AmpServer::GetInstance().IsFunctionAggregation();
#else
        return false;
#endif
    }

    void RefreshActiveLine();
};

// This class keeps track of function execution time and call stack
// Time starts counting in constructor and stops in destructor
// Updates the view stats object with the results
class ScopeFunctionTimer
{
public:
    ScopeFunctionTimer(ViewStats* viewStats, UInt32 swdHandle, unsigned pc, AmpProfileLevel profileLevel) : 
          Stats(viewStats), 
          SwdHandle(swdHandle), 
          PC(pc)
      { 
          if (!AmpServer::GetInstance().IsProfiling()
              || AmpServer::GetInstance().GetProfileLevel() < profileLevel)
          {
              Stats = NULL;
          }
          if (Stats != NULL)
          {
              StartTicks = Timer::GetProfileTicks();
              Stats->PushCallstack(swdHandle, pc, StartTicks);
          }
          else
          {
              StartTicks = 0;
          }
      }
      ~ScopeFunctionTimer()
      {
          if (Stats != NULL)
          {
              Stats->PopCallstack(SwdHandle, PC, Timer::GetProfileTicks() - StartTicks);
          }
      }
      UInt64                GetStartTicks() const   { return StartTicks; }
private:
    UInt64              StartTicks;
    ViewStats*          Stats;
    UInt32              SwdHandle;
    UInt32              PC;
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif

#endif
