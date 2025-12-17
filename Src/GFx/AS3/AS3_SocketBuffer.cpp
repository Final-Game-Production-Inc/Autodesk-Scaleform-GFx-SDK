/**************************************************************************

Filename    :   SocketBuffer.cpp
Created     :   December 2012
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_SocketBuffer.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {


// File virtual method override
int SocketBuffer::SkipBytes(int numBytes)
{ 
    readPosition += numBytes; 
    return numBytes; 
}

// File virtual method override
int SocketBuffer::BytesAvailable()
{ 
    return (static_cast<int>(Data.GetSize()) - readPosition); 
}

// File virtual method override
// Not supported for SocketBuffer
int SocketBuffer::Seek(int offset, int origin)
{ 
    SF_UNUSED(origin);
    SF_ASSERT(origin == Seek_Set);
    readPosition = offset;  // skip the header
    return offset; 
}

// File virtual method override
// Not supported for AmpStream
SInt64 SocketBuffer::LSeek(SInt64 offset, int origin)
{ 
    SF_DEBUG_WARNING(offset > (SInt64)0x7FFFFFFF, "SocketBuffer::LSeek offset out of range, 64bit seek not yet supported" );
    return Seek((int)offset, origin);
}

// File virtual method override
// Not supported for AmpStream
bool SocketBuffer::ChangeSize(int newSize)
{ 
    SF_UNUSED(newSize); 
    SF_ASSERT(false); // ChangeSize not supported
    return false;
}

// File virtual method override
// Not supported for AmpStream
int SocketBuffer::CopyFromStream(File *pstream, int byteSize)
{ 
    SF_UNUSED2(pstream, byteSize);
    SF_ASSERT(false); // Copy from stream not supported
    return -1;
}

// Serialization
void SocketBuffer::Read(File& str)
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
void SocketBuffer::Write(File& str) const
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
int SocketBuffer::Write(const UByte *pbufer, int numBytes)
{
    memcpy(&Data[GetBufferSize() - numBytes], pbufer, numBytes);
    return numBytes;
}

// Read a number of bytes from the stream
// File virtual method override
int SocketBuffer::Read(UByte *pbufer, int numBytes)
{
    memcpy(pbufer, &Data[readPosition], numBytes);
    readPosition += numBytes;
    return numBytes;
}

// Append a buffer
void SocketBuffer::Append(const UByte* buffer, UPInt bufferSize)
{
    Data.Append(buffer, bufferSize);
}

// Data accessor
const UByte* SocketBuffer::GetBuffer() const
{
    return Data.GetDataPtr();
}

// Buffer size
UPInt SocketBuffer::GetBufferSize() const
{
    return Data.GetSize();
}

// Clear the buffer
void SocketBuffer::Clear()
{
    Data.Clear();
    SeekToBegin();
}

void SocketBuffer::DiscardReadBytes()
{
    if (readPosition > 0)
    {
        for (UPInt i = readPosition; i < Data.GetSize(); ++i)
        {
            Data[i - readPosition] = Data[i];
        }
        Data.Resize(Data.GetSize() - readPosition);
        readPosition = 0;
    }
}

int SocketBuffer::GetReadPosition()
{
    return readPosition;
}



// Start reading from the beginning
int SocketBuffer::SeekToBegin()
{
    // skip the header
    return Seek(0);
}

} // namespace AS3
} // namespace GFx
} // namespace Scaleform
