/**************************************************************************

Filename    :   GameDataLoader.cpp
Content     :   Simple game data loader implementation
Created     :   Oct, 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameDataLoader.h"
#include "Kernel/SF_SysFile.h"

//////////////////////////////////////////////////////////////////////////
//

#undef new

GameDataLoader::GameDataLoader() :
    Status(0), TotalRead(0), FileLen(0), pDataChunk(NULL), DataChunkSize(0)
{
}

GameDataLoader::~GameDataLoader()
{ 
    Stop();
}

void GameDataLoader::OnDataReceived(UByte* pdata, int dataSize)
{
    SF_UNUSED(pdata);
    SF_UNUSED(dataSize);

    // Do something here...
    Thread::MSleep(20);
}

void GameDataLoader::OnLoadingError()
{
    fprintf(stderr, "Can not open file: %s\n", FileName.ToCStr());
}
void GameDataLoader::OnLoadingFinished()
{
    fprintf(stderr, "Data loading completed\n");
}

void GameDataLoader::LoadFile(const char* fname, int dataChunkSize)
{
    Stop();
    if (!pThread)
    {
        Thread::CreateParams params(ReadFunc, this, 16 * 1024);
        pThread = *new Thread(params);
    }
    DataChunkSize = dataChunkSize;
    pDataChunk = new UByte[DataChunkSize];
    FileName = fname;
    pThread->Start();
}

//////////////////////////////////////////////////////////////////////////
//

void GameDataLoader::Enable(bool enable)
{
    if (enable)
    {
        Mutex::Locker lock(&EnableMutex);
        Status |= IOEnabled;
        EnableWC.Notify();
    }
    else
    {
        Status &= ~IOEnabled;
        Mutex::Locker lock(&IOFinishedMutex);
        while(Status & IOInProgess)
            IOFinishedWC.Wait(&IOFinishedMutex);
    }

}

void GameDataLoader::Restart()
{
    if (!FileName.IsEmpty())
    {
        if (pThread)
        {
            EnableMutex.DoLock();
            Status |= StopRequested;
            EnableWC.Notify();
            EnableMutex.Unlock();
            pThread->Wait();
            Status &= ~StopRequested;
            pFile = NULL;
        }
        else
        {
            Thread::CreateParams params(ReadFunc, this, 10*1024);
            pThread = *new Thread(params);
        }
        pThread->Start();
        fprintf(stderr, "Data loading restarted\n");
    }
}

void GameDataLoader::Stop()
{
    if (pThread)
    {
        EnableMutex.DoLock();
        Status |= StopRequested;
        EnableWC.Notify();
        EnableMutex.Unlock();
        pThread->Wait();

        delete[] pDataChunk;
        pDataChunk = NULL;
        DataChunkSize = 0;

        if (pFile)
            pFile->Close();
        pFile = 0;
        FileName = "";
    }
}

float GameDataLoader::GetProcess()
{
    if (!pFile || FileLen == 0)
        return 0.0f;
    return 100.0f * TotalRead/FileLen;
}

//////////////////////////////////////////////////////////////////////////
//

int GameDataLoader::ReadFunc(Thread* h, void* d)
{
    SF_UNUSED(h);

    GameDataLoader* loader = (GameDataLoader*)d;
    while (1)
    {
        {
            Mutex::Locker lock(&loader->EnableMutex);
            while (!(loader->Status & (StopRequested | IOEnabled)))
                loader->EnableWC.Wait(&loader->EnableMutex);
            if (loader->Status & StopRequested)
                break;
        }
        {
            Mutex::Locker lock(&loader->IOFinishedMutex);
            loader->Status |= IOInProgess;
        }

        bool done = false;
        bool error = false;
        int recv_bytes = 0;
        if (!loader->pFile && !loader->FileName.IsEmpty())
        {
            loader->pFile = *new SysFile(loader->FileName);
            if (!loader->pFile->IsValid())
                error = true;
            else {
                loader->TotalRead = 0;
                loader->FileLen = loader->pFile->GetLength();
            }
        } 
        else 
        {
            recv_bytes = loader->pFile->Read(loader->pDataChunk, loader->DataChunkSize);
            loader->TotalRead += recv_bytes;
            if (loader->TotalRead >= loader->FileLen)
                done = true;
        }

        {
            Mutex::Locker lock(&loader->IOFinishedMutex);
            loader->Status &= ~IOInProgess;
            loader->IOFinishedWC.Notify();
        }

        if (error)
        {
            loader->pFile = 0;
            loader->Status |= StopRequested;
            loader->OnLoadingError();
        }
        else if (done)
        {
            loader->Status |= StopRequested;
            loader->OnLoadingFinished();
        }
        else if(recv_bytes > 0)
            loader->OnDataReceived(loader->pDataChunk, recv_bytes);

    }
    return 0;
}
