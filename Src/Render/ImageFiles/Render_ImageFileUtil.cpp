/**************************************************************************

Filename    :   Render_ImageFileUtil.cpp
Content     :   Helper classes for ImageFile format loading
Created     :   August 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_ImageFileUtil.h"
#include "Render/Render_TextureUtil.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


//--------------------------------------------------------------------
// ***** FileImageSource

FileImageSource::FileImageSource(File* file, ImageFormat format, UInt64 len)
: Format(format), Use(0), pFile(file), FileLen(len)
{
    FilePos = pFile->LTell();
    SF_AMP_CODE(ImageId = ImageBase::GetNextImageId();)
}

FileImageSource::~FileImageSource()
{
}

bool FileImageSource::seekFileToDecodeStart() const
{
    return pFile && (pFile->LSeek(FilePos) == FilePos);
}

//--------------------------------------------------------------------
// ***** MemoryBufferImage

MemoryBufferImage::MemoryBufferImage(
            ImageFormat format, const ImageSize& size, unsigned use,
            ImageUpdateSync *sync,
            File* file, SInt64 filePos, UPInt length)
 : Render::Image(0, sync), Format(format), Size(size), Use(use)   
{
    SF_ASSERT(file);
    if (!file)
    {
        Format = Image_None;
        return;
    }

    if (length == 0)
        length = (UPInt) (file->LGetLength() - filePos);
    FileData.Resize(length);
    if (FileData.GetSize() != length)
    {
        Format = Image_None;
        return;
    }
    file->LSeek(filePos);
    if (file->Read(&FileData[0], (int)length) < (int)length)
    {
        Format = Image_None;
        return;
    }
    FilePath = file->GetFilePath();
    SF_AMP_CODE(ImageId = ImageBase::GetNextImageId();)
}

MemoryBufferImage::MemoryBufferImage(ImageFormat format, const ImageSize& size, unsigned use,
                                     ImageUpdateSync* sync, const UByte* data, UPInt dataSize)
: Render::Image(0, sync), Format(format), Size(size), Use(use)  
{
    SF_ASSERT(data && dataSize);
    FileData.Resize(dataSize);
    memcpy(FileData.GetDataPtr(), data, dataSize);
    SF_AMP_CODE(ImageId = ImageBase::GetNextImageId();)
}

Texture* MemoryBufferImage::GetTexture(TextureManager* pmanager)
{
    if (pTexture && pTexture->GetTextureManager() == pmanager)
    {
        return pTexture;
    }

    pTexture = 0;
    Texture* ptexture = pmanager->CreateTexture(Format, 1, Size, Use, this);
    initTexture_NoAddRef(ptexture);
    return ptexture;
}

/*
bool MemoryBufferImage::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    MemoryFile file(FilePath, &FileData[0], FileData.GetSize());
    Input*     jin = FileReader::Instance.CreateInput(&file);
    if (!jin) return 0;

    return DecodeHelper(Format, jin, pdest, copyScanline, arg);
}
*/


//--------------------------------------------------------------------
// ***** FileHeaderReader

FileHeaderReaderImpl::FileHeaderReaderImpl(File* file,
                                           UByte* header, UPInt headerSize,
                                           UByte* tempBuffer, UByte sizeNeeded)
  : pHeader(0)
{   
    if (!file || !file->IsValid())
        return;

    if (!header || headerSize < sizeNeeded)
    {        
        SInt64 pos = file->LTell();
        int    bytesRead = file->Read(tempBuffer, sizeNeeded);
        file->LSeek(pos);
        if (bytesRead < sizeNeeded)
            return;
        pHeader = tempBuffer;
    }
    else
    {
        pHeader = header;
    }    
}


//--------------------------------------------------------------------
// ***** ImageScanlineBuffer Implementation

ImageScanlineBufferImpl::ImageScanlineBufferImpl(
        ImageFormat readFormat, unsigned width, ImageFormat convertSourceFormat,
        UByte* tempBuffer, UPInt tempBufferSize)
 : ReadFormat(readFormat), 
   ConvertSourceFormat((convertSourceFormat != Image_None) ? convertSourceFormat : readFormat),
   Width(width),
   pReadScanline(0), pConvertScanline(0),
   ReadScanlineSize((ImageData::GetFormatBitsPerPixel(readFormat)*width) / 8),
   ConvertScanlineSize(0),
   pConvertFunc(0),
   BuffersAllocated(false)
{
    if (ReadFormat != ConvertSourceFormat)
    {
        ConvertScanlineSize = (ImageData::GetFormatBitsPerPixel(ConvertSourceFormat)*width) / 8;
        pConvertFunc        = GetImageConvertFunc(ConvertSourceFormat, ReadFormat);
        if (!pConvertFunc)
        {
            SF_DEBUG_WARNING(1, "Render::ImageScanlineBuffer - GetImageConvertFunc failed");
            return; // IsValid() will fail since pReadScanline is null
        }
    }

    // Assign / allocate buffers
    UPInt readSizeRounded = (ReadScanlineSize + sizeof(void*)*2) & ~(UPInt)(sizeof(void*)*2-1);
    UPInt totalSize       = readSizeRounded + ConvertScanlineSize;

    if (totalSize <= tempBufferSize)
    {
        pReadScanline = tempBuffer;
    }
    else
    {
        pReadScanline    = (UByte*)SF_ALLOC(totalSize, Stat_Default_Mem);
        BuffersAllocated = true;       
    }
    if (pConvertFunc)
        pConvertScanline = pReadScanline + readSizeRounded;
}

ImageScanlineBufferImpl::~ImageScanlineBufferImpl()
{
    if (BuffersAllocated)
        SF_FREE(pReadScanline);
}

void ImageScanlineBufferImpl::ConvertReadBuffer(UByte* dest, Palette* pal,
                                                CopyFunc copyScanline, void* scanlineArg)
{
    // Convert in two steps if formats didn't match up.
    if (pConvertFunc)
    {
        pConvertFunc(pConvertScanline, pReadScanline, ReadScanlineSize, pal, 0);
        copyScanline(dest, pConvertScanline, ConvertScanlineSize, 0, scanlineArg);
    }
    else
    {
        copyScanline(dest, pReadScanline, ReadScanlineSize, pal, scanlineArg);
    }
}


}}; // namespace Scaleform::Render
