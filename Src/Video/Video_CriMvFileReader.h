/**************************************************************************

Filename    :   Video_CriMvFileReader.h
Content     :   Video file reader
Created     :   July 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_CRIMVFILEREADER_H
#define INC_GFX_VIDEO_CRIMVFILEREADER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
#include <cri_movie.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_ListAlloc.h"
#include "Kernel/SF_Threads.h"

#include "GFx/GFx_Loader.h"
#include "Video/Video_Video.h"

namespace Scaleform { namespace GFx { namespace Video {

class VideoReader;
class VideoReaderThrd;

//////////////////////////////////////////////////////////////////////////
//

class VideoReaderConfigSmp : public VideoReaderConfig
{
public:
    VideoReaderConfigSmp(MemoryHeap* pheap, FileOpenerBase* pfileopener) :
        VideoReaderConfig(pheap, pfileopener) {}
    ~VideoReaderConfigSmp() {}

    virtual VideoReader* CreateReader();
};

//////////////////////////////////////////////////////////////////////////
//

class VideoReader : public RefCountBaseNTS<VideoReader,Stat_Video_Mem>,
                    public CriMvFileReaderInterface
{
public:
    VideoReader(VideoReaderConfig* pfileopener);
    virtual ~VideoReader();

    virtual void   Open(Char8 *fname);
    virtual void   Close(void);
    virtual void   Read(Uint8 *buffer, Sint64 req_size);
    virtual Sint64 Seek(Sint64 size, CriMvFileReaderInterface::SeekOrigin offset);

    virtual Sint64 GetFileSize(void) { return FileSize; }
    virtual Sint64 GetReadSize(void) { return LastReadSize; }

    virtual CriMvFileReaderInterface::AsyncStatus GetOpenStatus(void)  { return OpenStatus; }
    virtual CriMvFileReaderInterface::AsyncStatus GetCloseStatus(void) { return CloseStatus; }
    virtual CriMvFileReaderInterface::AsyncStatus GetReadStatus(void)  { return ReadStatus; }

protected:
    Ptr<VideoReaderConfig> pReaderConfig;
    Ptr<File>              pFile;
    int                    LastReadSize;

    CriMvFileReaderInterface::AsyncStatus OpenStatus;
    CriMvFileReaderInterface::AsyncStatus CloseStatus;
    CriMvFileReaderInterface::AsyncStatus ReadStatus;

    Sint64 NextSeekPos;
    Sint64 ReadPos;
    Sint64 FileSize;

    void ExecuteOpen(Char8* fname);
    void ExecuteClose(void);
    void ExecuteRead(Uint8* buffer, Sint64 req_size);
};

//////////////////////////////////////////////////////////////////////////
//

class VideoReaderConfigThrd : public VideoReaderConfig
{
public:
    VideoReaderConfigThrd(MemoryHeap* pheap, FileOpenerBase* pfileopener);
    ~VideoReaderConfigThrd();

    virtual VideoReader* CreateReader();

    virtual void SetReadCallback(ReadCallback*);
    virtual void EnableIO(bool enable);

#ifdef SF_OS_XBOX360
    virtual void SetUsableProcessors(int proc) { ProcNumber = proc; }
#endif

    virtual void DetachReader(VideoReader*);

    enum CmdId
    {
        OpenCmd,
        ReadCmd,
        CloseCmd,
        TermCmd
    };

    void SubmitCmd(CmdId id, VideoReaderThrd* preader = 0, 
                   Char8* fname = 0, Uint8* buffer = 0, Sint64 req_size = 0);

    void Lock()   { CmdListLock.DoLock(); }
    void Unlock() { CmdListLock.Unlock(); }

    struct Cmd : public ListNode<Cmd>
    {
        CmdId               id;
        VideoReaderThrd*    preader;
        Char8*              fname;
        Uint8*              buffer;
        Sint64              req_size;
    };

private:
    ListAllocPOD<Cmd,127,Stat_Video_Mem> CmdsHolder;
    List<Cmd>               CmdList;
    Mutex                   CmdListLock;
    Scaleform::Event        Event;
    Ptr<Thread>             pThread;
    WaitCondition           IOFinished;
#ifdef SF_OS_XBOX360
    int                     ProcNumber;
#endif
    Ptr<ReadCallback>       pReadCallback;

    static int ReadFunc(Thread* pthread, void *h);
};

//////////////////////////////////////////////////////////////////////////
//

class VideoReaderThrd : public VideoReader
{
public:
    VideoReaderThrd(VideoReaderConfigThrd* preaderconfig) : VideoReader(preaderconfig) {}
    virtual ~VideoReaderThrd() {}

    virtual void   Open(Char8 *fname);
    virtual void   Close(void);
    virtual void   Read(Uint8 *buffer, Sint64 req_size);
    virtual Sint64 Seek(Sint64 size, CriMvFileReaderInterface::SeekOrigin offset);
  
private:
    friend class VideoReaderConfigThrd;

    VideoReaderConfigThrd* GetReaderConfigThrd() {
        return (VideoReaderConfigThrd*)pReaderConfig.GetPtr();
    }
};

}}} // Scaleform::GFx::Video

#endif // GFC_NO_VIDEO

#endif // INC_GFX_VIDEO_CRIMVFILEREADER_H
