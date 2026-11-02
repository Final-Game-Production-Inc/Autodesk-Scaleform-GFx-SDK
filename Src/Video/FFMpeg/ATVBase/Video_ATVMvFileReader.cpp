/**************************************************************************

Filename    :   Video_ATVMvFileReader.cpp
Content     :   Video file reader for the ATV (Analog TV) FFmpeg backend
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/FFMpeg/ATVBase/Video_ATVMvFileReader.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

bool VideoReaderSmp::Create(const char* url)
{
    Destroy();
    if (!pFileOpener || !url)
        return false;

    pFile = *pFileOpener->OpenFile(url);
    return pFile && pFile->IsValid();
}

void VideoReaderSmp::Destroy()
{
    if (pFile)
    {
        pFile->Close();
        pFile = NULL;
    }
}

int VideoReaderSmp::Read(UByte* pbuffer, int numBytes)
{
    if (!pFile || numBytes <= 0)
        return -1;
    return pFile->Read(pbuffer, numBytes);
}

SInt64 VideoReaderSmp::GetSize() const
{
    return pFile ? pFile->LGetLength() : -1;
}

SInt64 VideoReaderSmp::SetPosition(SInt64 offset, int origin)
{
    if (!pFile)
        return -1;
    return pFile->LSeek(offset, origin);
}

SInt64 VideoReaderSmp::GetPosition()
{
    return pFile ? pFile->LTell() : -1;
}

//////////////////////////////////////////////////////////////////////////
//

VideoReader* VideoReaderConfigSmp::CreateReader()
{
    VideoReader* preader = SF_HEAP_NEW(pHeap) VideoReaderSmp(pHeap, pFileOpener);
    if (preader)
        AttachReader(preader);
    return preader;
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
