/**************************************************************************

Filename    :   GameDataLoader.cpp
Content     :   Sample asynchronous data loader for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameDataLoader.h"
// #include "GSysFile.h"
#include "GFx.h"
#include "GFx_Kernel.h"
#include "Kernel/SF_HeapNew.h"

using namespace GFx;

GameDataLoader::GameDataLoader(const LoaderDef& defn)
    : Defn(defn), bDone(false), Status(IOEnabled), NumFilesRead(0), CurrentFilePct(0.f)
{
    if (!pThread)
    {
        Thread::CreateParams params(ReadFunc, this, 16 * 1024);
        pThread = *new Thread(params);
    }
}


GameDataLoader::~GameDataLoader()
{
    Stop();
}

void    GameDataLoader::Start()
{
    pThread->Start();
}

void    GameDataLoader::Stop()
{
    if (pThread)
    {
        IOEnableMutex.DoLock();
        Status |= StopRequested;
        WaitingForIoEnabled.Notify();
        IOEnableMutex.Unlock();
        pThread->Wait();
    }
}

float GameDataLoader::GetProgress()
{
    return (NumFilesRead + CurrentFilePct) / (float)Defn.Filenames.GetSize();
}

int GameDataLoader::ReadFunc(Thread* h, void* d)
{
    SF_UNUSED1(h);
    GameDataLoader* loader = (GameDataLoader*)d;

    loader->FileCache = *SF_HEAP_AUTO_NEW(loader) GameMemoryFileCache();

    unsigned                fileIndex   = 0;
    const char*             filename    = NULL;
    Ptr<File>               psrcFile;
    Ptr<GameMemoryFile>     pmemFile;
    
    Array<String>&          filenames       = loader->Defn.Filenames;
    unsigned                fileSize        = 0;
    unsigned                fileReadDataSz  = 0;

    bool                    doneRead    = false;

    // Use provided chunk size; if invalid, use defaults
    unsigned dataChunkSz = loader->Defn.DataChunkSz;
    if (dataChunkSz == 0)
    {
#if defined(SF_OS_WII)
        dataChunkSz = 64 * 1024;
#else
        dataChunkSz = 128 * 1024;
#endif
    }
    UByte*   dataChunkBuffer = (UByte*)SF_ALLOC(dataChunkSz, Stat_Default_Mem);

    // Perform delay before actual load if requested
    Thread::MSleep(loader->Defn.DelayInSeconds * 1000);

    loader->OnPreload();

    while(1)
    {
        bool    paused;
        {
            Mutex::Locker lock(&loader->IOEnableMutex);
            while ( !(loader->Status & (StopRequested | IOEnabled)) )
                loader->WaitingForIoEnabled.Wait(&loader->IOEnableMutex);
            if (loader->Status & StopRequested)
                break;
        }
        {
            Mutex::Locker lock(&loader->IOStatusMutex);
            paused = loader->IsPaused();
            if (!paused) loader->Status |= IOInProgess;
            else loader->WaitingForIoIdle.Notify();
        }

        if (!paused)
        {
            // If need to load new file
            if (!psrcFile &&  fileIndex < filenames.GetSize())
            {
                filename = filenames[fileIndex];
                psrcFile = *new SysFile(filename);
                fileSize = psrcFile->GetLength();
                if (!psrcFile->IsValid())
                {
                    SF_DEBUG_MESSAGE2(1, "%s failed to load '%s'!\n", loader->Name.ToCStr(), filename);
                    psrcFile = NULL;
                    fileReadDataSz = 0;
                    fileIndex++;    // Skip to next file

                    loader->CurrentFilePct = 0;
                    loader->NumFilesRead++;
                }
                else
                {
                    SF_ASSERT(fileSize > 0);
                    pmemFile = *loader->FileCache->CreateFile(filename, fileSize);
                    SF_DEBUG_MESSAGE2(1, "%s created memory file for '%s'!\n", loader->Name.ToCStr(), filename);
                }
            }
            else if (fileIndex == filenames.GetSize())
            {            
                doneRead = true;
            }
            else
            {
                signed gotBytes = psrcFile->Read(dataChunkBuffer, dataChunkSz);
                SF_ASSERT(gotBytes >= 0);

                pmemFile->WriteData(fileReadDataSz, dataChunkBuffer, gotBytes);
                fileReadDataSz += gotBytes;

                loader->CurrentFilePct = fileReadDataSz / (float)fileSize;

                if (fileReadDataSz >= fileSize)
                {
                    SF_DEBUG_MESSAGE2(1, "%s done loading '%s'!\n", loader->Name.ToCStr(), filename);
                    psrcFile = NULL;
                    fileReadDataSz = 0;
                    fileIndex++;

                    loader->CurrentFilePct = 0;
                    loader->NumFilesRead++;
                }
            }

            {
                Mutex::Locker lock(&loader->IOStatusMutex);
                loader->Status &= ~IOInProgess;
                loader->WaitingForIoIdle.Notify();
            }
        }

        if (doneRead)
        {
            loader->Status |= StopRequested;
        }
    }

    SF_FREE(dataChunkBuffer);

    SF_DEBUG_MESSAGE1(1, "%s done loading all files!\n", loader->Name.ToCStr());

    loader->OnPostload();

    loader->bDone = true;
    return 0;
}
