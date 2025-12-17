/**************************************************************************

Filename    :   GFx_PlayerTasks.cpp
Content     :   Input handling
Created     :   
Authors     :   Artem Bolgar

Notes       :   This file contains class declarations used in
GFxPlayerImpl.cpp only. Declarations that need to be
visible by other player files should be placed
in GFxCharacter.h.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx {

String GetUrlStrGfx(const String& url)
{
    String urlStrGfx;
    if ((url.GetSize() > 4) &&
        (String::CompareNoCase(url.ToCStr() + (url.GetSize() - 4), ".swf") == 0) )
    {
        urlStrGfx.Clear();
        urlStrGfx.AppendString(url.ToCStr(), url.GetSize() - 4);
        urlStrGfx += ".gfx";
    }
    return urlStrGfx;
}

// 
MoviePreloadTask::MoviePreloadTask(MovieImpl* pmovieRoot, const String& url, bool stripped, bool quietOpen)
: Task(Task::Id_MovieDataLoad), Url(url), Done(0)
{
    pLoadStates = *SF_NEW LoadStates(pmovieRoot->pMainMovieDef->pLoaderImpl, pmovieRoot->GetStateBagImpl());
    LoadFlags = pmovieRoot->pMainMovieDef->GetLoadFlags() | Loader::LoadImageFiles;
    // we don't want to wait for anything
    LoadFlags &= ~(Loader::LoadWaitFrame1 | Loader::LoadWaitCompletion);
    if (quietOpen)
        LoadFlags |= Loader::LoadQuietOpen;
    pmovieRoot->GetMainMoviePath(&Level0Path);
    if (stripped)
        UrlStrGfx = GetUrlStrGfx(Url);
}

void MoviePreloadTask::Execute()
{
    if (UrlStrGfx.GetLength() > 0)
    {
        URLBuilder::LocationInfo loc(URLBuilder::File_LoadMovie, UrlStrGfx, Level0Path);
        pDefImpl = *LoaderImpl::CreateMovie_LoadState(pLoadStates, loc, LoadFlags);
    }
    if (!pDefImpl) 
    {
        //printf("MoviePreloadTask::Execute()  ");
        URLBuilder::LocationInfo loc(URLBuilder::File_LoadMovie, Url, Level0Path);
        pDefImpl = *LoaderImpl::CreateMovie_LoadState(pLoadStates, loc, LoadFlags);
        //printf("   %p\n",pDefImpl);
    }
    AtomicOps<unsigned>::Store_Release(&Done, 1);
}

bool MoviePreloadTask::IsDone() const 
{
    unsigned done = AtomicOps<unsigned>::Load_Acquire(&Done);
    return done == 1;
}

MovieDefImpl* MoviePreloadTask::GetMoiveDefImpl()
{
    return pDefImpl;
}

// ****** GFxLoadingMovieEntry

LoadQueueEntryMT::LoadQueueEntryMT(LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot)
: pNext(NULL), pPrev(NULL), pMovieImpl(pmovieRoot), pQueueEntry(pqueueEntry)
{
}

LoadQueueEntryMT::~LoadQueueEntryMT()
{
    delete pQueueEntry;
}

void LoadQueueEntryMT::Cancel()
{
    pQueueEntry->Canceled = true;
}

#ifdef GFX_AS2_ENABLE_LOADVARS
/////////////////// load vars /////////////////
LoadVarsTask::LoadVarsTask(LoadStates* pls, const String& level0Path, const String& url)
: Task(Task::Id_MovieDataLoad),
pLoadStates(pls), Level0Path(level0Path), Url(url), FileLen(0), Done(0), Succeeded(false)
{
}

void LoadVarsTask::Execute()
{
    URLBuilder::LocationInfo loc(URLBuilder::File_LoadVars, Url, Level0Path);
    String                   fileName;
    pLoadStates->BuildURL(&fileName, loc);

    // File loading protocol
    Ptr<File> pfile;
    Array<UByte> bytes;
    if (URLBuilder::IsProtocol(fileName))
    {
#ifdef SF_ENABLE_HTTP_LOADING
        if (URLBuilder::SendURLRequest(&bytes, fileName) && !bytes.IsEmpty())
        {
            pfile = *SF_NEW MemoryFile(fileName, bytes.GetDataPtr(), (int)bytes.GetSize());
        }
#endif
    }
    else
    {
        pfile = *pLoadStates->OpenFile(fileName.ToCStr());
    }

    if (pfile)
    {
        Succeeded = MovieImpl::ReadTextData(&Data, pfile, &FileLen, false);
    }
    else
    {
        Succeeded = false;
    }

    AtomicOps<unsigned>::Store_Release(&Done, 1);
}
bool LoadVarsTask::GetData(String* data, int* fileLen, bool* succeeded) const
{
    SF_ASSERT(data);
    SF_ASSERT(fileLen);
    unsigned done = AtomicOps<unsigned>::Load_Acquire(&Done);
    if (done != 1)
        return false;

    *data = Data;
    *fileLen = FileLen;
    *succeeded = Succeeded;

    return true;
}

LoadQueueEntryMT_LoadVars::LoadQueueEntryMT_LoadVars(LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot)
: LoadQueueEntryMT(pqueueEntry, pmovieRoot)
{
    pLoadStates = *new LoadStates(pMovieImpl->pMainMovieDef->pLoaderImpl, pMovieImpl->GetStateBagImpl());
    String level0Path;
    pMovieImpl->GetMainMoviePath(&level0Path);
    pTask = *SF_NEW LoadVarsTask(pLoadStates, level0Path,pqueueEntry->URL);
    pMovieImpl->GetTaskManager()->AddTask(pTask);
}

LoadQueueEntryMT_LoadVars::~LoadQueueEntryMT_LoadVars()
{
}

// Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
// Returns true if the movie is loaded completely or in the case of errors.
bool LoadQueueEntryMT_LoadVars::LoadFinished()
{
    String data;
    int       fileLen;
    bool      succeeded;

    bool btaskDone = pTask->GetData(&data,&fileLen,&succeeded);
    // If canceled, wait until task is done
    // [PPS] Change this logic to call AbandonTask and 
    // handle it appropriately inside the task to die
    // instantly
    if (pQueueEntry->Canceled && btaskDone)
        return true;
    if (!btaskDone)
        return false;  
    pMovieImpl->pASMovieRoot->ProcessLoadVarsMT(pQueueEntry, pLoadStates, data, fileLen, succeeded);
    return true;
}


/////////////////// load binary /////////////////
LoadBinaryTask::LoadBinaryTask(LoadStates* pls, const String& level0Path, const String& url)
: Task(Task::Id_MovieDataLoad),
pLoadStates(pls), Level0Path(level0Path), Url(url), FileLen(0), Done(0), Succeeded(false)
{
}

void LoadBinaryTask::Execute()
{
    URLBuilder::LocationInfo loc(URLBuilder::File_LoadVars, Url, Level0Path);
    String                   fileName;
    pLoadStates->BuildURL(&fileName, loc);

    // File loading protocol
    Ptr<File> pfile;
    Array<UByte> bytes;
    if (URLBuilder::IsProtocol(fileName))
    {
#ifdef SF_ENABLE_HTTP_LOADING
        if (URLBuilder::SendURLRequest(&bytes, fileName) && !bytes.IsEmpty())
        {
            pfile = *SF_NEW MemoryFile(fileName, bytes.GetDataPtr(), (int)bytes.GetSize());
        }
#endif
    }
    else
    {
        pfile = *pLoadStates->OpenFile(fileName.ToCStr());
    }
    if (pfile)
    {
        Succeeded = MovieImpl::ReadBinaryData(&Data, pfile, &FileLen);
    }
    else
    {
        Succeeded = false;
    }

    AtomicOps<unsigned>::Store_Release(&Done, 1);
}
bool LoadBinaryTask::GetData(ArrayPOD<UByte>* data, int* fileLen, bool* succeeded) const
{
    SF_ASSERT(data);
    SF_ASSERT(fileLen);
    unsigned done = AtomicOps<unsigned>::Load_Acquire(&Done);
    if (done != 1)
        return false;

    data->Resize(Data.GetSize());
    memcpy(data->GetDataPtr(), Data.GetDataPtr(), Data.GetSize());
    *fileLen = FileLen;
    *succeeded = Succeeded;

    return true;
}

LoadQueueEntryMT_LoadBinary::LoadQueueEntryMT_LoadBinary(LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot)
: LoadQueueEntryMT(pqueueEntry, pmovieRoot)
{
    pLoadStates = *new LoadStates(pMovieImpl->pMainMovieDef->pLoaderImpl, pMovieImpl->GetStateBagImpl());
    String level0Path;
    pMovieImpl->GetMainMoviePath(&level0Path);
    pTask = *SF_NEW LoadBinaryTask(pLoadStates, level0Path,pqueueEntry->URL);
    pMovieImpl->GetTaskManager()->AddTask(pTask);
}

LoadQueueEntryMT_LoadBinary::~LoadQueueEntryMT_LoadBinary()
{
}

// Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
// Returns true if the movie is loaded completely or in the case of errors.
bool LoadQueueEntryMT_LoadBinary::LoadFinished()
{
    ArrayPOD<UByte> data;
    int       fileLen;
    bool      succeeded;

    bool btaskDone = pTask->GetData(&data,&fileLen,&succeeded);
    // If canceled, wait until task is done
    // [PPS] Change this logic to call AbandonTask and 
    // handle it appropriately inside the task to die
    // instantly
    if (pQueueEntry->Canceled && btaskDone)
        return true;
    if (!btaskDone)
        return false;  
    pMovieImpl->pASMovieRoot->ProcessLoadBinaryMT(pQueueEntry, pLoadStates, data, fileLen, succeeded);
    return true;
}
#endif //#ifdef GFX_AS2_ENABLE_LOADVARS

}} // namespace Scaleform::GFx


