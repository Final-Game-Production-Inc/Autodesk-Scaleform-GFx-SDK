/**************************************************************************

Filename    :   Amp_ViewStats.cpp
Content     :   Performance statistics for a MovieView
Created     :   February, 2010
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Amp_ViewStats.h"
#ifdef SF_ENABLE_STATS
#include "Kernel/SF_HeapNew.h"
#include "GFx/GFx_Player.h"
#include "Amp_Server.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// Constructor
ViewStats::ViewStats() : 
    NextNativeFunctionId(static_cast<UInt32>(Amp_Num_Native_Function_Ids)),
    NextTreeItemId(0),
    CallstackDepthPause(-1),
    ActiveFileId(0),
    ActiveLineNumber(0),
    ViewHandle(0), 
    CurrentFrame(0), 
    Version(0),
    Width(0),
    Height(0),
    FrameRate(0),
    FrameCount(0),
    SkipSamples(0), 
    LastTimer(0),
    RootsNumber(0),
    FreedRootsNumber(0)
{
    // Generate unique handle so that view can be identified by AMP
    Lock::Locker locker(&ViewLock);
    static UInt32 nextHandle = 1;
    ViewHandle = nextHandle;
    ++nextHandle;
    DebugEvent.SetEvent();
    RegisterScriptFunction(NativeCodeSwdHandle, Amp_Native_Function_Id_Begin_ObjectInterface, "Object Interface", 0, 0, false);
    NativeFunctionIdMap.Add("Object Interface", Amp_Native_Function_Id_Begin_ObjectInterface);
}


void ViewStats::NativePushCallstack(const char* functionName, AmpNativeFunctionId functionId, UInt64 funcTime)
{
    Lock::Locker locker(&ViewLock);
    UInt32 functionHandle = static_cast<UInt32>(functionId);
    if (functionId == Amp_Native_Function_Id_Invalid)
    {
        StringHashLH<UInt32>::Iterator it = NativeFunctionIdMap.Find(functionName);
        if (it == NativeFunctionIdMap.End())
        {
            RegisterScriptFunction(NativeCodeSwdHandle, NextNativeFunctionId, functionName, 0, 0, false);
            NativeFunctionIdMap.Add(functionName, NextNativeFunctionId);
            it = NativeFunctionIdMap.Find(functionName);
            ++NextNativeFunctionId;
        }
        functionHandle = it->Second;
    }
    else
    {
        RegisterScriptFunction(NativeCodeSwdHandle, functionHandle, functionName, 0, 0, false);
    }
    PushCallstack(NativeCodeSwdHandle, functionHandle, funcTime);
}

void ViewStats::NativePopCallstack(UInt64 time)
{
    Lock::Locker locker(&ViewLock);
    if (!Callstack.IsEmpty())
    {
        UInt64 functionId = Callstack.Back().FunctionInfo->FunctionId;
        PopCallstack(static_cast<UInt32>(functionId >> 32), static_cast<UInt32>(functionId), time);
    }
}


// Registers a script function so that AMP can refer to it by name
void ViewStats::RegisterScriptFunction(UInt32 swdHandle, UInt32 swfOffset, const char* name, 
                                       unsigned byteCodeLength, UInt32 asVersion, bool updateSource)
{
    SF_ASSERT(swdHandle != 0);
    UInt64 iKey = MakeSwdOffsetKey(swdHandle, swfOffset);
    FunctionDescMap::Iterator it = FunctionInfoMap.Find(iKey);
    if (it == FunctionInfoMap.End())
    {
        // Register only if not done so already
        FunctionDesc* pDesc = SF_HEAP_AUTO_NEW(this) FunctionDesc();
        pDesc->Name = name;
        pDesc->Length = byteCodeLength;
        pDesc->ASVersion = asVersion;
        pDesc->FileId = updateSource ? GetActiveFile() : 0;
        pDesc->FileLine = updateSource ? GetActiveLine() : 0;

        FunctionInfoMap.Set(iKey, *pDesc);
    }
    else
    {
        // Make sure the duplicate registration refers to the same function
        FunctionDesc* pDesc = it->Second;
        SF_ASSERT(pDesc->Length == byteCodeLength);
        SF_ASSERT(byteCodeLength != 0 || SFstrcmp(name, pDesc->Name.ToCStr()) == 0);
        UInt32 newLine = GetActiveLine();
        if (updateSource && newLine != 0)
        {
            SF_ASSERT(pDesc->FileId == 0 || pDesc->FileId == GetActiveFile());
            pDesc->FileId = GetActiveFile();
            SF_ASSERT(pDesc->FileId != 0);

            if (pDesc->FileLine != 0)
            {
                pDesc->FileLine = Alg::Min(pDesc->FileLine, newLine);
            }
            else
            {
                pDesc->FileLine = newLine;
            }
        }
    }
}

// Registers a source file so that AMP can refer to it by name
void ViewStats::RegisterSourceFile(UInt32 swdHandle, UInt32 index, const char* name)
{
    SF_ASSERT(swdHandle != 0);
    UInt64 iKey = MakeSwdOffsetKey(swdHandle, index);
    SourceFileDescMap::Iterator it = SourceLineInfoMap.Find(iKey);
    if (it == SourceLineInfoMap.End())
    {
        // Register only if not done so already
        UPInt nameLength = SFstrlen(name);
        String cleanName;
        for (UPInt i = 0; i < nameLength; ++i)
        {
            if (name[i] == ';')
            {
                cleanName.AppendChar('\\');
            }
            else
            {
                cleanName.AppendChar(name[i]);
            }
        }
        SourceLineInfoMap.Set(iKey, cleanName);
        SF_AMP_CODE(AmpServer::GetInstance().AddSourceFile(iKey, cleanName);)
    }
}


// Pushes a function onto the call stack
// This information is used to determine statistics for each called function within a function
void ViewStats::PushCallstack(UInt32 swdHandle, UInt32 swfOffset, UInt64 funcTime)
{
    Lock::Locker locker(&ViewLock);
    Ptr<FuncTreeItem> newCall = *SF_HEAP_AUTO_NEW(this) FuncTreeItem();
    newCall->BeginTime = funcTime;
    newCall->FunctionId = MakeSwdOffsetKey(swdHandle, swfOffset);
    newCall->TreeItemId = ++NextTreeItemId;
    Callstack.PushBack(CallInfo(newCall, 0, 0));
    RefreshActiveLine();
}

// Pops a function from the call stack
// Records the time measured for a given function
// Thread-safe because function calls can be made from the renderer thread
void ViewStats::PopCallstack(UInt32 swdHandle, UInt32 swfOffset, UInt64 funcTime)
{
    if (!swdHandle)
        return;

    Lock::Locker locker(&ViewLock);

    SF_ASSERT(swdHandle != 0);
    if (Callstack.IsEmpty())
    {
        return;
    }

    if (MakeSwdOffsetKey(swdHandle, swfOffset) != Callstack.Back().FunctionInfo->FunctionId)
    {
        // Must have connected AMP in the middle of a frame
        return;
    }

    Ptr<FuncTreeItem> poppedCall = Callstack.Back().FunctionInfo;
    poppedCall->EndTime = poppedCall->BeginTime + funcTime;
    Callstack.PopBack();
    RefreshActiveLine();

    // Record the function time
    ParentChildFunctionPair keyParentChild;
    keyParentChild.FunctionId = MakeSwdOffsetKey(swdHandle, swfOffset);
    if (Callstack.GetSize() == 0)
    {
        // Top-level function
        keyParentChild.CallerId = 0;
    }
    else
    {
        // Record the Caller
        keyParentChild.CallerId = Callstack.Back().FunctionInfo->FunctionId;
    }

    if (!IsFunctionAggregation())
    {
        if (Callstack.GetSize() > 0)
        {
            Callstack.Back().FunctionInfo->Children.PushBack(poppedCall);
        }
        else
        {
            // Add a virtual root to user calls
            if (swdHandle == NativeCodeSwdHandle
                && swfOffset > Amp_Native_Function_Id_Begin_ObjectInterface 
                && swfOffset < Amp_Native_Function_Id_End_ObjectInterface)
            {
                UInt64 objectInterfaceRootId = MakeSwdOffsetKey(NativeCodeSwdHandle, Amp_Native_Function_Id_Begin_ObjectInterface);
                bool foundObjectInterfaceRoot = false;
                for (UPInt i = 0; i < FunctionRoots.GetSize(); ++i)
                {
                    if (FunctionRoots[i]->FunctionId == objectInterfaceRootId)
                    {
                        foundObjectInterfaceRoot = true;
                        FunctionRoots[i]->EndTime += funcTime;
                        FunctionRoots[i]->Children.PushBack(poppedCall);
                        break;
                    }
                }

                if (!foundObjectInterfaceRoot)
                {
                    Ptr<FuncTreeItem> newCall = *SF_HEAP_AUTO_NEW(this) FuncTreeItem();
                    newCall->BeginTime = 0;
                    newCall->EndTime = funcTime;
                    newCall->FunctionId = objectInterfaceRootId;
                    newCall->TreeItemId = ++NextTreeItemId;
                    newCall->Children.PushBack(poppedCall);
                    FunctionRoots.PushBack(newCall);
                }
            }
            else
            {
                FunctionRoots.PushBack(poppedCall);
            }
        }
    }
    else
    {
        FunctionStatMap::Iterator it = FunctionTimingMap.Find(keyParentChild);
        if (it == FunctionTimingMap.End())
        {
            // This caller-callee combination has not been recorded before


            // Add a virtual root to user calls
            if (keyParentChild.CallerId == 0)
            {
                if (swdHandle == NativeCodeSwdHandle
                    && swfOffset > Amp_Native_Function_Id_Begin_ObjectInterface 
                    && swfOffset < Amp_Native_Function_Id_End_ObjectInterface)
                {
                    ParentChildFunctionPair rootParentChild;
                    rootParentChild.CallerId = 0;
                    rootParentChild.FunctionId = MakeSwdOffsetKey(NativeCodeSwdHandle, Amp_Native_Function_Id_Begin_ObjectInterface);
                    keyParentChild.CallerId = rootParentChild.FunctionId;

                    if (it == FunctionTimingMap.End())
                    {
                        AmpFunctionStats stats;
                        stats.TimesCalled = 1;
                        stats.TotalTime = funcTime; 
                        FunctionTimingMap.Set(rootParentChild, stats);
                    }
                    else
                    {
                        ++it->Second.TimesCalled;
                        it->Second.TotalTime += funcTime;
                    }
                }
            }

            AmpFunctionStats stats;
            stats.TimesCalled = 0;
            stats.TotalTime = 0; 
            FunctionTimingMap.Set(keyParentChild, stats);
            it = FunctionTimingMap.Find(keyParentChild);
            SF_ASSERT(it != FunctionTimingMap.End());
        }

        // Update statistics
        AmpFunctionStats& funcStats = it->Second;
        ++funcStats.TimesCalled;

        // Update total time only if the same caller-callee pair 
        // is not already being timed further up in the stack. 
        bool recursion = false;
        for (UPInt i = 1; i < Callstack.GetSize(); ++i)
        {
            if (Callstack[i - 1].FunctionInfo->FunctionId == keyParentChild.CallerId 
                && Callstack[i].FunctionInfo->FunctionId == keyParentChild.FunctionId)
            {
                recursion = true;
                break;
            }
        }
        if (!recursion)
        {
            funcStats.TotalTime += funcTime;
        }
    }

    // Pause the debugger if the call stack is the right size
    if (CallstackDepthPause >= 0 && static_cast<int>(Callstack.GetSize()) >= CallstackDepthPause)
    {
        DebugEvent.ResetEvent();
        CallstackDepthPause = -1;
    }
}

UInt32 ViewStats::GetActiveLine() const
{
    Lock::Locker lock(&ActiveLock);
    return ActiveLineNumber;
}

void ViewStats::SetActiveLine(UInt32 lineNumber)
{
    if (!Callstack.IsEmpty())
    {
        Callstack.Back().LineNumber = lineNumber;
        RefreshActiveLine();
    }
    else
    {
        Lock::Locker lock(&ActiveLock);
        ActiveLineNumber = lineNumber;
    }
}

UInt64 ViewStats::GetActiveFile() const
{
    Lock::Locker lock(&ActiveLock);
    return ActiveFileId;
}

void ViewStats::SetActiveFile(UInt64 fileId)
{
    if (!Callstack.IsEmpty())
    {
        Callstack.Back().FileId = fileId;
        RefreshActiveLine();
    }
    else
    {
        Lock::Locker lock(&ActiveLock);
        ActiveFileId = fileId;
    }
}

// Locks and returns the array of times for a given ActionScript buffer
// Gives direct access to the array for fast modification
// There is one entry in the array for each byte offset, even though each instruction
// is several bytes long. This is done for fast access.
// Must call ReleaseBufferInstructionTimes when done modifying the buffer times
ArrayLH<UInt64>& ViewStats::LockBufferInstructionTimes(UInt32 swdHandle, UInt32 swfBufferOffset, 
                                                              unsigned bufferLength)
{
    InstructionTimingMutex.DoLock();

    SF_ASSERT(swdHandle != 0);
    UInt64 key = MakeSwdOffsetKey(swdHandle, swfBufferOffset);
    InstructionTimingMap::Iterator it = InstructionTimingsMap.Find(key);
    if (it == InstructionTimingsMap.End())
    {
        // profilertodo: In the case where the corresponding GASActionBufferData is freed, 
        // free this buffer after it is flushed to the network
        BufferInstructionTimes* pBuffer = 
            SF_HEAP_AUTO_NEW(this) BufferInstructionTimes(bufferLength);
        InstructionTimingsMap.Set(key, *pBuffer);
        it = InstructionTimingsMap.Find(key);
    }
    SF_ASSERT(bufferLength <= it->Second->Times.GetSize());

    return it->Second->Times;
}

// Unlocks the view stats
// Called to match LockBufferInstructionTimes
void ViewStats::ReleaseBufferInstructionTimes()
{
    InstructionTimingMutex.Unlock();
}

// Updates timing for given file and line
void ViewStats::RecordSourceLineTime(UInt64 lineTime)
{
    FileLinePair fileLine;
    fileLine.LineNumber = GetActiveLine();
    if (fileLine.LineNumber != 0)
    {
        fileLine.FileId = GetActiveFile();
        SourceLineStatMap::Iterator it = SourceLineTimingsMap.Find(fileLine);
        if (it == SourceLineTimingsMap.End())
        {
            SourceLineTimingsMap.Set(fileLine, 0);
            it = SourceLineTimingsMap.Find(fileLine);
            SF_ASSERT(it != SourceLineTimingsMap.End());
        }

        // Update statistics
        it->Second += lineTime;
    }
}

void ViewStats::RefreshActiveLine()
{
    if (!Callstack.IsEmpty())
    {
        CallInfo& back = Callstack.Back();
        Lock::Locker lock(&ActiveLock);
        ActiveFileId = back.FileId;
        ActiveLineNumber = back.LineNumber;
    }
}

// Obtains statistics for the movie view.
void ViewStats::GetStats(StatBag* bag, bool reset)
{
    if (bag != NULL)
    {
        Ptr<ProfileFrame> stats = *SF_HEAP_AUTO_NEW(this) ProfileFrame();
        CollectTimingStats(stats);

        TimerStat timerStat;
        timerStat.Reset();
        timerStat.AddTicks(stats->AdvanceTime);
        bag->Add(StatMV_Advance_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->TimelineTime);
        bag->Add(StatMV_Timeline_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->ActionTime);
        bag->Add(StatMV_Action_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->InputTime);
        bag->Add(StatMV_Input_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->MouseTime);
        bag->Add(StatMV_Mouse_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->GetVariableTime);
        bag->Add(StatMV_GetVariable_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->SetVariableTime);
        bag->Add(StatMV_SetVariable_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->InvokeTime);
        bag->Add(StatMV_Invoke_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->DisplayTime);
        bag->Add(StatMV_Display_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->FontThrashingTime);
        bag->Add(StatMV_FontThrash_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->GradientGenTime);
        bag->Add(StatMV_GradientGen_Tks, &timerStat);
        timerStat.Reset();
        timerStat.AddTicks(stats->FontMissTime);
        bag->Add(StatMV_FontMiss_Tks, &timerStat);
    }

    if (reset)
    {
        ClearAmpFunctionStats();
    }
}

// Sets the friendly name for the view
void ViewStats::SetName(const char* name)
{
    ViewName = name;

    // Remove the path, if any
    UPInt nameLength = ViewName.GetLength();
    for (UPInt i = 0; i < nameLength; ++i)
    {
        UPInt index = nameLength - i - 1;
        if (ViewName[index] == '/' || ViewName[index] == '\\')
        {
            ViewName = ViewName.Substring(index + 1, nameLength);
            break;
        }
    }

}

// Gets the friendly name for the view
const String& ViewStats::GetName() const
{
    return ViewName;
}


// Recovers the unique handle for the view
UInt32 ViewStats::GetViewHandle() const 
{ 
    Lock::Locker locker(&ViewLock);
    return ViewHandle; 
}

// Assigns the flash file properties
void ViewStats::SetMovieDef(MovieDef* movieDef)
{
    Lock::Locker locker(&ViewLock);
    if (movieDef != NULL)
    {
        Version = movieDef->GetVersion();
        Width = movieDef->GetWidth();
        Height = movieDef->GetHeight();
        FrameRate = movieDef->GetFrameRate();
        FrameCount = movieDef->GetFrameCount();
    }
}

// Flash version
UInt32 ViewStats::GetVersion() const
{
    Lock::Locker locker(&ViewLock);
    return Version;
}

// Flash display width
float ViewStats::GetWidth() const
{
    Lock::Locker locker(&ViewLock);
    return Width;
}

// Flash display height
float ViewStats::GetHeight() const
{
    Lock::Locker locker(&ViewLock);
    return Height;
}

// Flash file frame rate
float ViewStats::GetFrameRate() const
{
    Lock::Locker locker(&ViewLock);
    return FrameRate;
}

// Total number of frames in the flash file
UInt32 ViewStats::GetFrameCount() const
{
    Lock::Locker locker(&ViewLock);
    return FrameCount;
}

// Returns the current Flash frame for this movie
// Called by AMP
UInt32 ViewStats::GetCurrentFrame() const 
{ 
    Lock::Locker lock(&ViewLock);
    return CurrentFrame; 
}

// Sets the current Flash frame for this movie
// Called by GFx
void ViewStats::SetCurrentFrame(UInt32 frame)
{
    Lock::Locker lock(&ViewLock);
    CurrentFrame = frame;
}

// Returns a time for instruction sampling
// samplePeriod is the average number of samples skipped
// samplePeriod = 0 means timing every instruction
UInt64 ViewStats::GetInstructionTime(UInt32 samplePeriod)
{
    UInt64 timeDiff = 0;

    if (samplePeriod == 0)
    {
        UInt64 nextTimer = Timer::GetRawTicks();
        timeDiff = nextTimer - LastTimer;
        LastTimer = nextTimer;
    }
    else
    {
        if (LastTimer != 0)
        {
            timeDiff = (Timer::GetRawTicks() - LastTimer) * samplePeriod;
        }

        if (SkipSamples == 0)
        {
            LastTimer = Timer::GetRawTicks();
            UInt64 random = RandomGen.NextRandom();
            random *= 2 * samplePeriod;
            random /= 0xFFFFFFFF;
            SkipSamples = static_cast<UInt32>(random);
        }
        else
        {
            LastTimer = 0;
            --SkipSamples;
        }
    }

    return timeDiff;
}

// Adds a marker to be displayed by AMP
void ViewStats::AddMarker(const char* markerType)
{
    StringHashLH<UInt32>::Iterator it = Markers.Find(markerType);

    if (it == Markers.End())
    {
        Markers.Add(markerType, 1);
    }
    else
    {
        ++it->Second;
    }
}

void ViewStats::AddGcRoots(UInt32 numRoots)
{
    RootsNumber += numRoots;
}

void ViewStats::AddGcFreedRoots(UInt32 numFreedRoots)
{
    FreedRootsNumber += numFreedRoots;
}


// Step into the function on the current line when the debugger is paused
void ViewStats::DebugStep(int depth)
{
    CallstackDepthPause = static_cast<int>(Callstack.GetSize()) + depth;
    DebugEvent.PulseEvent();
}

void ViewStats::DebugGo()
{
    CallstackDepthPause = -1;
    DebugEvent.SetEvent();
}

void ViewStats::DebugPause()
{
    CallstackDepthPause = -1;
    DebugEvent.ResetEvent();
}

// Pause the debugger
void ViewStats::DebugWait()
{
    DebugEvent.Wait();
}

bool ViewStats::IsDebugPaused() const
{
    return !DebugEvent.IsSignaled();
}


// Collects all the instruction statistics for the frame and 
// puts them into the the ProfileFrame structure for sending to the AMP client.
void ViewStats::CollectAmpInstructionStats(MovieProfile* movieProfile)
{
    if (InstructionTimingMutex.TryLock())
    {
        InstructionTimingMap::Iterator it;
        for (it = InstructionTimingsMap.Begin(); it != InstructionTimingsMap.End(); ++it)
        {
            Ptr<BufferInstructionTimes>& buffer = it->Second;

            // Count non-zero entries so we can size the packed array
            unsigned nonZeroTimes = 0;
            for (UPInt i = 0; i < buffer->Times.GetSize(); i++)
            {
                if (buffer->Times[i] != 0)
                {
                    ++nonZeroTimes;
                }
            }

            if (nonZeroTimes > 0)
            {
                MovieInstructionStats::ScriptBufferStats* nonZeroValues = SF_HEAP_AUTO_NEW(movieProfile) 
                    MovieInstructionStats::ScriptBufferStats();

                nonZeroValues->SwdHandle = static_cast<UInt32>(it->First >> 32);
                nonZeroValues->BufferOffset = static_cast<UInt32>(it->First);
                nonZeroValues->BufferLength = static_cast<UInt32>(buffer->Times.GetSize());
                nonZeroValues->InstructionTimesArray.Resize(nonZeroTimes);

                unsigned nonZeroCount = 0;
                for (UPInt i = 0; i < buffer->Times.GetSize(); i++)
                {
                    if (buffer->Times[i] != 0)
                    {
                        MovieInstructionStats::InstructionTimePair& pair = 
                            nonZeroValues->InstructionTimesArray[nonZeroCount];
                        pair.Offset = static_cast<UInt32>(i);
                        // convert to microseconds
                        pair.Time = buffer->Times[i] * 1000000 / Timer::GetRawFrequency(); 
                        ++nonZeroCount;
                    }
                }

                movieProfile->InstructionStats->BufferStatsArray.PushBack(*nonZeroValues);
            }
        }
        InstructionTimingMutex.Unlock();
    }
}

// Clears the ActionScript buffer times for recording the next frame
void ViewStats::ClearAmpInstructionStats()
{
    if (InstructionTimingMutex.TryLock())
    {
        InstructionTimingsMap.Clear();

//         InstructionTimingMap::Iterator it;
//         for (it = InstructionTimingsMap.Begin(); it != InstructionTimingsMap.End(); ++it)
//         {
//             ArrayLH<UInt64>& buffer = it->Second->Times;
//             memset(&buffer[0], 0, buffer.GetSize() * sizeof(buffer[0]));
//         }
        InstructionTimingMutex.Unlock();
    }
}

// Collects the function statistics for the frame and
// puts them into the the ProfileFrame structure for sending to the AMP client.
void ViewStats::CollectAmpFunctionStats(MovieProfile* movieProfile)
{
    Lock::Locker locker(&ViewLock);

    // Copy over all the non-zero function timings
    FunctionStatMap::Iterator funcTimeIter;
    for (funcTimeIter = FunctionTimingMap.Begin(); funcTimeIter != FunctionTimingMap.End(); ++funcTimeIter)
    {
        AmpFunctionStats& funcTime = funcTimeIter->Second;
        if (funcTime.TimesCalled > 0 || funcTime.TotalTime > 0)
        {
            MovieFunctionStats::FuncStats funcStats;
            funcStats.FunctionId = funcTimeIter->First.FunctionId;
            funcStats.CallerId = funcTimeIter->First.CallerId;
            funcStats.TimesCalled = funcTime.TimesCalled;
            funcStats.TotalTime = funcTime.TotalTime; // microseconds
            movieProfile->FunctionStats->FunctionTimings.PushBack(funcStats);

            // Send the friendly name along with the function Id
            UInt64 swfHandleAndOffset = funcTimeIter->First.FunctionId;
            FunctionDescMap::Iterator funcNameIter = FunctionInfoMap.Find(swfHandleAndOffset);
            if (funcNameIter != FunctionInfoMap.End())
            {
                FunctionDescMap& descMap = movieProfile->FunctionStats->FunctionInfo;
                if (descMap.Find(swfHandleAndOffset) == descMap.End())
                {
                    descMap.Set(swfHandleAndOffset, funcNameIter->Second);
                }
            }
        }
    }

    HashSet<UInt64> functionIds;
    if (!IsFunctionAggregation())
    {
        for (UPInt i = 0; i < FunctionRoots.GetSize(); ++i)
        {
            movieProfile->FunctionTreeStats->FunctionRoots.PushBack(FunctionRoots[i]);
            FunctionRoots[i]->GetAllFunctions(&functionIds);
        }
    }

    for (HashSet<UInt64>::Iterator it = functionIds.Begin(); it != functionIds.End(); ++it)
    {
        FunctionDescMap::Iterator funcNameIter = FunctionInfoMap.Find(*it);
        if (funcNameIter != FunctionInfoMap.End())
        {
            FunctionDescMap& descMap = movieProfile->FunctionTreeStats->FunctionInfo;
            if (descMap.Find(*it) == descMap.End())
            {
                descMap.Set(*it, funcNameIter->Second);
            }
        }
    }
}

// Clears the function times stored in the view for recording the next frame
void ViewStats::ClearAmpFunctionStats()
{
    Lock::Locker locker(&ViewLock);

    FunctionTimingMap.Clear();
    FunctionRoots.Clear();
    NextTreeItemId = 0;
}

// Collects the source line statistics for the frame and
// puts them into the the ProfileFrame structure for sending to the AMP client.
void ViewStats::CollectAmpSourceLineStats(MovieProfile* movieProfile)
{
    Lock::Locker locker(&ViewLock);

    // Copy over all the non-zero timings
    SourceLineStatMap::Iterator sourceTimeIter;
    for (sourceTimeIter = SourceLineTimingsMap.Begin(); sourceTimeIter != SourceLineTimingsMap.End(); ++sourceTimeIter)
    {
        if (sourceTimeIter->Second > 0)
        {
            MovieSourceLineStats::SourceStats sourceStats;
            sourceStats.FileId = sourceTimeIter->First.FileId;
            sourceStats.LineNumber = sourceTimeIter->First.LineNumber;
            sourceStats.TotalTime = sourceTimeIter->Second * 1000000 / Timer::GetRawFrequency(); // microseconds
            movieProfile->SourceLineStats->SourceLineTimings.PushBack(sourceStats);


            // Send the file name along with the file Id
            SourceFileDescMap::Iterator fileNameIter = SourceLineInfoMap.Find(sourceStats.FileId);
            if (fileNameIter != SourceLineInfoMap.End())
            {
                SourceFileDescMap& descMap = movieProfile->SourceLineStats->SourceFileInfo;
                if (descMap.Find(sourceStats.FileId) == descMap.End())
                {
                    descMap.Set(sourceStats.FileId, fileNameIter->Second);
                }
            }
        }
    }
}

// Clears the source line times stored in the view for recording the next frame
void ViewStats::ClearAmpSourceLineStats()
{
    Lock::Locker locker(&ViewLock);

    SourceLineStatMap::Iterator it;
    for (it = SourceLineTimingsMap.Begin(); it != SourceLineTimingsMap.End(); ++it)
    {
        it->Second = 0;
    }
}

class FuncStatsVisitor
{
public:
    FuncStatsVisitor(ProfileFrame* profileFrame, ViewStats* callingView) : FrameInfo(profileFrame), CallingView(callingView) { }
    void operator()(const FuncTreeItem* treeItem)
    {
        CallingView->UpdateStats(treeItem->FunctionId, static_cast<UInt32>(treeItem->EndTime - treeItem->BeginTime), 1, FrameInfo);
    }
private:
    ProfileFrame* FrameInfo;
    ViewStats*    CallingView;
};

// Collects the performance statistics for the view and
// puts them into the the ProfileFrame structure for sending to the AMP client.
void ViewStats::CollectTimingStats(ProfileFrame* pFrameInfo)
{
    Lock::Locker locker(&ViewLock);

    if (!IsFunctionAggregation())
    {
        FuncStatsVisitor visitor(pFrameInfo, this);
        for (UPInt i = 0; i < FunctionRoots.GetSize(); ++i)
        {
            FuncTreeItem* rootItem = FunctionRoots[i];
            rootItem->Visit(visitor);
        }
    }
    else
    {
        for (FunctionStatMap::Iterator it = FunctionTimingMap.Begin(); it != FunctionTimingMap.End(); ++it)
        {
            UpdateStats(it->First.FunctionId, static_cast<UInt32>(it->Second.TotalTime), static_cast<UInt32>(it->Second.TimesCalled), pFrameInfo);
        }
    }
}

// Collects the markers for the view and
// puts them into the the ProfileFrame structure for sending to the AMP client.
void ViewStats::CollectMarkers(MovieProfile* movieProfile)
{
    Lock::Locker locker(&ViewLock);
    for (StringHashLH<UInt32>::Iterator it = Markers.Begin(); it != Markers.End(); ++it)
    {
        Ptr<MovieProfile::MarkerInfo> info = *SF_HEAP_AUTO_NEW(movieProfile) MovieProfile::MarkerInfo();
        info->Name = it->First;
        info->Number = it->Second;
        movieProfile->Markers.PushBack(info);
    }
}

// Clear all markers
void ViewStats::ClearMarkers()
{
    Lock::Locker locker(&ViewLock);
    Markers.Clear();
}

void ViewStats::CollectGcStats(ProfileFrame* frameProfile)
{
    Lock::Locker locker(&ViewLock);
    frameProfile->GcRootsNumber += RootsNumber;
    frameProfile->GcFreedRootsNumber += FreedRootsNumber;
}

void ViewStats::ClearGcStats()
{
    Lock::Locker locker(&ViewLock);
    RootsNumber = 0;
    FreedRootsNumber = 0;
}

void ViewStats::UpdateStats(UInt64 functionId, UInt32 functionTime, UInt32 functionCalls, ProfileFrame* frameProfile)
{
    UInt32 funcOffset = static_cast<UInt32>(functionId);
    UInt32 funcSwdHandle = static_cast<UInt32>(functionId >> 32);
    if (funcSwdHandle == NativeCodeSwdHandle)
    {
        switch (funcOffset)
        {
        case Amp_Native_Function_Id_Advance:
            frameProfile->AdvanceTime += functionTime;
            break;
        case Amp_Native_Function_Id_AdvanceFrame:
            frameProfile->TimelineTime += functionTime;
            break;
        case Amp_Native_Function_Id_DoActions:
            frameProfile->ActionTime += functionTime;
            break;
        case Amp_Native_Function_Id_ProcessInput:
            frameProfile->InputTime += functionTime;
            break;
        case Amp_Native_Function_Id_ProcessMouse:
            frameProfile->MouseTime += functionTime;
            break;
        case Amp_Native_Function_Id_GcCollect:
            frameProfile->GcCollectTime += functionTime;
            break;
        case Amp_Native_Function_Id_GcMarkInCycle:
            frameProfile->GcMarkInCycleTime += functionTime;
            break;
        case Amp_Native_Function_Id_GcScanInUse:
            frameProfile->GcScanInUseTime += functionTime;
            break;
        case Amp_Native_Function_Id_GcFreeGarbage:
            frameProfile->GcFreeGarbageTime += functionTime;
            break;
        case Amp_Native_Function_Id_GcFinalize:
            frameProfile->GcFinalizeTime += functionTime;
            break;
        case Amp_Native_Function_Id_GcDelayedCleanup:
            frameProfile->GcDelayedCleanupTime += functionTime;
            break;
        case Amp_Native_Function_Id_GetVariable:
        case Amp_Native_Function_Id_GetVariableArray:
        case Amp_Native_Function_Id_GetVariableArraySize:
        case Amp_Native_Function_Id_ObjectInterface_GetMember:
        case Amp_Native_Function_Id_ObjectInterface_GetArraySize:
        case Amp_Native_Function_Id_ObjectInterface_GetElement:
        case Amp_Native_Function_Id_ObjectInterface_GetText:
            frameProfile->UserTime += functionTime;
            frameProfile->GetVariableTime += functionTime;
            ++frameProfile->UserCalls;
            break;
        case Amp_Native_Function_Id_SetVariable:
        case Amp_Native_Function_Id_SetVariableArray:
        case Amp_Native_Function_Id_SetVariableArraySize:
        case Amp_Native_Function_Id_ObjectInterface_SetMember:
        case Amp_Native_Function_Id_ObjectInterface_SetArraySize:
        case Amp_Native_Function_Id_ObjectInterface_SetElement:
        case Amp_Native_Function_Id_ObjectInterface_SetText:
            frameProfile->UserTime += functionTime;
            frameProfile->SetVariableTime += functionTime;
            ++frameProfile->UserCalls;
            break;
        case Amp_Native_Function_Id_Invoke:
        case Amp_Native_Function_Id_InvokeAlias:
        case Amp_Native_Function_Id_InvokeArgs:
        case Amp_Native_Function_Id_InvokeAliasArgs:
        case Amp_Native_Function_Id_ObjectInterface_Invoke:
        case Amp_Native_Function_Id_ObjectInterface_InvokeClosure:
            frameProfile->UserTime += functionTime;
            frameProfile->InvokeTime += functionTime;
            ++frameProfile->UserCalls;
            break;
        case Amp_Native_Function_Id_Display:
        case Amp_Native_Function_Id_NextCapture:
        case Amp_Native_Function_Id_Draw:
            frameProfile->DisplayTime += functionTime;
            break;
        case Amp_Native_Function_Id_Present:
            frameProfile->PresentTime += functionTime;
            break;
        case Amp_Native_Function_Id_Tessellate:
            // no longer care about this. Report thrashing (eviction) time instead, which includes tessellation
            break;
        case Amp_Native_Function_Id_GlyphTextureMapper_Create:
            frameProfile->NumFontCacheTextureUpdates += functionCalls;
            break;
        case Amp_Native_Function_Id_GradientFill:
            frameProfile->GradientGenTime += functionTime;
            break;
        case Amp_Native_Function_Id_GlyphCache_RasterizeGlyph:
        case Amp_Native_Function_Id_GlyphCache_RasterizeShadow:
            frameProfile->FontMissTime += functionTime;
            frameProfile->FontMisses += functionCalls;
            break;
        case Amp_Native_Function_Id_GlyphCache_EvictText:
            frameProfile->FontThrashing += functionCalls;
            frameProfile->FontThrashingTime += functionTime;
            break;
        }
    }
}


} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif
