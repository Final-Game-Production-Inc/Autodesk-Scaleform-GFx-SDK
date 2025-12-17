/**************************************************************************

Filename    :   GFx_Stream.cpp
Content     :   Byte/bit packed stream used for reading SWF files.
Created     :   
Authors     :   

Notes       :   wrapper class, for loading variable-length data from a
                Stream, and keeping track of SWF tag boundaries.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "Kernel/SF_File.h"
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Log.h"
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Alg.h"

#include "GFx/GFx_StreamContext.h"
#include "Kernel/SF_Debug.h"

#include <string.h>

namespace Scaleform { namespace GFx {

Stream::Stream(File* pinput, MemoryHeap* pheap,
                     Log *plog, ParseControl *pparseControl) 
    : FileName(pheap)
{
    BufferSize  = sizeof(BuiltinBuffer);
    pBuffer     = BuiltinBuffer;

    Initialize(pinput, plog, pparseControl);
}

Stream::Stream(const UByte* pbuffer, unsigned bufSize, MemoryHeap* pheap,
                     Log *plog, ParseControl *pparseControl)
    : FileName(pheap)
{
    pBuffer     = const_cast<UByte*>(pbuffer);
    if (pBuffer)
        BufferSize  = bufSize;
    else
        BufferSize  = 0;

    Initialize(NULL, plog, pparseControl);

    DataSize    = BufferSize;
    FilePos     = BufferSize;
}

Stream::~Stream()
{
}

void    Stream::Initialize(File* pinput, Log *plog, ParseControl *pparseControl)
{    
    pInput      = pinput;
    pLog        = plog;
    pParseControl = pparseControl;
    ParseFlags  = pparseControl ? pparseControl->GetParseFlags() : 0;
    CurrentByte = 0;
    UnusedBits  = 0;

    if (pinput)
        FileName = pinput->GetFilePath();
    else
        FileName.Clear();

    TagStackEntryCount = 0;
    TagStack[0] = 0;
    TagStack[1] = 0;

    Pos         = 0;
    DataSize    = 0;
    ResyncFile  = false;
    FilePos     = pinput ? pinput->Tell() : 0;
}


void    Stream::ShutDown()
{
    // Clear FileName since it contains allocation from the users heap.
    FileName.Clear();
    pInput = 0;
    pLog   = 0;
    pParseControl = 0;
}
    
// Reads a bit-packed unsigned integer from the Stream
// and returns it.  The given bitcount determines the
// number of bits to read.
unsigned    Stream::ReadUInt(unsigned bitcount)
{
    SF_ASSERT(bitcount <= 32);
        
    UInt32  value      = 0;
    int     bitsNeeded = bitcount;

    while (bitsNeeded > 0)
    {
        if (UnusedBits) 
        {
            if (bitsNeeded >= UnusedBits) 
            {
                // Consume all the unused bits.
                value |= (CurrentByte << (bitsNeeded - UnusedBits));

                bitsNeeded -= UnusedBits;                
                UnusedBits = 0;
            } 
            else 
            {
                int ubits = UnusedBits - bitsNeeded;

                // Consume some of the unused bits.
                value |= (CurrentByte >> ubits);
                // mask off the bits we consumed.
                CurrentByte &= ((1 << ubits) - 1);
                
                // We're done.
                UnusedBits = (UByte)ubits;
                bitsNeeded = 0;
            }
        } 
        else 
        {
            CurrentByte = ReadU8();
            UnusedBits  = 8;
        }
    }

    SF_ASSERT(bitsNeeded == 0);

    return value;
}


unsigned    Stream::ReadUInt1()
{    
    UInt32  value;
   
    if (UnusedBits == 0)
    {
        CurrentByte = ReadU8();
        UnusedBits  = 7;
        value       = (CurrentByte >> 7);
        CurrentByte &= ((1 << 7) - 1);
    }
    else
    {
        UnusedBits--;
        // Consume some of the unused bits.
        value       = (CurrentByte >> UnusedBits);
        CurrentByte &= ((1 << UnusedBits) - 1);
    }
    
    return value;
}


// Reads a bit-packed little-endian signed integer
// from the Stream.  The given bitcount determines the
// number of bits to read.
int    Stream::ReadSInt(unsigned bitcount)
{
    SF_ASSERT(bitcount <= 32);

    SInt32  value = (SInt32) ReadUInt(bitcount);

    // Sign extend...
    if (value & (1 << (bitcount - 1)))
    {
        value |= -1 << bitcount;
    }

    return value;
}


// Makes data available in buffer. Stores zeros if not available.
bool    Stream::PopulateBuffer(int size)
{
    if (DataSize == 0)
    {
        // In case Underlying file position was changed.
        if (pInput)
        {
            FilePos    = pInput->Tell();
            ResyncFile = false;
        }
    }
    SF_ASSERT(ResyncFile == false);

    // move data to front
    if (Pos < DataSize)
    {
        memmove(pBuffer, pBuffer + Pos, DataSize - Pos);
        DataSize = DataSize - Pos;
        Pos      = 0;        
    }
    else if (Pos >= DataSize)
    {
        SF_ASSERT(Pos == DataSize);
        DataSize = 0;
        Pos      = 0;
    }
    bool rv;
    if (pInput)
    {
        int readSize = BufferSize - (int)DataSize;
        int bytes    = pInput->Read(pBuffer + DataSize, readSize);

        if (bytes < readSize)
        {
            if (bytes > 0)
            {
                DataSize += (unsigned) bytes;
                FilePos  += (unsigned) bytes;
            }
            // Did not read enough. Set to zeros - error case.
            memset(pBuffer + DataSize, 0, BufferSize - DataSize);

            rv = (size <= (int)(DataSize - Pos)); //!AB: what if 'size' is greater than BufferSize???
            if ((int)(DataSize - Pos) < size)
            {
                int extraBytes = size - (int)(DataSize - Pos);
                // There is an attempt to read beyond the end-of-file. To avoid
                // assertion we simulate reading of all zeros.
                SF_DEBUG_ERROR3(1, "Read error: attempt to read %d bytes beyond the EOF, file %s, filepos %d", 
                    extraBytes, FileName.ToCStr(), (int)FilePos);
                DataSize += (unsigned) extraBytes;
            }
        }
        else
        {
            DataSize += (unsigned) bytes;
            FilePos  += (unsigned) bytes;
            rv       = true;
        } 
    }
    else 
    {
        // there is no file set to the stream - just 
        pBuffer     = BuiltinBuffer;
        BufferSize  = sizeof(BuiltinBuffer);
        memset(pBuffer, 0, BufferSize);
        Pos         = 0;
        DataSize    = BufferSize;
        FilePos    += DataSize;
        rv          = false;
    }
    SF_ASSERT(DataSize <= BufferSize );
    return rv;
}

bool    Stream::PopulateBuffer1()
{
    // Non-inline to simplify the call in ReadU8.
    return PopulateBuffer(1);
}

// Reposition the underlying file to the desired location
void    Stream::SyncFileStream()
{
    int pos = pInput->Seek(Tell());
    if (pos != -1)
    {
        Pos       = 0;
        FilePos   = (unsigned) pos;
        DataSize  = 0;
    }   
}



bool   Stream::ReadString(String* pstr)
{
    Align();

    char         c;
    Array<char> buffer;
    
    while ((c = ReadU8()) != 0)    
        buffer.PushBack(c);    
    buffer.PushBack(0);

    if (buffer.GetSize() == 0)
    {
        pstr->Clear();
        return false;
    }

    pstr->AssignString(&buffer[0], buffer.GetSize()-1);
    return true;
}

bool   Stream::ReadStringWithLength(String* pstr)
{
    // Use special initializer to avoid allocation copy.
    struct StringReader : public String::InitStruct
    {
        Stream* pStream;
        StringReader(Stream* pstream) : pStream(pstream) { }
        StringReader(StringReader&) { } // avoid warning

        virtual void InitString(char* pbuffer, UPInt size) const
        {
            for (UPInt i = 0; i < size; i++)
                pbuffer[i] = pStream->ReadU8();    
        }
    };

    Align();

    int len = ReadU8();
    if (len <= 0)
    {
        pstr->Clear();
        return false;
    }

    // String data is read in from InitStrng callback.
    StringReader sreader(this);
    pstr->AssignString(sreader, len);
    return true;
}

// Reads *and SF_ALLOC()'s* the string from the given file.
// Ownership passes to the caller; caller must delete[] the
// string when it is done with it.
char* Stream::ReadString(MemoryHeap* pheap)
{
    Align();

    char         c;
    Array<char> buffer;

    while ((c = ReadU8()) != 0)
        buffer.PushBack(c);    
    buffer.PushBack(0);

    if (!buffer.GetSize())
        return 0;

    char* retval = (char*)SF_HEAP_ALLOC(pheap, buffer.GetSize(), Stat_Default_Mem);
    memcpy(retval, &buffer[0], buffer.GetSize());
    return retval;
}

// Reads *and new[]'s* the string from the given file.
// Ownership passes to the caller; caller must delete[] the
// string when it is done with it.
char*   Stream::ReadStringWithLength(MemoryHeap* pheap)
{
    Align();

    int len = ReadU8();
    if (len <= 0)   
        return 0;
    
    char*   buffer = (char*)SF_HEAP_ALLOC(pheap, len + 1, Stat_Default_Mem);
    int     i;
    for (i = 0; i < len; i++)
        buffer[i] = ReadU8();
    buffer[i] = 0;
    return buffer;
}



int   Stream::ReadToBuffer(UByte* pdestBuf, unsigned sz)
{
    unsigned bytes_read = 0;
    if (DataSize == 0)
    {
        // In case Underlying file position was changed.
        FilePos    = pInput->Tell();
        ResyncFile = false;
    }
    SF_ASSERT(ResyncFile == false);

    // move existing data to front of destination buffer
    if (Pos < DataSize)
    {
        unsigned szFromBuf = Alg::Min(sz, DataSize - Pos);
        memmove(pdestBuf, pBuffer + Pos, szFromBuf);
        Pos += szFromBuf;
        sz  -= szFromBuf;
        pdestBuf += szFromBuf;
        bytes_read += szFromBuf;
    }
    if (Pos >= DataSize)
    {
        DataSize = 0;
        Pos      = 0;
    }

    if (sz)
    {
        int bytes = pInput->Read(pdestBuf, (int)sz);
        bytes_read += bytes;
        FilePos  += (unsigned) bytes;
        if (bytes < (int)sz)
        {
            // Did not read enough. Set to zeros - error case.
            memset(pdestBuf + bytes, 0, sz - bytes);
        }
    }
    return (int)bytes_read;
}



// Set the file position to the given value.
void    Stream::SetPosition(int pos)
{
    Align();

    // If we're in a tag, make sure we're not seeking outside the tag.
    if (TagStackEntryCount > 0)
    {
        SF_ASSERT(pos <= GetTagEndPosition());
        // @@ check start pos somehow???
    }
            
    if ((pos >= (int)(FilePos - DataSize) && (pos < (int)FilePos)))
    {
        // If data falls within the buffer, reposition the pointer.
        // Note that if we are here we know that (DataSize != 0) because
        // otherwise the (pos < (int)FilePos) condition would fail.
        SF_ASSERT((DataSize != 0) && (ResyncFile == false));
        Pos = unsigned(pos - FilePos + DataSize);
    }
    else
    {
        if (!ResyncFile && (Tell() == pos))
        {
            // If we are already at the right location, nothing to do.
            // This may happen frequently if we have just read all data and
            // are being repositioned to the same location, so just return.
            SF_ASSERT(FilePos == (unsigned)pInput->Tell());
        }
        else if (pInput->Seek(pos) >= 0)
        {
            ResyncFile  = false;

            // MA DEBUG
            //SF_DEBUG_MESSAGE3(1, "Stream::SetPosition(%d) - seek out of buffer; Tell() == %d, BuffStart = %d",
            //                      pos, Tell(), FilePos - DataSize);        
            Pos         = 0;
            DataSize    = 0;
            FilePos     = pos;             
        }
    }
}



// *** Tag handling.

// Return the tag type.
TagType Stream::OpenTag(TagInfo* pTagInfo)
{
    Align();
    int tagOffset   = Tell();
    int tagHeader   = ReadU16();
    int tagType     = tagHeader >> 6;
    int tagLength   = tagHeader & 0x3F;
    
    SF_ASSERT(UnusedBits == 0);
    if (tagLength == 0x3F)
        tagLength = ReadS32();
    
    pTagInfo->TagOffset     = tagOffset;
    pTagInfo->TagType       = (TagType)tagType;
    pTagInfo->TagLength     = tagLength;
    pTagInfo->TagDataOffset = Tell();

    if (IsVerboseParse())
        LogParse("---------------Tag type = %d, Tag length = %d, offset = %d\n", tagType, tagLength, tagOffset);
        
    // Remember where the end of the tag is, so we can
    // fast-forward past it when we're done reading it.
    SF_ASSERT(TagStackEntryCount < Stream_TagStackSize);
    TagStack[TagStackEntryCount] = Tell() + tagLength;
    TagStackEntryCount++;

    SF_ASSERT((TagStackEntryCount == 1) ||
            (TagStack[TagStackEntryCount-1] >= TagStack[TagStackEntryCount]) );
    
    return (TagType)tagType;
}

// Simplified OpenTag - no info reported back.
TagType Stream::OpenTag()
{
    Align();
    int tagHeader   = ReadU16();
    int tagType     = tagHeader >> 6;
    int tagLength   = tagHeader & 0x3F;

    SF_ASSERT(UnusedBits == 0);
    if (tagLength == 0x3F)
        tagLength = ReadS32();
    
    if (IsVerboseParse())
        LogParse("---------------Tag type = %d, Tag length = %d\n", tagType, tagLength);

    // Remember where the end of the tag is, so we can
    // fast-forward past it when we're done reading it.
    SF_ASSERT(TagStackEntryCount < Stream_TagStackSize);
        TagStack[TagStackEntryCount] = Tell() + tagLength;
    TagStackEntryCount++;

    return (TagType)tagType;
}

// Seek to the end of the most-recently-opened tag.
void    Stream::CloseTag()
{
    SF_ASSERT(TagStackEntryCount > 0);
    TagStackEntryCount--;
    SetPosition(TagStack[TagStackEntryCount]);
    UnusedBits = 0;
}

// Return the file position of the end of the current tag.
int Stream::GetTagEndPosition()
{
    // Bounds check - assumes unsigned.
    SF_ASSERT((TagStackEntryCount-1) < Stream_TagStackSize); 
    return ((TagStackEntryCount-1) < Stream_TagStackSize) ?
           TagStack[TagStackEntryCount-1] : 0;
}



// *** Reading SWF data types

// Loading functions
void    Stream::ReadMatrix(Render::Matrix2F *pm)
{
    Align();
    pm->SetIdentity();

    int hasScale = ReadUInt1();
    if (hasScale)
    {
        unsigned    ScaleNbits = ReadUInt(5);
        pm->Sx() = ReadSInt(ScaleNbits) / 65536.0f;
        pm->Sy() = ReadSInt(ScaleNbits) / 65536.0f;
    }
    int hasRotate = ReadUInt1();
    if (hasRotate)
    {
        unsigned    rotateNbits = ReadUInt(5);
        pm->Shy() = ReadSInt(rotateNbits) / 65536.0f;
        pm->Shx() = ReadSInt(rotateNbits) / 65536.0f;
    }

    int translateNbits = ReadUInt(5);
    if (translateNbits > 0)
    {
        pm->Tx() = (float) ReadSInt(translateNbits);
        pm->Ty() = (float) ReadSInt(translateNbits);
    }
}

void    Stream::ReadCxformRgb(Render::Cxform *pcxform)
{
    Align();
    unsigned hasAdd = ReadUInt1();
    unsigned hasMult = ReadUInt1();
    unsigned nbits = ReadUInt(4);

    if (hasMult)
    {
        // The divisor value must be 256,
        // since multiply factor 1.0 has value 0x100, not 0xFF
        pcxform->M[0][0] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][1] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][2] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][3] = 1.0f;
    }
    else
    {
        for (unsigned i = 0; i < 4; i++) { pcxform->M[0][i] = 1.0f; }
    }
    if (hasAdd)
    {
        pcxform->M[1][0] = (float) ReadSInt(nbits);
        pcxform->M[1][1] = (float) ReadSInt(nbits);
        pcxform->M[1][2] = (float) ReadSInt(nbits);
        pcxform->M[1][3] = 1.0f;
    }
    else
    {
        for (unsigned i = 0; i < 4; i++) { pcxform->M[1][i] = 0.0f; }
    }
    pcxform->Normalize();
}

void    Stream::ReadCxformRgba(Render::Cxform *pcxform)
{
    Align();

    unsigned hasAdd = ReadUInt1();
    unsigned hasMult = ReadUInt1();
    unsigned nbits = ReadUInt(4);

    if (hasMult)
    {
        // The divisor value must be 256,
        // since multiply factor 1.0 has value 0x100, not 0xFF
        pcxform->M[0][0] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][1] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][2] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][3] = ReadSInt(nbits) / 256.0f;
    }
    else
    {
        for (int i = 0; i < 4; i++) { pcxform->M[0][i] = 1.0f; }
    }
    if (hasAdd)
    {
        pcxform->M[1][0] = (float) ReadSInt(nbits);
        pcxform->M[1][1] = (float) ReadSInt(nbits);
        pcxform->M[1][2] = (float) ReadSInt(nbits);
        pcxform->M[1][3] = (float) ReadSInt(nbits);
    }
    else
    {
        for (int i = 0; i < 4; i++) { pcxform->M[1][i] = 0.0f; }
    }       
    pcxform->Normalize();
}


void    Stream::ReadRect(RectF *pr)
{
    Align();
    int nbits = ReadUInt(5);
    pr->x1  = (float) ReadSInt(nbits);
    pr->x2  = (float) ReadSInt(nbits);
    pr->y1  = (float) ReadSInt(nbits);
    pr->y2  = (float) ReadSInt(nbits);
}


void    Stream::ReadPoint(Render::PointF *ppt)
{
    SF_UNUSED(ppt);
    SF_DEBUG_ERROR(1, "Stream::ReadPoint not implemented");
}


// Color
void    Stream::ReadRgb(Color *pc)
{
    // Read: R, G, B
    pc->SetRed(ReadU8());
    pc->SetGreen(ReadU8());
    pc->SetBlue(ReadU8());
    pc->SetAlpha(0xFF); 
}

void    Stream::ReadRgba(Color *pc)
{
    // Read: RGB, A
    ReadRgb(pc);
    pc->SetAlpha(ReadU8()); 
}

UInt32 Stream::ReadVU32()
{
    UInt32 v = 0;
    UInt32 shift = 0;
    UInt8 b;
    do 
    {
        b = ReadU8();
        v |= (b & 0x7F) << shift;
        shift += 7;
    } while (shift < 32 && (b & 0x80) != 0);
    return v;
}

SInt32 Stream::ReadVS32()
{
    UInt32 v = 0;
    UInt32 shift = 0;
    UInt8 b;
    do 
    {
        b = ReadU8();
        v |= (b & 0x7F) << shift;
        shift += 7;
    } while (shift < 32 && (b & 0x80) != 0);
    if (b & 0x40) // negative value
    {
        return (SInt32)v - SInt32(1 << shift);
    }
    return (SInt32)v;
}

Double Stream::ReadDouble()
{
    EnsureBufferSize(sizeof(Double));
    // Go through a union to avoid pointer strict aliasing problems.
    union {
        UInt64 ival;
        double dval;
    };
    ival = 0;
    //AB: for XBox360 and PS3 float should be aligned on boundary of 8!
    SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt32)*2);
    int shift = 0;
    for (size_t i = 0; i < sizeof(Double); ++i)
    {
        ival |= (UInt64(pBuffer[Pos++]) << shift);
        shift += 8;
    }
    return dval;
}

// GFxLogBase impl
Log* Stream::GetLog() const
    { return pLog; }
bool    Stream::IsVerboseParse() const
    { return (ParseFlags & ParseControl::VerboseParse) != 0; }
bool    Stream::IsVerboseParseShape() const
    { return (ParseFlags & ParseControl::VerboseParseShape) != 0; }
bool    Stream::IsVerboseParseMorphShape() const
    { return (ParseFlags & ParseControl::VerboseParseMorphShape) != 0; }
bool    Stream::IsVerboseParseAction() const
    { return (ParseFlags & ParseControl::VerboseParseAction) != 0; }


    
// *** Helper functions to log contents or required types

#ifdef GFX_VERBOSE_PARSE

#define GFX_STREAM_BUFFER_SIZE  512

void    Stream::LogParseClass(Color color)
{
    char buff[GFX_STREAM_BUFFER_SIZE];
    Format(buff, sizeof(buff), color);
    LogParse("%s", buff);
}

void    Stream::LogParseClass(const RectF &rc)
{
    char buff[GFX_STREAM_BUFFER_SIZE];


    SFsprintf(buff, GFX_STREAM_BUFFER_SIZE,
        "xmin = %g, ymin = %g, xmax = %g, ymax = %g\n",
        TwipsToPixels(rc.x1),
        TwipsToPixels(rc.y1),
        TwipsToPixels(rc.x2),
        TwipsToPixels(rc.y2));

    //G_Format(
    //    StringDataPtr(buff, sizeof(buff)),
    //    "xmin = {0:g}, ymin = {1:g}, xmax = {2:g}, ymax = {3:g}\n",
    //    TwipsToPixels(rc.Left),
    //    TwipsToPixels(rc.Top),
    //    TwipsToPixels(rc.Right),
    //    TwipsToPixels(rc.Bottom)
    //    );

    LogParse("%s", buff);
}

void    Stream::LogParseClass(const Render::Cxform &cxform)
{
    char buff[GFX_STREAM_BUFFER_SIZE];
    Format(buff, sizeof(buff), cxform);
    LogParse("%s", buff);
}
void    Stream::LogParseClass(const Render::Matrix2F &matrix)
{
    char buff[GFX_STREAM_BUFFER_SIZE];
    Format(buff, sizeof(buff), matrix);
    LogParse("%s", buff);
}

// Log the contents of the current tag, in hex.
void    Stream::LogTagBytes()
{
    LogBytes(GetTagEndPosition() - Tell());
}

// Log the contents of the current tag, in hex.
void    Stream::LogBytes(unsigned numOfBytes)
{
    static const int    ROW_BYTES = 16;

    char    rowBuf[ROW_BYTES];
    int     rowCount = 0;

    for (unsigned i = 0; i < numOfBytes; ++i)
    {
        unsigned    c = ReadU8();
        LogParse("%02X", c);

        if (c < 32) c = '.';
        if (c > 127) c = '.';
        rowBuf[rowCount] = (UByte)c;

        rowCount++;
        if (rowCount >= ROW_BYTES)
        {
            LogParse("    ");
            for (int i_inner = 0; i_inner < ROW_BYTES; i_inner++)
            {
                LogParse("%c", rowBuf[i_inner]);
            }

            LogParse("\n");
            rowCount = 0;
        }
        else
        {
            LogParse(" ");
        }
    }

    if (rowCount > 0)
    {
        LogParse("\n");
    }
}
#endif //GFX_VERBOSE_PARSE


unsigned StreamContext::ReadUInt(unsigned bitcount)
{
    static const UInt8 bytesInBits[] = { 0,1,1,1,1,1,1,1,1, 
        2,2,2,2,2,2,2,2,
        3,3,3,3,3,3,3,3,
        4,4,4,4,4,4,4,4 };
    unsigned v;
    unsigned mask = ((1 << (8 - CurBitIndex)) - 1);
    unsigned rshift;
    unsigned lastBitIndex = CurBitIndex + bitcount;
    SF_ASSERT(bitcount < sizeof(bytesInBits)/sizeof(bytesInBits[0]));
    switch(bytesInBits[bitcount])
    {
    case 0: return 0;
    case 1: // 1 octet 1..8 bits
        if (lastBitIndex <= 8) // whole value fits in 1 bytes
        {
            v = pData[CurByteIndex] & mask;
            rshift = (8 - lastBitIndex);
        }
        else
        {
            // assemble a preliminary value using 2 bytes
            // and init rshift accordingly
            v = (unsigned(pData[CurByteIndex] & mask) << 8) | 
                 unsigned(pData[CurByteIndex + 1]);
            ++CurByteIndex;
            rshift = (16 - lastBitIndex);
        }
        break;
    case 2: // 2 octets 9..16 bits
        if (lastBitIndex <= 16) // whole value fits in 2 bytes
        {
            v = ((unsigned(pData[CurByteIndex]) & mask) << 8) | unsigned(pData[CurByteIndex + 1]);
            ++CurByteIndex;
            rshift = (16 - lastBitIndex);
        }
        else
        {
            // assemble a preliminary value using 3 bytes
            // and init rshift accordingly
            v = (unsigned(pData[CurByteIndex] & mask) << 16) | 
                (unsigned(pData[CurByteIndex + 1]) << 8) | 
                 unsigned(pData[CurByteIndex + 2]);
            CurByteIndex += 2;
            rshift = (24 - lastBitIndex);
        }
        break;
    case 3: // 3 octets 17..24 bits
        if (lastBitIndex <= 24) // whole value fits in 3 bytes 
        {
            v = (unsigned(pData[CurByteIndex] & mask) << 16) | 
                (unsigned(pData[CurByteIndex + 1]) << 8) | 
                 unsigned(pData[CurByteIndex + 2]);
            CurByteIndex += 2;
            rshift = (24 - lastBitIndex);
        }
        else
        {
            // assemble a preliminary value using 4 bytes
            // and init rshift accordingly
            v = (unsigned(pData[CurByteIndex] & mask) << 24) | 
                (unsigned(pData[CurByteIndex + 1]) << 16) | 
                (unsigned(pData[CurByteIndex + 2]) << 8) | 
                 unsigned(pData[CurByteIndex + 3]);
            CurByteIndex += 3;
            rshift = (32 - lastBitIndex);
        }
        break;
    case 4: // 4 octets 25..32 bits
        if (lastBitIndex <= 32) // whole value fits in 4 bytes 
        {
            v = (unsigned(pData[CurByteIndex] & mask) << 24) | 
                (unsigned(pData[CurByteIndex + 1]) << 16) | 
                (unsigned(pData[CurByteIndex + 2]) << 8) | 
                 unsigned(pData[CurByteIndex + 3]);
            CurByteIndex += 3;
            rshift = (32 - lastBitIndex);
        }
        else
        {
            // assemble a preliminary value using 4 bytes
            v = (unsigned(pData[CurByteIndex] & mask) << 24) | 
                (unsigned(pData[CurByteIndex + 1]) << 16) | 
                (unsigned(pData[CurByteIndex + 2]) << 8) | 
                 unsigned(pData[CurByteIndex + 3]);
            CurByteIndex += 4;

            // need to read 5th byte and take "extraBits" bits from it
            unsigned extraBits = (lastBitIndex - 32);
            v <<= extraBits;
            v |= unsigned(pData[CurByteIndex]) >> (8 - extraBits);

            // value is already shifted correctly.
            // just modify CurBitIndex and return
            CurBitIndex = extraBits;
            SF_ASSERT(CurBitIndex < 8);
            return v;
        }
        break;
    default: SF_ASSERT(0); v = 0; rshift = 0; // only up to 32-bits values supported
    }
    if (rshift)
    {
        v >>= rshift;
        CurBitIndex = 8 - rshift;
    }
    else
    {
        ++CurByteIndex;
        CurBitIndex = 0;
    }
    SF_ASSERT(CurBitIndex < 8);
    return v;
}

void StreamContext::ReadMatrix(Render::Matrix2F *pm)
{
    Align();
    pm->SetIdentity();

    int hasScale = ReadUInt1();
    if (hasScale)
    {
        unsigned ScaleNbits = ReadUInt(5);
        pm->Sx() = ReadSInt(ScaleNbits) / 65536.0f;
        pm->Sy() = ReadSInt(ScaleNbits) / 65536.0f;
    }
    int hasRotate = ReadUInt1();
    if (hasRotate)
    {
        unsigned rotateNbits = ReadUInt(5);
        pm->Shy() = ReadSInt(rotateNbits) / 65536.0f;
        pm->Shx() = ReadSInt(rotateNbits) / 65536.0f;
    }

    int translateNbits = ReadUInt(5);
    if (translateNbits > 0)
    {
        pm->Tx() = (float) ReadSInt(translateNbits);
        pm->Ty() = (float) ReadSInt(translateNbits);
    }
}

void StreamContext::ReadCxformRgb(Render::Cxform *pcxform)
{
    Align();
    unsigned hasAdd = ReadUInt1();
    unsigned hasMult = ReadUInt1();
    unsigned nbits = ReadUInt(4);

    if (hasMult)
    {
        // The divisor value must be 256,
        // since multiply factor 1.0 has value 0x100, not 0xFF
        pcxform->M[0][0] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][1] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][2] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][3] = 1.0f;
    }
    else
    {
        for (unsigned i = 0; i < 4; i++) { pcxform->M[0][i] = 1.0f; }
    }
    if (hasAdd)
    {
        pcxform->M[1][0] = (float) ReadSInt(nbits);
        pcxform->M[1][1] = (float) ReadSInt(nbits);
        pcxform->M[1][2] = (float) ReadSInt(nbits);
        pcxform->M[1][3] = 1.0f;
    }
    else
    {
        for (unsigned i = 0; i < 4; i++) { pcxform->M[1][i] = 0.0f; }
    }
    pcxform->Normalize();
}

void StreamContext::ReadCxformRgba(Render::Cxform *pcxform)
{
    Align();

    unsigned hasAdd = ReadUInt1();
    unsigned hasMult = ReadUInt1();
    unsigned nbits = ReadUInt(4);

    if (hasMult)
    {
        // The divisor value must be 256,
        // since multiply factor 1.0 has value 0x100, not 0xFF
        pcxform->M[0][0] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][1] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][2] = ReadSInt(nbits) / 256.0f;
        pcxform->M[0][3] = ReadSInt(nbits) / 256.0f;
    }
    else
    {
        for (int i = 0; i < 4; i++) { pcxform->M[0][i] = 1.0f; }
    }
    if (hasAdd)
    {
        pcxform->M[1][0] = (float) ReadSInt(nbits);
        pcxform->M[1][1] = (float) ReadSInt(nbits);
        pcxform->M[1][2] = (float) ReadSInt(nbits);
        pcxform->M[1][3] = (float) ReadSInt(nbits);
    }
    else
    {
        for (int i = 0; i < 4; i++) { pcxform->M[1][i] = 0.0f; }
    }       
    pcxform->Normalize();
}

}} // Scaleform::GFx

