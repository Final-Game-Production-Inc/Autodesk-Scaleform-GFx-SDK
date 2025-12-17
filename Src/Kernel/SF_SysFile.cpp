/**************************************************************************

Filename    :   SF_SysFile.cpp
Content     :   File wrapper class implementation (Win32)

Created     :   April 5, 1999
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#define  GFILE_CXX

// Standard C library (Captain Obvious guarantees!)
#include <stdio.h>

#include "SF_SysFile.h"
#include "SF_HeapNew.h"

namespace Scaleform {

// This is - a dummy file that fails on all calls.

class UnopenedFile : public File
{
public:
    UnopenedFile()  { }
    ~UnopenedFile() { }

    virtual const char* GetFilePath()               { return 0; }

    // ** File Information
    virtual bool        IsValid()                   { return 0; }
    virtual bool        IsWritable()                { return 0; }
    //virtual bool      IsRecoverable()             { return 0; }
    // Return position / file size
    virtual int         Tell()                      { return 0; }
    virtual SInt64      LTell()                     { return 0; }
    virtual int         GetLength()                 { return 0; }
    virtual SInt64      LGetLength()                { return 0; }

//  virtual bool        Stat(FileStats *pfs)        { return 0; }
    virtual int         GetErrorCode()              { return Error_FileNotFound; }

    // ** Stream implementation & I/O
    virtual int         Write(const UByte *pbuffer, int numBytes)     { return -1; SF_UNUSED2(pbuffer, numBytes); }
    virtual int         Read(UByte *pbuffer, int numBytes)            { return -1; SF_UNUSED2(pbuffer, numBytes); }
    virtual int         SkipBytes(int numBytes)                       { return 0;  SF_UNUSED(numBytes); }
    virtual int         BytesAvailable()                              { return 0; }
    virtual bool        Flush()                                       { return 0; }
    virtual int         Seek(int offset, int origin)                  { return -1; SF_UNUSED2(offset, origin); }
    virtual SInt64      LSeek(SInt64 offset, int origin)              { return -1; SF_UNUSED2(offset, origin); }

    virtual bool        ChangeSize(int newSize)                       { return 0;  SF_UNUSED(newSize); }
    virtual int         CopyFromStream(File *pstream, int byteSize)   { return -1; SF_UNUSED2(pstream, byteSize); }
    virtual bool        Close()                                       { return 0; }
    virtual bool        CloseCancel()                                 { return 0; }
};



// ***** System File

// System file is created to access objects on file system directly
// This file can refer directly to path

// ** Constructor
SysFile::SysFile() : DelegatedFile(0)
{
    pFile = *new UnopenedFile;
}

#ifdef SF_OS_WII
File* FileWiiDvdOpen(const char *ppath, int flags=File::Open_Read);
#elif defined(SF_OS_3DS)
File* File3DSOpen(const char *ppath, int flags=File::Open_Read);
#elif defined(SF_OS_PSVITA)
File *FileSceIOOpen(const char *ppath, int flags=File::Open_Read);
#elif defined(SF_OS_WIIU)
File *FileWiiUOpen(const String& path, int flags, int mode);
#else
File* FileFILEOpen(const String& path, int flags, int mode);
#endif

// Opens a file
SysFile::SysFile(const String& path, int flags, int mode) : DelegatedFile(0)
{
    Open(path, flags, mode);
}


// ** Open & management
// Will fail if file's already open
bool    SysFile::Open(const String& path, int flags, int mode)
{
#ifdef SF_OS_WII
    pFile = *FileWiiDvdOpen(path.ToCStr());
#elif defined(SF_OS_3DS)
    pFile = *File3DSOpen(path.ToCStr(), flags);
#elif defined(SF_OS_PSVITA)
    pFile = *FileSceIOOpen(path.ToCStr(), flags);
#elif defined(SF_OS_WIIU)
    pFile = *FileWiiUOpen(path.ToCStr(), flags, mode);
#else
    pFile = *FileFILEOpen(path, flags, mode);
#endif
    if ((!pFile) || (!pFile->IsValid()))
    {
        pFile = *SF_NEW UnopenedFile;
        return 0;
    }
#if defined(SF_OS_WII) || defined(SF_OS_WIIU)
    pFile = *SF_NEW BufferedFile(pFile);
#else    
    //pFile = *SF_NEW DelegatedFile(pFile); // MA Testing
    if (flags & Open_Buffered)
        pFile = *SF_NEW BufferedFile(pFile);
#endif
    return 1;
}


// ** Overrides

int     SysFile::GetErrorCode()
{
    return pFile ? pFile->GetErrorCode() : Error_FileNotFound;
}


// Overrides to provide re-open support
bool    SysFile::IsValid()
{
    return pFile && pFile->IsValid();
}
bool    SysFile::Close()
{
    if (IsValid())
    {
        DelegatedFile::Close();
        pFile = *SF_NEW UnopenedFile;
        return 1;
    }
    return 0;
}

/*
bool    SysFile::CloseCancel()
{
    if (IsValid())
    {
        BufferedFile::CloseCancel();
        pFile = *SF_NEW UnopenedFile;
        return 1;
    }
    return 0;
}
*/

} // Scaleform
