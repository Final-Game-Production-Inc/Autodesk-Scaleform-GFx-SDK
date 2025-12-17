/**************************************************************************

Filename    :   Amp_Stream.cpp
Content     :   Stream used by AMP to serialize messages
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Amp_Stream.h"
#ifdef SF_ENABLE_STATS

#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// Default constructor (for writing a new message)
AmpStream::AmpStream() : readPosition(0)
{
}

// Buffer constructor (for parsing a received message)
// Assumes that the input buffer includes the header information (length)
AmpStream::AmpStream(const UByte* buffer, UPInt bufferSize) : readPosition(0)
{
    Append(buffer, bufferSize);
}

// File virtual method override
int AmpStream::SkipBytes(int numBytes)
{ 
    readPosition += numBytes; 
    return numBytes; 
}

// File virtual method override
int AmpStream::BytesAvailable()
{ 
    return (static_cast<int>(Data.GetSize()) - readPosition); 
}

// File virtual method override
// Not supported for AmpStream
int AmpStream::Seek(int offset, int origin)
{ 
    SF_UNUSED(origin);
    SF_ASSERT(origin == Seek_Set);
    readPosition = offset;  // skip the header
    return offset; 
}

// File virtual method override
// Not supported for AmpStream
SInt64 AmpStream::LSeek(SInt64 offset, int origin)
{ 
    SF_DEBUG_WARNING(offset > (SInt64)0x7FFFFFFF, "AmpStream::LSeek offset out of range, 64bit seek not yet supported" );
    return Seek((int)offset, origin);
}

// File virtual method override
// Not supported for AmpStream
bool AmpStream::ChangeSize(int newSize)
{ 
    SF_UNUSED(newSize); 
    SF_ASSERT(false); // ChangeSize not supported
    return false;
}

// File virtual method override
// Not supported for AmpStream
int AmpStream::CopyFromStream(File *pstream, int byteSize)
{ 
    SF_UNUSED2(pstream, byteSize);
    SF_ASSERT(false); // Copy from stream not supported
    return -1;
}

// Serialization
void AmpStream::Read(File& str)
{
    UInt32 streamLength = str.ReadUInt32();
    Data.Resize(streamLength);
    for (UInt32 i = 0; i < streamLength; ++i)
    {
        Data[i] = str.ReadUByte();
    }
    SeekToBegin();
}

// Serialization
void AmpStream::Write(File& str) const
{
    UInt32 streamLength = static_cast<UInt32>(GetBufferSize());
    str.WriteUInt32(streamLength);
    if (streamLength > 0)
    {
        str.Write(GetBuffer(), static_cast<int>(GetBufferSize()));
    }
}


// Write a number of bytes to the stream
// File virtual method override
// Updates the message size stored in the header
int AmpStream::Write(const UByte *pbufer, int numBytes)
{
    IncreaseMessageSize(numBytes);
    memcpy(&Data[GetBufferSize() - numBytes], pbufer, numBytes);
    return numBytes;
}

// Read a number of bytes from the stream
// File virtual method override
int AmpStream::Read(UByte *pbufer, int numBytes)
{
    memcpy(pbufer, &Data[readPosition], numBytes);
    readPosition += numBytes;
    return numBytes;
}

// Append a buffer
// The buffer should already be in the proper format
// i.e. with message sizes stored before each message
void AmpStream::Append(const UByte* buffer, UPInt bufferSize)
{
    Data.Append(buffer, bufferSize);
    SeekToBegin();
}

// Data accessor
const UByte* AmpStream::GetBuffer() const
{
    return Data.GetDataPtr();
}

// Buffer size
UPInt AmpStream::GetBufferSize() const
{
    return Data.GetSize();
}

// Update the message size
// Assumes that there is only one message currently in the stream
void AmpStream::IncreaseMessageSize(UInt32 newSize)
{
    // Make space for header
    UInt32 sizePlusHeader;
    if (Data.GetSize() == 0)
    {
        sizePlusHeader = sizeof(UInt32) + newSize;
    }
    else
    {
        sizePlusHeader = static_cast<UInt32>(Data.GetSize()) + newSize;
    }
    Data.Resize(sizePlusHeader);

    // Write the header
    UInt32 sizeLE = Alg::ByteUtil::SystemToLE(sizePlusHeader);
    memcpy(&Data[0], reinterpret_cast<UByte*>(&sizeLE), sizeof(UInt32));
}

// Clear the buffer
void AmpStream::Clear()
{
    Data.Clear();
    WriteUInt32(0);
    SeekToBegin();
}

// Start reading from the beginning
int AmpStream::SeekToBegin()
{
    // skip the header
    return Seek(sizeof(UInt32));
}

// The size of the data without the header
UPInt AmpStream::FirstMessageSize()
{
    int oldIndex = readPosition;
    readPosition = 0;
    UInt32 msgSize = ReadUInt32();
    readPosition = oldIndex;
    return msgSize;
}

// Remove the first message from the stream
// Returns true if successful
bool AmpStream::PopFirstMessage()
{
    UPInt messageSize = FirstMessageSize();
    UPInt bufferLength = GetBufferSize();
    if (messageSize > bufferLength)
    {
        // incomplete message
        return false;
    }
    for (UPInt i = messageSize; i < bufferLength; ++i)
    {
        Data[i - messageSize] = Data[i];
    }
    Data.Resize(bufferLength - messageSize);
    SeekToBegin();
    return true;
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif
