/**************************************************************************

Filename    :   SIF_ImageWriter.cpp
Content     :   SIF Image writer implementation
Created     :   July 2011
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SIF_ImageFile.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
//#include "Kernel/SF_HeapNew.h"
#include "Render_ImageFileUtil.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace SIF {

#define SIF_HEADER "SIF "

class SIFFileImageSource : public FileImageSource
{
    ImageData         Data;
    SIFHeaderInfo     HeaderInfo;
public:
    SIFFileImageSource(File* file, ImageFormat format)
        : FileImageSource(file,format)
    {
    }
    virtual      ~SIFFileImageSource(){}
    bool ReadHeader();
    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    virtual unsigned        GetMipmapCount() const { return Alg::Max<unsigned>(1, HeaderInfo.LevelCount); }

    SF_AMP_CODE(virtual UPInt GetBytes(int* memRegion) const { if (memRegion) *memRegion = 0; return Data.GetBitsPerPixel() * Data.GetSize().Area() / 8; })
};

bool SIFFileImageSource::ReadHeader()
{
    UByte  fourcc[4];
    pFile->Read(fourcc, 4);
	if (strncmp((char*)fourcc, SIF_HEADER, 4))
		return false;
    UByte hsize = pFile->ReadUByte();
    if (hsize != 4 + 1 + 4 + 4 + 1 + 1 + 2)
        return false;
    HeaderInfo.Format = Format = static_cast<ImageFormat>(pFile->ReadSInt32());
    Use = pFile->ReadUInt32();
    HeaderInfo.Flags = (pFile->ReadUByte() & (~ImageData::Flag_AllocPlanes));
    HeaderInfo.LevelCount = pFile->ReadUByte();
    HeaderInfo.RawPlaneCount = pFile->ReadUInt16();
    
    HeaderInfo.Width = pFile->ReadUInt32();
    HeaderInfo.Height = pFile->ReadUInt32();

    Size = ImageSize(HeaderInfo.Width, HeaderInfo.Height);

    FilePos = pFile->LTell();
    return true;
}

bool SIFFileImageSource::Decode( ImageData* pdest, CopyScanlineFunc copyScanline, void* arg ) const
{
    SF_UNUSED2(arg, copyScanline); 
    if (!seekFileToDecodeStart())
        return false;

    //MemoryHeap* pheap = Memory::GetHeapByAddress(pdest);
    //ImagePlane* pplanes = (ImagePlane*)
    //    SF_HEAP_ALLOC(pheap, sizeof(ImagePlane)* HeaderInfo.RawPlaneCount, Stat_Image_Mem);

    //pdest->Initialize(Format, HeaderInfo.LevelCount, pplanes, HeaderInfo.RawPlaneCount, (HeaderInfo.Flags & ImageData::Flag_SeparateMipmaps) != 0);
    //pdest->Initialize(Format, HeaderInfo.LevelCount, (HeaderInfo.Flags & ImageData::Flag_SeparateMipmaps) != 0);

    pdest->Flags |= HeaderInfo.Flags;

    UInt32 pitch = pFile->ReadUInt32();
    UInt32 dataSize = pFile->ReadUInt32();

    SF_ASSERT (pdest->pPlanes[0].DataSize == dataSize);
    SF_ASSERT (pdest->GetPitch() == pitch);
    if (pdest->pPlanes[0].DataSize != dataSize || pdest->GetPitch() != pitch)
        return false;
    //UByte* pdata = (UByte*)SF_ALLOC(dataSize, Stat_Image_Mem);// plane data need to be freed by caller
    //pdest->SetPlane(0, Size, pitch, dataSize, pdata);
    for (UPInt j = 0; j < pdest->pPlanes[0].DataSize; ++j)
    {
        pdest->pPlanes[0].pData[j] = pFile->ReadUByte();
    }

    for (UInt16 i = 1; i < pdest->RawPlaneCount; ++i)
    {
        UInt32 width = pFile->ReadUInt32();
        UInt32 height = pFile->ReadUInt32();
        ImageSize planeSize(width, height);
        UInt32 imgPitch = pFile->ReadUInt32();
        UInt32 imgDataSize = pFile->ReadUInt32();
        SF_ASSERT (pdest->pPlanes[i].DataSize == imgDataSize);
        SF_ASSERT (pdest->GetPitch() == imgPitch);
        if (pdest->pPlanes[0].DataSize != imgDataSize || pdest->GetPitch() != imgPitch)
            return false;
        //UByte* pdata = (UByte*)SF_ALLOC(dataSize, Stat_Image_Mem);// plane data need to be freed by caller
        //pdest->SetPlane(i, planeSize, pitch, dataSize, pdata);
        for (UPInt j = 0; j < pdest->pPlanes[i].DataSize; ++j)
        {
            pdest->pPlanes[i].pData[j] = pFile->ReadUByte();
        }
    }

    UInt16 colorCount = pFile->ReadUInt16();
    if (colorCount > 0)
    {
        bool hasAlpha = (pFile->ReadUByte() != 0);
        pdest->pPalette = Palette::Create(colorCount, hasAlpha);
        for (UInt16 i = 0; i < colorCount; ++i)
        {
            (*pdest->pPalette)[i] = Color::FromColor32(pFile->ReadUInt32());
        }
    }
    else
    {
        pdest->pPalette = NULL;
    }
    return true;
}



bool FileReader::MatchFormat( File* file, UByte* headerArg, UPInt headerArgSize ) const
{
    FileHeaderReader<4> header(file, headerArg, headerArgSize);
    if (!header)
        return false;
    if ((header[0] != 'S') || (header[1] != 'I') || (header[2] != 'F') || (header[3] != ' '))
        return false;
    return true;
}

ImageSource* FileReader::ReadImageSource( File* file, const ImageCreateArgs& args /*= ImageCreateArgs()*/ ) const
{
    if (!file || !file->IsValid())
        return 0;

    SIFFileImageSource* source = SF_NEW SIFFileImageSource(file, args.Format);
    if (source && !source->ReadHeader())
    {
        source->Release();
        return 0;
    }
    return source; 
}




FileReader FileReader::Instance;

}}}; // namespace Scaleform::Render::SIF

