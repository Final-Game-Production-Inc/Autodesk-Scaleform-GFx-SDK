/**************************************************************************

Filename    :   GameMemoryFileCache.h
Content     :   Sample memory file manager for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameMemoryFileCache_H
#define INC_GameMemoryFileCache_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_String.h"

using namespace Scaleform;

// ********************************************************************
// A memory file
// - Used to load file data into memory on loading/asynch tasks
//
class GameMemoryFile : public RefCountBase<GameMemoryFile, Stat_Default_Mem>
{
    friend class GameMemoryFileCache;
public:
    virtual ~GameMemoryFile();

    UByte*      GetData() const     { return pData; }
    int         GetLength() const   { return Length; }

    void        WriteData(unsigned offset, UByte* in, unsigned inSz);

private:
    UByte*      pData;
    int         Length;

    // Private ctor
    GameMemoryFile(int dataSz);
};


// ********************************************************************
// Memory file cache
// - The file data is accessible by their filenames/paths
//
class GameMemoryFileCache : public RefCountBase<GameMemoryFileCache, Stat_Default_Mem>
{
    typedef     Hash<String, Ptr<GameMemoryFile>, String::HashFunctor>    MemoryFileHash;

public:
    GameMemoryFileCache() {}

    GameMemoryFile*    GetFile(const char* filename);

    GameMemoryFile*     CreateFile(const char* filename, int dataSz);
    
    // Add memory files to the file cache
    void    AddSet(const GameMemoryFileCache* src);

    // Remove memory files from the file cache
    void    Remove(const char* pfilename);
    void    RemoveSet(const GameMemoryFileCache* src);

private:
    MemoryFileHash      FileCache;
};

#endif  // INC_GameMemoryFileCache_H
