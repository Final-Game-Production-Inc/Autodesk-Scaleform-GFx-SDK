/**************************************************************************

Filename    :   Video_ATVMvFileReader.h
Content     :   Video file reader for the ATV (Analog TV) FFmpeg backend
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_ATV_VIDEO_MVFILEREADER_H
#define INC_GFX_ATV_VIDEO_MVFILEREADER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_File.h"

#include "Video/Common/Video_Video.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
// VideoReader
//
// Generic movie-file reader interface. The ATV (Analog TV) FFmpeg backend
// implements it on top of the Scaleform FileOpener so that every movie file
// - including files stored inside virtual file systems (e.g. ZIP archives) -
// can be handed to libavformat through a custom AVIOContext.
//
// The interface is defined here because the legacy SDK only forward-declares
// VideoReader in Video/Common/Video_Video.h and the CRI backend (which does
// its file I/O inside CRI Mana) never needed a definition.

class VideoReader : public RefCountBaseNTS<VideoReader, Stat_Video_Mem>
{
public:
    VideoReader(MemoryHeap* pheap, FileOpenerBase* pfileopener) :
        pHeap(pheap), pFileOpener(pfileopener), pFile(NULL) {}
    virtual ~VideoReader() {}

    // Opens the movie file through the configured FileOpener.
    virtual bool    Create(const char* url) = 0;
    virtual void    Destroy() = 0;

    // File I/O. Read returns the number of bytes read (0 at EOF, -1 on error).
    virtual int     Read(UByte* pbuffer, int numBytes) = 0;
    virtual SInt64  GetSize() const = 0;
    virtual SInt64  SetPosition(SInt64 offset, int origin = File::Seek_Set) = 0;
    virtual SInt64  GetPosition() = 0;

    MemoryHeap*     GetHeap()       { return pHeap; }
    FileOpenerBase* GetFileOpener() { return pFileOpener; }

protected:
    MemoryHeap*         pHeap;
    Ptr<FileOpenerBase> pFileOpener;
    Ptr<File>           pFile;
};

//////////////////////////////////////////////////////////////////////////
// VideoReaderSmp - single-threaded reader backed by FileOpener/File

class VideoReaderSmp : public VideoReader
{
public:
    VideoReaderSmp(MemoryHeap* pheap, FileOpenerBase* pfileopener) :
        VideoReader(pheap, pfileopener) {}
    virtual ~VideoReaderSmp() { Destroy(); }

    virtual bool    Create(const char* url);
    virtual void    Destroy();
    virtual int     Read(UByte* pbuffer, int numBytes);
    virtual SInt64  GetSize() const;
    virtual SInt64  SetPosition(SInt64 offset, int origin);
    virtual SInt64  GetPosition();
};

//////////////////////////////////////////////////////////////////////////
// VideoReaderConfigSmp - creates VideoReaderSmp instances for the ATV
// video system. The engine hands this config to the player through
// Video::GetReaderConfig() when GFX_ENABLE_FFMPEGVIDEO is defined.

class VideoReaderConfigSmp : public VideoReaderConfig
{
public:
    VideoReaderConfigSmp(MemoryHeap* pheap, FileOpenerBase* pfileopener) :
        VideoReaderConfig(pheap, pfileopener) {}
    ~VideoReaderConfigSmp() {}

    virtual VideoReader* CreateReader();
    virtual void         SetReadCallback(ReadCallback*) {}
    virtual void         EnableIO(bool enable) { VideoReaderConfig::EnableIO(enable); }
    virtual void         DetachReader(VideoReader* preader) { VideoReaderConfig::DetachReader(preader); }
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_ATV_VIDEO_MVFILEREADER_H
