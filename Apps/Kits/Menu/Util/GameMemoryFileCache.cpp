/**************************************************************************

Filename    :   GameMemoryFileCache.cpp
Content     :   Sample resource manager for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameMemoryFileCache.h"
#include "Kernel/SF_HeapNew.h"


GameMemoryFile::GameMemoryFile(int dataSz) : pData(NULL), Length(0)
{
    pData = (UByte*)SF_ALLOC(dataSz, Stat_Default_Mem);
    SF_ASSERT(pData);
    Length = dataSz;
}

GameMemoryFile::~GameMemoryFile()
{
    if (pData) SF_FREE(pData);
}

void GameMemoryFile::WriteData(unsigned offset, UByte* in, unsigned inSz)
{
    SF_ASSERT(pData);
    SF_ASSERT(int(offset + inSz) <= Length);
    memcpy(&pData[offset], in, inSz);
}

GameMemoryFile*    GameMemoryFileCache::GetFile(const char* filename)
{
    SF_ASSERT(filename);
    Ptr<GameMemoryFile>* pfile = FileCache.Get(filename);
    if (pfile) return *pfile;
    return NULL;
}

GameMemoryFile*     GameMemoryFileCache::CreateFile(const char* filename, int dataSz)
{
    Ptr<GameMemoryFile> pfile = SF_HEAP_AUTO_NEW(this) GameMemoryFile(dataSz);
    FileCache.Add(filename, pfile);
    return pfile;
}

// Add memory files to the file cache
void    GameMemoryFileCache::AddSet(const GameMemoryFileCache* src)
{
    SF_ASSERT(src);
    for (MemoryFileHash::ConstIterator iter = src->FileCache.Begin(); iter != src->FileCache.End(); ++iter)
    {
        FileCache.Add(iter->First, iter->Second);
    }
}

// Remove memory files from the file cache
void    GameMemoryFileCache::Remove(const char* pfilename)
{
    SF_ASSERT(pfilename);
    FileCache.Remove(pfilename);
}
void    GameMemoryFileCache::RemoveSet(const GameMemoryFileCache* src)
{
    SF_ASSERT(src);
    for (MemoryFileHash::ConstIterator iter = src->FileCache.Begin(); iter != src->FileCache.End(); ++iter)
    {
        FileCache.Remove(iter->First);
    }
}


