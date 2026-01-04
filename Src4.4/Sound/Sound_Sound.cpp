/**************************************************************************

Filename    :   Sound_Sound.cpp
Content     :   Sound samples
Created     :   November, 2008
Authors     :   Andrew Reisse, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "Sound_Sound.h"

#include "Kernel/SF_Alg.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Std.h"
#include "Kernel/SF_AmpInterface.h"

#define STREAM_SOUND_DATA_CHUNK_SIZE 16384

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

SoundData::SoundData(unsigned format, unsigned rate, unsigned length, unsigned dsize) :
    SoundDataBase(format, rate, length), pData(NULL), DataSize(dsize)
{
    pData = (UByte *)SF_MEMALIGN(DataSize, 32, Stat_Sound_Mem);
    Alg::MemUtil::Set(pData, 0, DataSize);
#if defined(SF_AMP_SERVER) && !defined(SF_MEMORY_ENABLE_DEBUG_INFO)
    AmpServer::GetInstance().AddSound(DataSize);
#endif
}

SoundData::~SoundData()
{
#if defined(SF_AMP_SERVER) && !defined(SF_MEMORY_ENABLE_DEBUG_INFO)
    AmpServer::GetInstance().RemoveSound(DataSize);
#endif
    SF_FREE_ALIGN(pData); 
}

//////////////////////////////////////////////////////////////////////////
//

AppendableSoundData::DataChunk::DataChunk(unsigned capacity) : pNext(NULL),
    DataSize(0), StartSample(0), SampleCount(0)
{
    pData = (UByte *)SF_MEMALIGN(capacity, 32, Stat_Sound_Mem);
    Alg::MemUtil::Set(pData,0,capacity);
}

AppendableSoundData::DataChunk::~DataChunk()
{
    SF_FREE_ALIGN(pData);
}

AppendableSoundData::AppendableSoundData(unsigned format, unsigned rate) : SoundDataBase(format, rate, 0),
    pFirstChunk(NULL), pFillChunk(NULL), pReadChunk(NULL), DataSize(0), ReadPos(0)
{
    Format |= SoundDataBase::Sample_Stream;
}

AppendableSoundData::~AppendableSoundData()
{
    while(pFirstChunk)
    {
        DataChunk* ptmp = pFirstChunk->pNext;
        delete pFirstChunk;
        pFirstChunk = ptmp;
    }
}

UByte* AppendableSoundData::LockDataForAppend(unsigned length, unsigned dsize)
{
    ChunkLock.DoLock();
    SF_ASSERT(dsize < STREAM_SOUND_DATA_CHUNK_SIZE);
    if (!pFirstChunk)
    {
        pFirstChunk = SF_NEW DataChunk(STREAM_SOUND_DATA_CHUNK_SIZE);
        pFillChunk = pFirstChunk;
        pReadChunk = pFillChunk;
    }
    UByte* pbuffer = 0;
    if (dsize > STREAM_SOUND_DATA_CHUNK_SIZE - pFillChunk->DataSize)
    {
        pFillChunk->pNext = SF_NEW DataChunk(STREAM_SOUND_DATA_CHUNK_SIZE);
        pFillChunk->pNext->StartSample = pFillChunk->StartSample + pFillChunk->SampleCount;
        pFillChunk = pFillChunk->pNext;
    }
    pbuffer = pFillChunk->pData + pFillChunk->DataSize;
    pFillChunk->DataSize += dsize;
    pFillChunk->SampleCount += length;
    DataSize += dsize;
    Length += length;
    return pbuffer;
}

void AppendableSoundData::UnlockData()
{
    ChunkLock.Unlock();
}

unsigned AppendableSoundData::GetData(UByte* buff, unsigned dsize)
{
    Lock::Locker lock(&ChunkLock);
    if (!pReadChunk)
        return 0;

    unsigned got_bytes = 0;
    while(dsize > 0)
    {
        unsigned ds = dsize;
        if (ReadPos + dsize > pReadChunk->DataSize)
        {
            ds = pReadChunk->DataSize - ReadPos;
            if (ds == 0)
            {
                if (!pReadChunk->pNext)
                    return got_bytes;
                pReadChunk = pReadChunk->pNext;
                ReadPos = 0;
                continue;
            }
        }
        Alg::MemUtil::Copy(buff+got_bytes, pReadChunk->pData + ReadPos, ds);
        dsize -= ds;
        ReadPos += ds;
        got_bytes += ds;            
    }
    return got_bytes;
}

bool AppendableSoundData::SeekPos(unsigned pos)
{
    Lock::Locker lock(&ChunkLock);
    if (!pReadChunk)
        return false;

    unsigned lpos = 0;
    pReadChunk = pFirstChunk;
    while(1)
    {
        lpos += pReadChunk->DataSize;
        if (pos < lpos)
        {
            ReadPos = pos - (lpos - pReadChunk->DataSize);
            return true;
        }
        if (!pReadChunk->pNext)
            break;
        pReadChunk = pReadChunk->pNext;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
//

SoundFile::SoundFile(const char* fname, unsigned rate, unsigned length, bool streaming) :
    SoundDataBase(SoundDataBase::Sample_File, rate, length)
{
    if (streaming)
        Format |= SoundDataBase::Sample_Stream;
    UPInt len = SFstrlen(fname) + 1;
    pFileName = (char *)SF_ALLOC(len, Stat_Sound_Mem);
    Alg::MemUtil::Set(pFileName, 0, len);

    SFstrcpy(pFileName, len, fname);
}

SoundFile::~SoundFile()
{
    SF_FREE(pFileName);
}

}} // Scaleform::Sound

#endif // GFX_ENABLE_SOUND
