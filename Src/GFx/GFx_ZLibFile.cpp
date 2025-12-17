/**************************************************************************

Filename    :   GFx_ZLibFile.cpp
Content     :   
Created     :   April 5, 1999
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_ZLibFile.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"

#ifdef SF_ENABLE_ZLIB

#include <zlib.h>

namespace Scaleform { namespace GFx {

// This flag can be set to verify that read data after seeks and Resets
// always matches the actual decompressed stream. We've seen this fail
// due to EAX2/DirectSound corrupting memory under FMOD with XP64 (32-bit)
// and Realtek HA Audio driver.
//#define SF_LIBFILE_VERIFY_DATA


// ***** File interface

// Internal class used to store data necessary for deflation
class ZLibFileImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    Ptr<File> pIn;
    
    z_stream    ZStream;
    int         InitialStreamPos;   // position of the input stream where we started inflating.
    int         LogicalStreamPos;   // current stream position of uncompressed data.
    bool        AtEofFlag;
    int         ErrorCode;

    enum ZLibConstants
    {
        ZLib_BuffSize       = 4096,
#ifdef SF_LIBFILE_VERIFY_DATA
        ZLib_BacktrackSize  = 256
#else
        // ZLib_BacktrackSize should be as big as JPEG_BufferSize in GJPEGUtil_jpeglib.cpp
        // to avoid expensive z-lib buffer restarts.
        // It also shouldn't be less than Zlib_BuffSize for zlib-packed images (!AB)
        ZLib_BacktrackSize  = 4096
#endif
    };

    // User position in file, can be < then LogicalStreamPos.
    int     UserPos;
    int     BacktrackTail; // Pointer to last byte of buffer + 1.
                           // Data can warp around circularly, spanning
                           // range from [BacktrackTail, BacktrackTail-1].
    int     BacktrackSize; // Number of usable bytes in backtrack buffer.

    // Last bytes output - circular buffer used to allow cheap seek-back.
    UByte   BacktrackBuffer[ZLib_BacktrackSize];
    // Data buffer used by ZLib for input.
    UByte   DataBuffer[ZLib_BuffSize];
    
#ifdef SF_LIBFILE_VERIFY_DATA
    UByte* pHugeBuffer;
    int    HugeSize;
#endif

    // Constructor.
    ZLibFileImpl(File* pin)       
    {       
        // Init file pointer and other values
        pIn                 = pin;
        InitialStreamPos    = pIn->Tell();
        LogicalStreamPos    = 0;
        AtEofFlag           = 0;
        ErrorCode           = 0;
        
        // Initialize ZStream to our allocator and no I/O data for now.
        if (ZLibFile::ZLib_InitStream(&ZStream, this) != Z_OK)
        {
            //log_error("Error: ZLibFileImpl::ctor() inflateInit() returned %d\n", err);
            ErrorCode = 1;
            return;
        }

        // No backtrack data available yet.
        BacktrackSize = 0;
        BacktrackTail= 0;
        UserPos       = 0;

#ifdef SF_LIBFILE_VERIFY_DATA
        pHugeBuffer = (UByte*)SF_ALLOC(1024*1024*8, Stat_Default_Mem);
        HugeSize    = Inflate(pHugeBuffer, 1024*1024*8);
        Reset();
#endif        
    }

    ~ZLibFileImpl()
    {
#ifdef SF_LIBFILE_VERIFY_DATA
        SF_FREE(pHugeBuffer);
#endif
    }


    // Discard current results and rewind to the beginning.
    // Necessary in order to seek backwards.    
    void    Reset()
    {
        ErrorCode = 0;
        AtEofFlag = 0;
        int err = inflateReset(&ZStream);
        if (err != Z_OK)
        {
            ErrorCode = 1;
            return;
        }

        ZStream.next_in     = 0;
        ZStream.avail_in    = 0;
        ZStream.next_out    = 0;
        ZStream.avail_out   = 0;

        // Rewind the underlying stream.
        pIn->Seek(InitialStreamPos);

        LogicalStreamPos = 0;

        BacktrackSize = 0;
        BacktrackTail= 0;
        UserPos       = 0;
    }


    // General reading.
    // Inflate while updating backtrack buffer.
    // Can also be used to read data after backtrack.
    int     Inflate(void* dst, int bytes)
    {
        int  bytesOutput = 0;

#ifdef SF_LIBFILE_VERIFY_DATA
        UByte * porigDst = (UByte*)dst;
        unsigned   copySize1 = 0, copySize2 = 0;
        unsigned    tailSpace1 = 0;
#endif

        // Check the backtrack part of the stream and read/copy it.
        if (UserPos < LogicalStreamPos)
        {
            // Data must be in buffer (otherwise there is a bug in SetPosition).
            SF_ASSERT(UserPos >= (LogicalStreamPos - BacktrackSize));
            SF_ASSERT(BacktrackTail < (ZLib_BacktrackSize+1));

            // Determine how many bytes we will take from backtrack buffer,
            // and staring where (backtrackDataPos counts back from current pointer).
            int backtrackDataPos = LogicalStreamPos - UserPos;            
            int backtrackData    = (backtrackDataPos > bytes) ? bytes : backtrackDataPos;
            int backtrackDataSave= backtrackData;
            
            if (backtrackDataPos > BacktrackTail)
            {
                // Wrap-around can only happen once we have full buffer.
                SF_ASSERT(BacktrackSize == ZLib_BacktrackSize);

                // Need to grab data from tail first.
                int    copyOffset = BacktrackSize - (backtrackDataPos - BacktrackTail);
                int    copySize   = BacktrackSize - copyOffset;
                if (copySize > backtrackData)
                    copySize = backtrackData;

#ifdef SF_LIBFILE_VERIFY_DATA
                copySize1 = copySize;
#endif

                memcpy(dst, BacktrackBuffer + copyOffset, copySize);
                backtrackData -= copySize;
                backtrackDataPos -= copySize;
                dst = ((UByte*)dst) + copySize;
            }

            // The rest of the data comes from the first part before the start
            if (backtrackData > 0)
            {
                SF_ASSERT(backtrackDataPos <= BacktrackTail);
                memcpy(dst, BacktrackBuffer + (BacktrackTail - backtrackDataPos), backtrackData);
                dst = ((UByte*)dst) + backtrackData;

#ifdef SF_LIBFILE_VERIFY_DATA
                copySize2 = backtrackData;
#endif
            }

            bytes       -= backtrackDataSave;
            bytesOutput += backtrackDataSave;
            UserPos     += backtrackDataSave;
        }

        // Read the data from ZLib.
        if (bytes > 0)
        {
            int bytesRead = InflateFromStream(dst, bytes);

            // If the request would not fit in its entirety in backtrack
            // buffer, remember its last chunk.
            if (bytesRead >= ZLib_BacktrackSize)
            {
                BacktrackTail = ZLib_BacktrackSize;
                BacktrackSize = ZLib_BacktrackSize;
                memcpy(BacktrackBuffer, ((UByte*)dst) + (bytesRead - BacktrackSize), BacktrackSize);
            }

            else if (bytesRead > 0)
            {
                UByte *psrc = (UByte*)dst;

                // Read the piece into the tail of the buffer.
                int tailSpace= ZLib_BacktrackSize - BacktrackTail;
                int copySize = (tailSpace < bytesRead) ? tailSpace : bytesRead;

#ifdef SF_LIBFILE_VERIFY_DATA
                tailSpace1 = tailSpace;
#endif

                if (copySize > 0)
                {
                    memcpy(BacktrackBuffer + BacktrackTail, psrc, copySize);
                    psrc += copySize;
                    BacktrackTail += copySize;
                }

                // If there is mode data, do wrap-around to beginning of buffer.
                if (bytesRead > copySize)
                {
                    BacktrackTail = bytesRead - copySize;
                    memcpy(BacktrackBuffer, psrc, BacktrackTail);
                }
                
                // Once maximum backtrack size is reached, it doesn't change.
                if (BacktrackSize < ZLib_BacktrackSize)
                {
                    BacktrackSize += bytesRead;
                    if (BacktrackSize > ZLib_BacktrackSize)
                        BacktrackSize = ZLib_BacktrackSize;
                }
            }

            UserPos = LogicalStreamPos;
            bytesOutput += bytesRead;
        }
        
#ifdef SF_LIBFILE_VERIFY_DATA
        if (UserPos < HugeSize) 
        {
            //SF_ASSERT(!memcmp(pHugeBuffer + UserPos - bytesOutput, porigDst, bytesOutput));
            UByte* pcompareBuffer = pHugeBuffer + UserPos - bytesOutput;

            for (int i = 0; i < bytesOutput; i++)
            {
                SF_ASSERT(pcompareBuffer[i] == porigDst[i]);
            }
        }
#endif

        return bytesOutput;
    }



    // Seek to the target position.
    int    SetPosition(int offset)
    {        
        if (offset < LogicalStreamPos)
        {
            // If we can seek within a backtrack buffer, do so.
            if (offset >= (LogicalStreamPos - BacktrackSize))
            {
                UserPos = offset;
                return UserPos;
            }

            // MA DEBUG
            SF_DEBUG_MESSAGE1(1, "ZLibFileImpl::SetPosition(%d) - Restarting\n", offset);

            // Otherwise we must re-read.
            Reset();
        }
        else if (offset > LogicalStreamPos)
        {
            //GFC_DEBUG_MESSAGE1(1, "ZLibFileImpl::SetPosition(%d) - Seeking forward", offset);
            UserPos = LogicalStreamPos;
        }

        UByte   temp[ZLibFileImpl::ZLib_BuffSize];
        
        // Now seek forwards, by just reading data in blocks.
        while (UserPos < offset)
        {
            int    toRead = offset - UserPos;
            int    toReadThisTime = Alg::Min<int>(toRead, ZLibFileImpl::ZLib_BuffSize);
            //assert(toReadThisTime > 0);

            int    bytesRead = Inflate(temp, toReadThisTime);
            SF_ASSERT(bytesRead <= toReadThisTime);

            // Trouble; can't seek any further.
            if (bytesRead == 0)
                break;
        }

        // Return new location.
        return UserPos;
    }



    // Decompress from ZLib stream into specified buffer.
    int    InflateFromStream(void* dst, int bytes)
    {
        if (ErrorCode)          
            return 0;           

        ZStream.next_out = (unsigned char*) dst;
        ZStream.avail_out = bytes;

        while (1)
        {
            if (ZStream.avail_in == 0)
            {
                // Get more raw data.
                int newBytes = pIn->Read(DataBuffer, ZLib_BuffSize);                
                if (newBytes == 0)
                {
                    // The cupboard is bare!  We have nothing to feed to inflate().
                    break;
                }
                else
                {
                    SF_ASSERT(newBytes > 0);
                    ZStream.next_in = DataBuffer;
                    ZStream.avail_in = newBytes;
                }
            }

            int err = inflate(&ZStream, Z_SYNC_FLUSH);
            if (err == Z_STREAM_END)
            {
                AtEofFlag = 1;
                break;
            }
            if (err != Z_OK)
            {
                // something's wrong.
                ErrorCode = 1;
                break;
            }

            if (ZStream.avail_out == 0)             
                break;              
        }

        int bytesRead = bytes - ZStream.avail_out;
        LogicalStreamPos += bytesRead;

        return bytesRead;
    }

    // If we have unused bytes in our input buffer, rewind
    // to before they started.
    void    RewindUnusedBytes() 
    {
        if (ZStream.avail_in > 0)
        {
            int pos = pIn->Tell();
            int rewoundPos = pos - ZStream.avail_in;
            // SF_ASSERT(pos >= 0);
            // SF_ASSERT(pos >= InitialStreamPos);
            // SF_ASSERT(rewound_pos >= 0);
            // SF_ASSERT(rewound_pos >= InitialStreamPos);

            pIn->Seek(rewoundPos);
        }
    }

};


// ***** ZLibFile Implementation

// ZLibFile must be constructed with a source file 
ZLibFile::ZLibFile(File *psourceFile)
{
    pImpl = 0;
    if (psourceFile && psourceFile->IsValid())
        pImpl = SF_HEAP_AUTO_NEW(this) ZLibFileImpl(psourceFile);
    else
    {
        SF_DEBUG_WARNING(1, "ZLibFile constructor failed, psourceFile is not valid");
    }
}

ZLibFile::~ZLibFile()
{
    // If implementation still exists, release inflater
    if (pImpl)
    {
        pImpl->RewindUnusedBytes();
        inflateEnd(&(pImpl->ZStream));
        delete pImpl;   
    }
}


// ** File Information
const char* ZLibFile::GetFilePath()
{
    return pImpl ? pImpl->pIn->GetFilePath() : 0;
}

// Return 1 if file's usable (open)
bool    ZLibFile::IsValid()
{
    return pImpl ? 1 : 0;
}

// Return position
int    ZLibFile::Tell ()
{
    if (!pImpl) return -1;
    return pImpl->UserPos;
}
SInt64  ZLibFile::LTell ()
    { return Tell(); }


int ZLibFile::GetLength ()
{
    if (!pImpl) return 0;
    if (pImpl->ErrorCode)
        return 0;

    // This is expensive..
    int oldPos = pImpl->UserPos;
    int endPos = SeekToEnd();
    Seek(oldPos);
    return endPos;
}

SInt64  ZLibFile::LGetLength ()
{
    return GetLength();
}

// Return errno-based error code
int ZLibFile::GetErrorCode()
{
    // TODO : convert to ERRNO
    return pImpl ? pImpl->ErrorCode : 0;
}


// ** Stream implementation & I/O

int ZLibFile::Write(const UByte *pbuffer, int numBytes)
{
    SF_UNUSED2(pbuffer, numBytes);
    SF_DEBUG_WARNING(1, "ZLibFile::Write is not yet supported");
    return 0;
}

int ZLibFile::Read(UByte *pbuffer, int numBytes)
{
    if (!pImpl) return -1;
    return pImpl->Inflate(pbuffer, numBytes);
}


int ZLibFile::SkipBytes(int numBytes)
{
    return Seek(numBytes, SEEK_CUR);
}

int ZLibFile::BytesAvailable()
{
    if (!pImpl) return 0;
    if (pImpl->ErrorCode)
        return 0;

    // This is pricey..
    int oldPos = pImpl->UserPos;
    int endPos = SeekToEnd();
    Seek(oldPos);

    return endPos - oldPos;
}

bool    ZLibFile::Flush()
{
    return 1;
}


// Returns new position, -1 for error
int ZLibFile::Seek(int offset, int origin)
{
    if (!pImpl) return -1;

    //If there us an error, bail
    if (pImpl->ErrorCode)
        return pImpl->UserPos;

    switch(origin)
    {
        case Seek_Cur:
            // Adjust offset and fall through
            offset += pImpl->UserPos;

        case Seek_Set:
        zlib_seek_set:
            pImpl->SetPosition(offset);
            // Return new location
            break;
        
        case Seek_End:
            // Seek forward as far as possible (till end).
            pImpl->SetPosition(0x7FFFFFFF);

            // If offset is not at the end (i.e. usually negative), seek back
            if (offset != 0)
            {
                offset = pImpl->UserPos + offset;
                goto zlib_seek_set;
            }
            break;
    }

    return pImpl->UserPos;
}


SInt64  ZLibFile::LSeek(SInt64 offset, int origin)
{
    SF_DEBUG_WARNING(offset > (SInt64)0x7FFFFFFF, "ZLibFile::LSeek offset out of range, 64bit seek not yet supported" );
    return Seek((int)offset, origin);
}

// Writing not supported..
bool ZLibFile::ChangeSize(int newSize)
{
    SF_UNUSED(newSize);
    SF_DEBUG_WARNING(1, "ZLibFile::ChangeSize is not yet supported");
    return 0;
}
int ZLibFile::CopyFromStream(File *pstream, int byteSize)
{
    SF_UNUSED2(pstream, byteSize);
    SF_DEBUG_WARNING(1, "ZLibFile::CopyFromStream is not yet supported");
    return 0;
}

// Closes the file  
bool ZLibFile::Close()
{
    if (!pImpl) return 0;

    pImpl->RewindUnusedBytes();
    // Close really should not fail, since there is no way to handle that nicely
    int err = inflateEnd(&(pImpl->ZStream));

    //  Close & release the file
    pImpl->pIn->Close();
    delete pImpl;
    pImpl = 0;
    return (err == Z_OK);
}


// *** ZLib helper functions

// Memory allocation functions for ZLib.
static voidpf ZLibAllocFunc(voidpf opaque, uInt items, uInt size)
{
    void* pmem = SF_HEAP_AUTO_ALLOC(opaque, size * items);
   //  GFC_DEBUG_MESSAGE3(1, "Zlib Alloc 0x%08X - 0x%08X, %d bytes", pmem, ((UPInt)pmem) +  size * items - 1,  size * items);
    return pmem;
    //return SF_HEAP_AUTO_ALLOC(opaque, size * items);
}
static void   ZLibFreeFunc(voidpf, voidpf address)
{
   // GFC_DEBUG_MESSAGE1(1, "Zlib Free  0x%08X", address);
    SF_FREE(address);
}

int    ZLibFile::ZLib_InitStream(struct z_stream_s* pstream, void* pallocowner,
                                  void* pbuffer, unsigned int bufferSize)
{
    pstream->zalloc     = ZLibAllocFunc;
    pstream->zfree      = ZLibFreeFunc;
    pstream->opaque     = (voidpf)pallocowner;

    pstream->next_in    = 0;
    pstream->avail_in   = 0;
    pstream->next_out   = (Byte*)pbuffer;
    pstream->avail_out  = (uInt) bufferSize;

    pstream->data_type  =0;
    pstream->adler      =0;
    pstream->reserved   =0;

    return inflateInit(pstream);
}

}} //namespace Scaleform { namespace GFx {

#endif // SF_ENABLE_ZLIB
