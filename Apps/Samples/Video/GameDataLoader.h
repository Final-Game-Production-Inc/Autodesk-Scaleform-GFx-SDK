/**************************************************************************

Filename    :   GameDataLoader.h
Content     :   Simple game data loader
Created     :   Oct, 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef _INC_GAME_DATA_LOADER_H_
#define _INC_GAME_DATA_LOADER_H_

#include "Kernel/SF_Threads.h"
#include "Kernel/SF_File.h"

using namespace Scaleform;

//////////////////////////////////////////////////////////////////////////
//

class GameDataLoader
{
public:
    GameDataLoader();
    virtual ~GameDataLoader();

    void LoadFile(const char* fileName, int dataChunkSize);

    virtual void OnDataReceived(UByte* pdata, int dataSize);
    virtual void OnLoadingError();
    virtual void OnLoadingFinished();

    void Enable(bool enable);
    void Stop();
    void Restart();

    float GetProcess();

private:
    enum LoadStatus
    {
        IOEnabled     = 0x01,
        IOInProgess   = 0x02,
        StopRequested = 0x04
    };

    static int ReadFunc(Thread* h, void* d);

    Ptr<Thread>         pThread;
    volatile unsigned   Status;
    AtomicInt<int>      TotalRead;
    volatile int        FileLen;

    Ptr<File>       pFile;
    String          FileName;
    UByte*          pDataChunk;
    int             DataChunkSize;
    Mutex           EnableMutex;
    WaitCondition   EnableWC;
    Mutex           IOFinishedMutex;
    WaitCondition   IOFinishedWC;
};

#endif // _INC_GAME_DATA_LOADER_H_
