/**************************************************************************

Filename    :   GameDataLoader.h
Content     :   Sample asynchronous data loader for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameDataLoader_H
#define INC_GameDataLoader_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Threads.h"
#include "GameMemoryFileCache.h"

using namespace Scaleform;

class GameDataLoader : public RefCountBase<GameDataLoader, Stat_Default_Mem>
{
public:
    struct  LoaderDef
    {
        String                  Name;
        Array<String>           Filenames;
        unsigned                DataChunkSz;
        unsigned                DelayInSeconds;

        LoaderDef() : DataChunkSz(0), DelayInSeconds(0) {}
    };

    GameDataLoader(const LoaderDef& defn);
    virtual ~GameDataLoader();

    // Retrieve name of task
    const String&  GetName() const         { return Name; }
    // Returns true if the task is done (subclasses are expected to set this flag)
    bool            IsDone() const          { return bDone; }
    // Returns true if the task is paused
    bool            IsPaused() const        
    { 
        // No locking needed
        return (Status & Paused) != 0; 
    }

    void            Start();
    void            Stop();

    void            Pause(bool pause)
    {
        Mutex::Locker lock(&IOStatusMutex);
        while(Status & IOInProgess)
            WaitingForIoIdle.Wait(&IOStatusMutex);
        if (pause)
            Status |= Paused;
        else
            Status &= ~Paused;
    }

    void            EnableIO(bool enable)  
    { 
        if (enable)
        {
            Mutex::Locker lock(&IOEnableMutex);
            Status |= IOEnabled;
            WaitingForIoEnabled.Notify();
        }
        else
        {
            Status &= ~IOEnabled;
            Mutex::Locker lock(&IOStatusMutex);
            while(Status & IOInProgess)
                WaitingForIoIdle.Wait(&IOStatusMutex);
        }
    }

    GameMemoryFileCache*    GetMemoryFileCache() const  { return FileCache; }
    float                   GetProgress();

protected:
    enum LoadStatus
    {
        IOEnabled     = 0x01,   // IO has not been locked by other task and loader has access
        IOInProgess   = 0x02,   // IO resources are locked by the loader
        Paused        = 0x04,   // IO access is paused; yielding resources to other tasks
        StopRequested = 0x08    // Set when higher priority IO tasks request resource lock
    };

    static int          ReadFunc(Thread* h, void* d);

    virtual void        OnPreload()     {}
    virtual void        OnPostload()    {}

    String              Name;
    volatile bool       bDone;

    LoaderDef           Defn;

    Ptr<Thread>         pThread;
    volatile unsigned   Status;

    volatile UPInt      NumFilesRead;
    volatile float      CurrentFilePct;

    Ptr<GameMemoryFileCache>   FileCache;

    Mutex              IOEnableMutex;
    WaitCondition      WaitingForIoEnabled;
    Mutex              IOStatusMutex;
    WaitCondition      WaitingForIoIdle;
};


#endif  // INC_GameDataLoader_H
