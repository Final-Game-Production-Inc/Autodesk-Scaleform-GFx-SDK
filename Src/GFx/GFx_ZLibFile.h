/**************************************************************************

Filename    :   GFx_ZLibFile.h
Content     :   Header for z-lib wrapped file input 
Created     :   June 24, 2005
Authors     :   Michael Antonov

Notes       :   ZLibFile is currently Read Only

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_ZLibFile_H
#define INC_SF_GFX_ZLibFile_H

#include "Kernel/SF_File.h"

struct z_stream_s;

namespace Scaleform { namespace GFx {

// ZLib functionality is only available if SF_ENABLE_ZLIB is defined
#ifdef SF_ENABLE_ZLIB

class ZLibFileImpl;

class ZLibFile : public File
{
    friend class ZLibFileImpl;
    friend class ZlibSupport;

    class ZLibFileImpl   *pImpl;
    
    // ZLib Helper: Initializes z_stream by setting allocators and calling inflateInit.
    //  - pallocOwner is used as 'this' for GALLOC_AUTO_HEAP.
    //  - pbuffer, buffer size are used to initialize the output buffer.
    // Returns zlib error code, which is Z_OK for success.
    static int    ZLib_InitStream(z_stream_s* pstream, void* pallocowner,
                                  void* pbuffer = 0, unsigned int bufferSize = 0);

public:

    // ZLibFile must be constructed with a source file 
    ZLibFile(File *psourceFile = 0);
    ~ZLibFile();

    // ** File Information
    virtual const char* GetFilePath();

    // Return 1 if file's usable (open)
    virtual bool        IsValid();
    // Return 0; ZLib files are not writable for now
    virtual bool        IsWritable() { return 0; }

    // Return position
    // Position position is reported in relation to the compressed stream, NOT the source file
    virtual int         Tell ();
    virtual SInt64      LTell ();
    virtual int         GetLength ();
    virtual SInt64      LGetLength ();
    // Return errno-based error code
    virtual int         GetErrorCode();

    // ** Stream implementation & I/O
    
    virtual int         Write(const UByte *pbufer, int numBytes);  
    virtual int         Read(UByte *pbufer, int numBytes);
    virtual int         SkipBytes(int numBytes);           
    virtual int         BytesAvailable();
    virtual bool        Flush();
    // Returns new position, -1 for error
    // Position seeking works in relation to the compressed stream, NOT the source file
    virtual int         Seek(int offset, int origin=SEEK_SET);
    virtual SInt64      LSeek(SInt64 offset, int origin=SEEK_SET);
    // Writing not supported..
    virtual bool        ChangeSize(int newSize);       
    virtual int         CopyFromStream(File *pstream, int byteSize);
    // Closes the file  
    virtual bool        Close();    
};

#endif // SF_ENABLE_ZLIB

}} //namespace Scaleform { namespace GFx {

#endif // INC_SF_GFX_ZLibFile_H
