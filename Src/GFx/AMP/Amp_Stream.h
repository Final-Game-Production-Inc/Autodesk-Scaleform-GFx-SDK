/**************************************************************************

Filename    :   Amp_Stream.h
Content     :   Stream used by AMP to serialize messages
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INCLUDE_GFX_AMP_STREAM_H
#define INCLUDE_GFX_AMP_STREAM_H

#include "GFxConfig.h"
#ifdef SF_ENABLE_STATS

#include "Kernel/SF_File.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// Custom stream class used to send messages across the network
// The size of each message precedes the data so we can split messages into 
// packets and reconstruct them after they have been received on the other end
//
// The data are stored as a Little-endian array of bytes
// The object normally holds only one message for write operations
// because the first four bytes are used to hold the message size
// For read operations, multiple messages can be contained on the stream
// After a message has been processed, PopFirstMessage is called to remove it
class AmpStream : public File
{
public:
    // Default constructor (for writing a new message)
    AmpStream();
    // Buffer constructor (for parsing a received message)
    AmpStream(const UByte* buffer, UPInt bufferSize);

    // File virtual method overrides
    virtual const char* GetFilePath()   { return "AMP stream"; } 
    virtual bool        IsValid()       { return true; }
    virtual bool        IsWritable()    { return true; }
    virtual int         Tell ()         { return readPosition; }
    virtual SInt64      LTell ()        { return  Tell(); }
    virtual int         GetLength ()    { return static_cast<int>(Data.GetSize()); }
    virtual SInt64      LGetLength ()   { return GetLength(); }
    virtual int         GetErrorCode()  { return 0; }
    virtual bool        Flush()         { return true; }
    virtual bool        Close()         { return false; }
    virtual int         Write(const UByte *pbufer, int numBytes);
    virtual int         Read(UByte *pbufer, int numBytes);
    virtual int         SkipBytes(int numBytes);                     
    virtual int         BytesAvailable();
    virtual int         Seek(int offset, int origin=Seek_Set);
    virtual int         SeekToBegin();
    virtual SInt64      LSeek(SInt64 offset, int origin=Seek_Set);
    virtual bool        ChangeSize(int newSize);
    virtual int         CopyFromStream(File *pstream, int byteSize);

    // Serialization
    void                Read(File& str);
    void                Write(File& str) const;

    // Append an buffer that is already in the AmpStream format
    void                Append(const UByte* buffer, UPInt bufferSize);

    // Data accessors
    const UByte*        GetBuffer() const;
    UPInt               GetBufferSize() const;

    // Clear the stream
    void                Clear();

    // Message retrieval methods
    UPInt               FirstMessageSize();
    bool                PopFirstMessage();

private:
    typedef ArrayConstPolicy<0, 4, true> NeverShrinkPolicy;
    ArrayLH<UByte, Stat_Default_Mem, NeverShrinkPolicy>     Data;
    int                                                     readPosition;  // next location to be read

    // Update the header with the message size
    // Assumes that there is only one message currently in the stream
    void IncreaseMessageSize(UInt32 newSize);
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform


#endif

#endif
