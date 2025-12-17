/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Stream.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX__STREAM_H
#define INC_SF_GFX__STREAM_H

#include "Kernel/SF_Array.h"
// Include Renderer to load up its data types
#include "Render/Render_Matrix2x4.h"
#include "Render/Render_CxForm.h"
#include "Render/Render_Color.h"
// Must include log for proper inheritance
#include "GFx/GFx_Types.h"
#include "GFx/GFx_Log.h"

#include "GFx/GFx_Tags.h"
#include "GFx/GFx_String.h"

namespace Scaleform { 

// ***** External Classes
class File;

namespace GFx {

// ***** Declared Classes
class Stream;

class ParseControl;


struct TagInfo
{
    GFx::TagType TagType;
    int     TagOffset;
    int     TagLength;
    int     TagDataOffset;
};

// Stream is used to encapsulate bit-packed file reads.
class Stream : public LogBase<Stream>
{
public:
    
    Stream(File* pinput, MemoryHeap* pheap,
           Log *plog, ParseControl *pparseControl);
    Stream(const UByte* pbuffer, unsigned bufSize, MemoryHeap* pheap,
           Log *plog, ParseControl *pparseControl);
    ~Stream();

    // Re-initializes the stream, similar to opening it
    // from the constructor. Any unused bits are discarded.
    void            Initialize(File* pinput, Log *plog, ParseControl *pparseControl);

    // Shuts-down stream, releasing file and references to heap.
    void            ShutDown();


    // *** Stream I/O interface

    // The following routines are used very frequently,
    // so they should be optimized as much as possible.
    // This includes:
    //   - Align / ReadUInt(bits) / ReadSInt(Bits)  - used for packed structures.
    //   - ReadU8/U16/U32/S8/S16/S32                - used for member fields.
    //   - Tell / SetPosition / GetTagPosition      - used for seeking / positioning.
    //   - OpenTag / CloseTag                       - used for tags.

    void            Align()  { UnusedBits = 0; }

    unsigned        ReadUInt(unsigned bitcount);
    int             ReadSInt(unsigned bitcount);
    // Special cases for frequently used bit counts.
    unsigned        ReadUInt1();
    //unsigned        ReadUInt4();
    //unsigned        ReadUInt5();

    SF_INLINE UByte   ReadU8();
    SF_INLINE SInt8   ReadS8();
    SF_INLINE UInt16  ReadU16(); 
    SF_INLINE SInt16  ReadS16();
    SF_INLINE UInt32  ReadU32();
    SF_INLINE SInt32  ReadS32();
    SF_INLINE float   ReadFloat();

    SF_INLINE bool  ReadU32(UInt32*);

    UInt32          ReadVU32();
    UInt32          ReadVU30() { return ReadVU32(); } // should be checked for  (v&0xc0000000) == 0
    SInt32          ReadVS32();
    Double          ReadDouble();

    // Seeking / positioning.
    SF_INLINE int   Tell();
    int             GetTagEndPosition();
    void            SetPosition(int pos);      

    // Tag access; there is a max 2-entry tag stack.    
    TagType         OpenTag();
    TagType         OpenTag(TagInfo* ptagInfo);
    void            CloseTag();

    // Special reader for fill/line styles.
    SF_INLINE unsigned  ReadVariableCount();

    // Obtain file interface for the current location.
    SF_INLINE File* GetUnderlyingFile();
 

    // Reads in a null-terminated string. Returns false if string was empty.    
    bool            ReadString(String* pstr);   
    // For string that begins with an 8-bit length code.    
    bool            ReadStringWithLength(String* pstr);     
    // Read version that allocated memory from heap and passes it to user.
    char*           ReadString(MemoryHeap* pheap);
    char*           ReadStringWithLength(MemoryHeap* pheap);

    // Reposition the underlying file to the current stream location
    void            SyncFileStream();

    String& GetFileName() {return FileName;}

    // *** Reading SWF data types

    // Loading functions
    void            ReadMatrix(Render::Matrix2F *pm);
    void            ReadCxformRgb(Render::Cxform *pcxform);
    void            ReadCxformRgba(Render::Cxform *pcxform);
    void            ReadRect(RectF *pr);
    void            ReadPoint(Render::PointF *ppt);
    // Color
    void            ReadRgb(Color *pc);
    void            ReadRgba(Color *pc);    
    
    int             ReadToBuffer(UByte* pdestBuf, unsigned sz);

    // *** Delegated Logging Support 

    // GFxLogBase will output log messages to the appropriate logging stream,
    // but only if the enable option in context is set. 
    Log*            GetLog() const;
    bool            IsVerboseParse() const;
    bool            IsVerboseParseShape() const;
    bool            IsVerboseParseMorphShape() const;
    bool            IsVerboseParseAction() const;
    
#ifndef GFX_VERBOSE_PARSE    
    void    LogTagBytes() { }

    void    LogParseClass(Color color) { SF_UNUSED(color); }
    void    LogParseClass(const RectF &rect) { SF_UNUSED(rect); }
    void    LogParseClass(const Render::Cxform &cxform) { SF_UNUSED(cxform); }
    void    LogParseClass(const Matrix2F &matrix) { SF_UNUSED(matrix); }
#else   
    // Log the contents of the current tag, in hex.
    void    LogTagBytes();
    void    LogBytes(unsigned numOfBytes);

    // *** Helper functions to log contents or required types
    void    LogParseClass(Color color);
    void    LogParseClass(const RectF &rect);
    void    LogParseClass(const Render::Cxform &cxform);
    void    LogParseClass(const Render::Matrix2F &matrix);
#endif

    ParseControl*      GetParseControl() const { return pParseControl; }

    MemoryHeap*          GetHeap() const         { return FileName.GetHeap(); }

private:

    // Logging context and parse message control.
    Log*            pLog;
    ParseControl*   pParseControl;
    // Cached ParseControl flags from the states for more efficient checks.
    unsigned        ParseFlags;

    // File used for input
    Ptr<File>       pInput;
    // Bytes used for bit I/O.
    UByte           CurrentByte;
    UByte           UnusedBits;

    // Filename string; also stores the heap
    // from which loaded data should be allocated.
    StringDH        FileName;

    enum {
        Stream_TagStackSize = 2,
        Stream_BufferSize   = 512,
    };

    // *** Tag stack
   
    int             TagStack[Stream_TagStackSize];
    unsigned        TagStackEntryCount;  // # of entries in tag stack.
    
    // *** File Buffering        
    unsigned        Pos;        // Position in buffer
    unsigned        DataSize;   // Data in buffer if reading
    unsigned        FilePos;    // Underlying file position 
    bool            ResyncFile; // Set if 'GetUnderlyingFile' was called, so we need resync.
    UByte*          pBuffer;
    unsigned        BufferSize;
    UByte           BuiltinBuffer[Stream_BufferSize];

    // Buffer initialization.
    SF_INLINE bool  EnsureBufferSize1();
    // Return true, if the requested "size" was actually read from the stream
    SF_INLINE bool  EnsureBufferSize(int size);
    // Makes data available in buffer. Stores zeros if not available.
    bool            PopulateBuffer(int size); // returns true, if all requested bytes were read.
    bool            PopulateBuffer1();         // returns true, if byte was read from the stream
};



// *** Inlines

// Buffer initialization.
bool    Stream::EnsureBufferSize1()
{
    Align();
    if (((int)DataSize-(int)Pos) < 1)
        return PopulateBuffer1();
    return true;
}

bool    Stream::EnsureBufferSize(int size)
{
    Align();
    if (((int)DataSize-(int)Pos) < size)
        return PopulateBuffer(size);
    return true;
}

// Unsigned reads - access the buffer directly.
UByte       Stream::ReadU8()
{        
    EnsureBufferSize1();
    return pBuffer[Pos++];
}

UInt16      Stream::ReadU16()
{        
    EnsureBufferSize(sizeof(UInt16));
    UInt16 val = (UInt16(pBuffer[Pos])) | (UInt16(pBuffer[Pos+1]<<8));
    Pos += sizeof(UInt16);
    return val;
}

UInt32      Stream::ReadU32()
{
    EnsureBufferSize(sizeof(UInt32));
    UInt32 val = (UInt32(pBuffer[Pos])) | (UInt32(pBuffer[Pos+1])<<8) |
                 (UInt32(pBuffer[Pos+2])<<16) | (UInt32(pBuffer[Pos+3])<<24);
    Pos += sizeof(UInt32);
    return val;
}

bool       Stream::ReadU32(UInt32* pdest)
{
    if (!EnsureBufferSize(sizeof(UInt32)))
        return false;
    UInt32 v = ReadU32();
    if (pdest)
        *pdest = v;
    return true;
}

float       Stream::ReadFloat()
{
    EnsureBufferSize(sizeof(float));
    //AB: for XBox360 and PS3 float should be aligned on boundary of 4!
    SF_COMPILER_ASSERT(sizeof(float) == sizeof(UInt32));
    // Go through a union to avoid pointer strict aliasing problems.
    union {
        UInt32 ival;
        float  fval;
    };
    ival = (UInt32(pBuffer[Pos])) | (UInt32(pBuffer[Pos+1])<<8) |
           (UInt32(pBuffer[Pos+2])<<16) | (UInt32(pBuffer[Pos+3])<<24);
    Pos += sizeof(float);
    return fval;
}

SInt8       Stream::ReadS8()
{
    return (SInt8) ReadU8();
}
SInt16      Stream::ReadS16()
{
    return (SInt16) ReadU16();
}
SInt32      Stream::ReadS32()
{
    return (SInt32) ReadU32();
}

unsigned    Stream::ReadVariableCount()
{
    unsigned count = ReadU8();
    if (count == 0xFF)
        count = ReadU16();
    return count;
}

// Seeking / positioning.
int         Stream::Tell()
{
    // Since Tell() is called often, we avoid handling resyc here, as this
    // hasn't been necessary. SetPosition handles this condition correctly
    // and can be used to resync.
    SF_ASSERT(ResyncFile == false);
    return FilePos - DataSize + Pos;
}

File*      Stream::GetUnderlyingFile()
{    
    SyncFileStream();
    ResyncFile = true;
    return pInput;
}

}} // Scaleform::GFx

#endif // INC_SF_GFX_STREAM_H
