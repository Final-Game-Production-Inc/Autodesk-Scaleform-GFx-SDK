/**************************************************************************

Filename    :   Video_CriMvFileReader.cpp
Content     :   Video file reader
Created     :   July 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_HeapNew.h"

#include "Video/Video_CriMvFileReader.h"

#define THREAD_STACK_SIZE   65536

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

VideoReader* VideoReaderConfigSmp::CreateReader()
{
    VideoReader* preader = SF_HEAP_NEW(GetHeap()) VideoReader(this);
    SF_ASSERT(preader);
    return preader;
}

VideoReader::VideoReader(VideoReaderConfig* preaderconfig) :
    pReaderConfig(preaderconfig), LastReadSize(0)
{
    preaderconfig->AttachReader(this);

    OpenStatus  = CriMvFileReaderInterface::ASYNC_STATUS_STOP;
    ReadStatus  = CriMvFileReaderInterface::ASYNC_STATUS_STOP;
    CloseStatus = CriMvFileReaderInterface::ASYNC_STATUS_STOP;
}

VideoReader::~VideoReader()
{
    pReaderConfig->DetachReader(this);
}

void VideoReader::Open(Char8 *fname)
{
    LastReadSize = 0;
    FileSize = -1;
    ReadPos = 0;
    NextSeekPos = 0;
    ExecuteOpen(fname);
}

void VideoReader::Close(void)
{
    ExecuteClose();
}

void VideoReader::Read(Uint8 *buffer, Sint64 req_size)
{
    while (!pReaderConfig->IsIOEnabled())
        Thread::MSleep(50);

    ExecuteRead(buffer, req_size);
}

void VideoReader::ExecuteOpen(Char8* fname)
{
    pFile = *pReaderConfig->GetFileOpener()->OpenFile(fname);
    if (!pFile || !pFile->IsValid()) 
    {
        // Set all (Open, Close and Read) file IO states to ASYNC_STATUS_ERROR to make
        // sure that CRI will close video player handle later on ExecuteDecode() stage.
        OpenStatus = CriMvFileReaderInterface::ASYNC_STATUS_ERROR;
        CloseStatus = ReadStatus = OpenStatus;
    }
    else
    {
        FileSize = pFile->GetLength();
        OpenStatus = CriMvFileReaderInterface::ASYNC_STATUS_COMPLETE;
    }
}

void VideoReader::ExecuteClose(void)
{
    if (pFile && !pFile->Close())
        CloseStatus = CriMvFileReaderInterface::ASYNC_STATUS_ERROR;
    else
        CloseStatus = CriMvFileReaderInterface::ASYNC_STATUS_COMPLETE;
    pFile = NULL;
}

void VideoReader::ExecuteRead(Uint8* buffer, Sint64 req_size)
{
    if (!pFile || !pFile->IsValid())
    {
        ReadStatus = CriMvFileReaderInterface::ASYNC_STATUS_ERROR;
        return;
    }

    if (!pReaderConfig->IsIOEnabled())
    {
        LastReadSize = 0;
        ReadStatus = CriMvFileReaderInterface::ASYNC_STATUS_COMPLETE;
        return;
    }

    if (ReadPos != NextSeekPos)
    {
        Sint64 newPos = pFile->LSeek(NextSeekPos, File::Seek_Set);
        if (newPos == -1)
        {
            ReadStatus = CriMvFileReaderInterface::ASYNC_STATUS_ERROR;
            return;
        }
        ReadPos = NextSeekPos;
    }

    Sint64 read_size;
    if (ReadPos + req_size > FileSize)
        read_size = FileSize - ReadPos;
    else
        read_size = req_size;

    int res = pFile->Read(buffer, (int)read_size);
    if (res == -1)
    {
        ReadStatus = CriMvFileReaderInterface::ASYNC_STATUS_ERROR;
        return;
    }
    LastReadSize = res;
    ReadPos += LastReadSize;
    NextSeekPos = ReadPos;
    ReadStatus = CriMvFileReaderInterface::ASYNC_STATUS_COMPLETE;
}

Sint64 VideoReader::Seek(Sint64 size, CriMvFileReaderInterface::SeekOrigin offset)
{
    switch (offset)
    {
    default:
    case CriMvFileReaderInterface::SEEK_FROM_BEGIN:
        NextSeekPos = 0;
        break;
    case CriMvFileReaderInterface::SEEK_FROM_CURRENT:
        NextSeekPos = ReadPos;
        break;
    case CriMvFileReaderInterface::SEEK_FROM_END:
        NextSeekPos = FileSize;
        break;
    }

    NextSeekPos += size;
    return NextSeekPos;
}

//////////////////////////////////////////////////////////////////////////
//

#if defined( SF_OS_ORBIS )

static void DefaultInitVideoReaderThrd( ScePthread pid, int requestedPriority )
{
	// Hack, SF thread fails to use SCE_PTHREAD_EXPLICIT_SCHED, so it fails to properly set anything
	// it just gets the calling thread (render threads) scheduling and priority
	// worse it uses the rendering threads mask for I/O

	const SceKernelSchedParam schedParam = { requestedPriority };
	{
		ScePthreadAttr attr;
		scePthreadAttrInit( &attr );
		scePthreadAttrGet( pid, &attr );
		scePthreadAttrSetinheritsched( &attr, SCE_PTHREAD_EXPLICIT_SCHED );
		scePthreadSetschedparam( pid, SCE_KERNEL_SCHED_FIFO, &schedParam ); // try FIFO for being greedier than RR
		scePthreadAttrDestroy( &attr );
	}

	// Use I/O affinity mask
	scePthreadSetaffinity( pid, (1U<<4)|(1U<<5) );
}

typedef void (*FUNC_INIT_VIDEO_READER_THRD)(ScePthread,int);
FUNC_INIT_VIDEO_READER_THRD InitVideoReaderThrd = &DefaultInitVideoReaderThrd;

#endif

VideoReaderConfigThrd::VideoReaderConfigThrd(MemoryHeap* pheap, FileOpenerBase* pfileopener) :
    VideoReaderConfig(pheap, pfileopener)
#ifdef SF_OS_XBOX360
,   ProcNumber(-1)
#endif
{
}

VideoReaderConfigThrd::~VideoReaderConfigThrd()
{
    SF_ASSERT(Readers.GetSize() == 0);
}

VideoReader* VideoReaderConfigThrd::CreateReader()
{
    if (!pThread)
    {
        Thread::CreateParams params(ReadFunc, this, THREAD_STACK_SIZE);
        params.priority = Thread::HighestPriority;
#ifdef SF_OS_XBOX360
        params.processor = ProcNumber;
#endif
        pThread = *SF_HEAP_NEW(GetHeap()) Thread(params);
        if ( pThread->Start() )
		{
            pThread->SetThreadName("Scaleform Video Reader");
#if defined( SF_OS_ORBIS )
			InitVideoReaderThrd( pThread->GetOSHandle(), SCE_KERNEL_PRIO_FIFO_HIGHEST );
#endif
		}
    }
    VideoReaderThrd* preader = SF_HEAP_NEW(GetHeap()) VideoReaderThrd(this);
    return preader;
}

void VideoReaderConfigThrd::DetachReader(VideoReader* preader)
{
    CmdListLock.DoLock();
    Cmd* pcmd = CmdList.GetFirst();
    while (!CmdList.IsNull(pcmd))
    {
        Cmd* ptmp = CmdList.GetNext(pcmd);
        if (pcmd->preader == preader)
        {
            CmdList.Remove(pcmd);
            CmdsHolder.Free(pcmd);
        }
        pcmd = ptmp;
    }
    CmdListLock.Unlock();

    VideoReaderConfig::DetachReader(preader);
    if (Readers.GetSize() == 0)
    {
        if (pThread)
        {
            SubmitCmd(TermCmd);
            pThread->Wait();
            pThread = 0;
        }
    }
}

void VideoReaderConfigThrd::EnableIO(bool enable)
{
    VideoReaderConfig::EnableIO(enable);
    Event.PulseEvent();
    Mutex::Locker lock(&CmdListLock);
    while (!CmdList.IsEmpty())
        IOFinished.Wait(&CmdListLock);
}

void VideoReaderConfigThrd::SetReadCallback(ReadCallback* pcallback)
{
    Mutex::Locker lock(&CmdListLock);
    pReadCallback = pcallback;
}

int VideoReaderConfigThrd::ReadFunc(Thread* pthread, void *h)
{
    SF_UNUSED(pthread);

    VideoReaderConfigThrd* preaderConfig = (VideoReaderConfigThrd*)h;
    Cmd* pcmd;

    bool bExecuteIO = true;
    while (bExecuteIO)
    {
        preaderConfig->Event.Wait();

        preaderConfig->CmdListLock.DoLock();
        if (preaderConfig->pReadCallback)
            preaderConfig->pReadCallback->OnReadRequested();
        preaderConfig->CmdListLock.Unlock();

        while (1)
        {
            {
            Mutex::Locker lock(&preaderConfig->CmdListLock);
            if (preaderConfig->CmdList.IsEmpty())
                break;
            pcmd = preaderConfig->CmdList.GetFirst();
            preaderConfig->CmdList.Remove(pcmd);
            }

            switch(pcmd->id)
            {
            case OpenCmd:
                pcmd->preader->ExecuteOpen(pcmd->fname);
                break;
            case CloseCmd:
                pcmd->preader->ExecuteClose();
                break;
            case ReadCmd:
                pcmd->preader->ExecuteRead(pcmd->buffer, pcmd->req_size);
                break;
            case TermCmd:
                bExecuteIO = false;
                break;
            }

            Mutex::Locker lock(&preaderConfig->CmdListLock);
            preaderConfig->CmdsHolder.Free(pcmd);
        }

        preaderConfig->CmdListLock.DoLock();
        if (preaderConfig->pReadCallback)
            preaderConfig->pReadCallback->OnReadCompleted();
        preaderConfig->IOFinished.Notify();
        preaderConfig->CmdListLock.Unlock();
    }

    return 0;
}

void VideoReaderConfigThrd::SubmitCmd(CmdId id, VideoReaderThrd* preader, 
                                      Char8* fname, Uint8* buffer, Sint64 req_size)
{
    CmdListLock.DoLock();
    Cmd* cmd = CmdsHolder.Alloc();
    cmd->id = id;
    cmd->preader = preader;
    cmd->fname = fname;
    cmd->buffer = buffer;
    cmd->req_size = req_size;
    CmdList.PushBack(cmd);
    CmdListLock.Unlock();
    Event.PulseEvent();
}

//////////////////////////////////////////////////////////////////////////
//

void VideoReaderThrd::Open(Char8 *fname)
{
    OpenStatus  = CriMvFileReaderInterface::ASYNC_STATUS_BUSY;
    ReadStatus  = CriMvFileReaderInterface::ASYNC_STATUS_STOP;
    CloseStatus = CriMvFileReaderInterface::ASYNC_STATUS_STOP;

    LastReadSize = 0;
    FileSize = -1;
    ReadPos = 0;
    NextSeekPos = 0;
    GetReaderConfigThrd()->SubmitCmd(VideoReaderConfigThrd::OpenCmd, this, fname);
}

void VideoReaderThrd::Close(void)
{
    CloseStatus = CriMvFileReaderInterface::ASYNC_STATUS_BUSY;
    GetReaderConfigThrd()->SubmitCmd(VideoReaderConfigThrd::CloseCmd, this);
}

void VideoReaderThrd::Read(Uint8 *buffer, Sint64 req_size)
{
    ReadStatus = CriMvFileReaderInterface::ASYNC_STATUS_BUSY;
    GetReaderConfigThrd()->SubmitCmd(VideoReaderConfigThrd::ReadCmd, this, 0, buffer, req_size);
}

Sint64 VideoReaderThrd::Seek(Sint64 size, CriMvFileReaderInterface::SeekOrigin offset)
{
    GetReaderConfigThrd()->Lock();
    Sint64 ret = VideoReader::Seek(size, offset);
    GetReaderConfigThrd()->Unlock();
    return ret;
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
