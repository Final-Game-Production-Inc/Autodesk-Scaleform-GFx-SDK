/**************************************************************************

Filename    :   GFx_ZlibSupport.cpp
Content     :   
Created     :   April 15, 2008
Authors     :   

Notes       :   Redesigned to use inherited state objects for GFx 2.2.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

///////////// WARNING ///////////////////////////////////////////////
// The implementation of ZlibSupport class is moved into a separate 
// file to avoid linking ZLIB functionality into the application if 
// it doesn't need ZLIB support and ZlibSupport state is not set to 
// the loader.

#include "GFx/GFx_Stream.h"
#include "GFx/GFx_Loader.h"

#ifdef SF_ENABLE_ZLIB
#include <zlib.h>
#include "GFx/GFx_ZLibFile.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx {

File* ZlibSupport::CreateZlibFile(File* in)
{
    return SF_HEAP_AUTO_NEW(this) ZLibFile(in);
}

// Wrapper function -- uses Zlib to uncompress bufferBytes worth
// of data from the input file into BufferBytes worth of data
// into *buffer.
void  ZlibSupport::InflateWrapper(Stream* pinStream, void* buffer, int bufferBytes)
{
    SF_ASSERT(buffer);
    SF_ASSERT(bufferBytes > 0);
    
    // Initialize decompression stream.
    z_stream D_stream; 
    int      err = ZLibFile::ZLib_InitStream(&D_stream, this, buffer, bufferBytes);

    if (err != Z_OK)
    {
        pinStream->LogError("GFx_InflateWrapper() inflateInit() returned %d", err);
        return;
    }

    enum { DStreamSourceBuffSize = 32};
    UByte    buff[DStreamSourceBuffSize];

    while(1)
    {
        // Fill a buffer.
        D_stream.next_in  = &buff[0];
        D_stream.avail_in = pinStream->ReadToBuffer(buff, DStreamSourceBuffSize);

        // D_stream.avail_in unsigned ...
        // Maybe we should check for overflow instead ...
        //if (D_stream.avail_in < 0)
        //{            
        //    pinStream->LogError("GFx_InflateWrapper() read data error");
        //    break;
        //}

        err = inflate(&D_stream, Z_SYNC_FLUSH);
        if (err == Z_STREAM_END) break;        
        if (err != Z_OK)
        {            
            pinStream->LogError("GFx_InflateWrapper() Inflate() returned %d", err);
            if (D_stream.avail_in > 0)
                pinStream->SetPosition(pinStream->Tell() - D_stream.avail_in);
            SF_ASSERT(0);
            break;
        }
        //buf[0] = pinStream->ReadU8();
    }

    if (D_stream.avail_in > 0)
        pinStream->SetPosition(pinStream->Tell() - D_stream.avail_in);

    err = inflateEnd(&D_stream);
    if (err != Z_OK)
    {
        pinStream->LogError("GFx_InflateWrapper() InflateEnd() return %d", err);
    }
}

}} //namespace Scaleform { namespace GFx {

#else

#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace GFx {

    File* ZlibSupport::CreateZlibFile(File* in)
{
    SF_UNUSED(in);
    SF_DEBUG_WARNING(1, "ZlibSupport::CreateZlibFile failed - SF_ENABLE_ZLIB not defined");
    return NULL;
}
void  ZlibSupport::InflateWrapper(Stream* pinStream, void* buffer, int BufferBytes)
{
    SF_UNUSED3(pinStream,buffer,BufferBytes);
    SF_DEBUG_WARNING(1, "ZlibSupport::InflateWrapper failed - SF_ENABLE_ZLIB not defined");
}

}} //namespace Scaleform { namespace GFx {

#endif

