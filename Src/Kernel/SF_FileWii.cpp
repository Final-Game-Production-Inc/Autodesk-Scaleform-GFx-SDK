/**************************************************************************

Filename    :   SF_FileWii.cpp
Content     :   File using DVD library
Created     :   
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_SysFile.h"
#include "SF_Debug.h"

#include <revolution.h>

namespace Scaleform {

// File class using the dvd library
// Because of restrictions on DVDRead(), it is only useful with BufferedFile

class FileWiiDvd : public File
{
protected:
    // Allocated filename
    String      FileName;

    // File handle & open mode
    bool        Opened;
    DVDFileInfo fs;
    int         Pos, FileSize;
    // Error code for last request
    int         ErrorCode;
    // buffer for unaligned calls
    UByte*      pABuffer;
    UPInt       ABufferSize;

public:

    FileWiiDvd() { Opened = 0; pABuffer = 0; ABufferSize = 0; }
    // Initialize file by opening it
    FileWiiDvd(const String& fileName, int flags, int Mode);

    ~FileWiiDvd()
    {       
        if (Opened)
            Close();
        if (pABuffer)
            SF_FREE(pABuffer);
    }

    virtual const char* GetFilePath();

    // ** File Information
    virtual bool        IsValid();
    virtual bool        IsWritable();
    // Return position / file size  
    virtual int         Tell();
    virtual SInt64      LTell();
    virtual int         GetLength();
    virtual SInt64      LGetLength();

    //  virtual bool        Stat(FileStats *pfs);  
    virtual int         GetErrorCode();         

    // ** GFxStream implementation & I/O
    virtual int         Write(const UByte *pbuffer, int numBytes);
    virtual int         Read(UByte *pbuffer, int numBytes);
    virtual int         SkipBytes(int numBytes);
    virtual int         BytesAvailable();
    virtual bool        Flush();
    virtual int         Seek(int offset, int origin);
    virtual SInt64      LSeek(SInt64 offset, int origin);

    virtual bool        ChangeSize(int newSize);
    virtual int         CopyFromStream(File *pStream, int byteSize);
    virtual bool        Close();
    //virtual bool      CloseCancel();
};


FileWiiDvd::FileWiiDvd(const String& fileName, int flags, int mode)
{   
    if (flags != Open_Read)
    {
        ErrorCode = Error_IOError;
        return;
    }

    SF_UNUSED(mode);
    // Save data
    FileName = fileName;

    Opened = DVDOpen(FileName.ToCStr(), &fs);
    if (!Opened)
        ErrorCode = Error_IOError;
    else
        ErrorCode = 0;
    Pos = 0;
    FileSize = DVDGetLength(&fs);
    pABuffer = 0;
    ABufferSize = 0;
}

const char* FileWiiDvd::GetFilePath()
{
    return FileName.ToCStr();
}


// ** File Information
bool    FileWiiDvd::IsValid()
{
    return Opened;
}
bool    FileWiiDvd::IsWritable()
{
    return 0;
}

// Return position / file size  
int    FileWiiDvd::Tell()
{
    return Pos;
}

SInt64  FileWiiDvd::LTell()
{
    return Pos;
}

int    FileWiiDvd::GetLength()
{
    return FileSize;
}
SInt64  FileWiiDvd::LGetLength()
{
    return GetLength();
}

int    FileWiiDvd::GetErrorCode()
{
    return ErrorCode;
}

int    FileWiiDvd::Write(const UByte *pbuffer, int numBytes)
{
    ErrorCode = Error_IOError;
    return 0;
}

int    FileWiiDvd::Read(UByte *pbuffer, int numBytes)
{
    int read;
    int rPos = Pos & ~3;

    if (numBytes + rPos > FileSize)
    {
        if (((FileSize - rPos + 31) & ~31) <= numBytes)
            numBytes = (FileSize - rPos + 31) & ~31;
        else
            numBytes = FileSize - rPos;
    }
    if (numBytes == 0)
        return 0;

    if (((UPInt)pbuffer & 31) || (numBytes & 31) || (Pos & 3))
    {
        if (ABufferSize < numBytes + 32)
        {
            SF_FREE(pABuffer);
            SF_DEBUG_WARNING(1, "FileWiiDvd::Read: buffer, count, or pos not aligned, allocating extra buffer");
            pABuffer = (UByte*) SF_MEMALIGN(numBytes+32, 32, Stat_Default_Mem);
            ABufferSize = numBytes+32;
        }
        read = DVDRead(&fs, pABuffer, (numBytes + 31) & ~31, rPos);
        memcpy(pbuffer, pABuffer + (Pos & 3), read);
        read -= (Pos & 3);
    }
    else
        read = DVDRead(&fs, pbuffer, numBytes, Pos);
    if (read > numBytes)
        Pos += numBytes;
    else if (read > 0)
        Pos += read;
    else if (read < numBytes)
        ErrorCode = Error_IOError;
    return read;
}

// Seeks ahead to skip bytes
int    FileWiiDvd::SkipBytes(int numBytes)
{   
    Pos += numBytes;
    return Pos;
}

// Return # of bytes till EOF
int    FileWiiDvd::BytesAvailable()
{
    return FileSize-Pos;
}

// Flush file contents
bool    FileWiiDvd::Flush()
{
    return 1;
}

int    FileWiiDvd::Seek(int offset, int origin)
{
    int newOrigin = 0;
    switch(origin)
    {
    case Seek_Set: Pos = offset; break;
    case Seek_Cur: Pos += offset; break;
    case Seek_End: Pos = FileSize + offset; break;
    }
    return Pos;
}

SInt64  FileWiiDvd::LSeek(SInt64 offset, int origin)
{
    return Seek((int)offset,origin);
}

bool    FileWiiDvd::ChangeSize(int newSize)
{
    SF_UNUSED(newSize);
    SF_DEBUG_WARNING(1, "File::ChangeSize not supported");
    ErrorCode = Error_IOError;
    return 0;
}

int    FileWiiDvd::CopyFromStream(File *pstream, int byteSize)
{
    ErrorCode = Error_IOError;
    return 0;
}


bool    FileWiiDvd::Close()
{
    bool closeRet = DVDClose(&fs);

    if (!closeRet)
    {
        ErrorCode = Error_IOError;
        return 0;
    }
    else
    {
        Opened    = 0;
        ErrorCode = 0;
    }   

    return 1;
}

File *FileWiiDvdOpen(const char *ppath, int flags)
{
    return new FileWiiDvd(ppath, flags, 0);
}

// Helper function: obtain file information time.
bool    SysFile::GetFileStat(FileStat* pfileStat, const String& ppath)
{
    SF_UNUSED2(pfileStat, ppath);
    return false;
}

} // Scaleform
