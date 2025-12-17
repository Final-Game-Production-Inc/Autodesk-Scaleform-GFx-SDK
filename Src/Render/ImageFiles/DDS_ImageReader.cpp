/**************************************************************************

Filename    :   DDS_ImageReader.cpp
Content     :   DDS Image file format reader implementation
Created     :   February 2010
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "DDS_ImageFile.h"
#include "Render_ImageFileUtil.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#ifdef SF_OS_XBOX360
#include <xgraphics.h>
#endif
#ifdef SF_OS_ANDROID
#include "Render/GL/GL_Common.h"
#endif

namespace Scaleform { namespace Render { namespace DDS {

// DDSD (DirectDraw Surface Descriptor) constants.
enum DDSFlags
{
    DDSF_CAPS               =0x00000001l,
    DDSF_HEIGHT             =0x00000002l,
    DDSF_WIDTH              =0x00000004l,
    DDSF_PITCH              =0x00000008l,
    DDSF_BACKBUFFERCOUNT    =0x00000020l,
    DDSF_ZBUFFERBITDEPTH    =0x00000040l,
    DDSF_ALPHABITDEPTH      =0x00000080l,
    DDSF_LPSURFACE          =0x00000800l,
    DDSF_PIXELFORMAT        =0x00001000l,
    DDSF_CKDESTOVERLAY      =0x00002000l,
    DDSF_CKDESTBLT          =0x00004000l,
    DDSF_CKSRCOVERLAY       =0x00008000l,
    DDSF_CKSRCBLT           =0x00010000l,
    DDSF_MIPMAPCOUNT        =0x00020000l,
    DDSF_REFRESHRATE        =0x00040000l,
    DDSF_LINEARSIZE         =0x00080000l,
    DDSF_TEXTURESTAGE       =0x00100000l,
    DDSF_FVF                =0x00200000l,
    DDSF_SRCVBHANDLE        =0x00400000l,
    DDSF_DEPTH              =0x00800000l
};

//
enum DDSPixelFormat
{
    DDSPF_ALPHAPIXELS       =0x00000001l,
    DDSPF_ALPHA             =0x00000002l,
    DDSPF_FOURCC            =0x00000004l,
    DDSPF_PALETTEINDEXED4   =0x00000008l,
    DDSPF_PALETTEINDEXEDTO8 =0x00000010l,
    DDSPF_PALETTEINDEXED8   =0x00000020l,
    DDSPF_RGB               =0x00000040l,
    DDSPF_COMPRESSED        =0x00000080l
};

// In case this platform doesn't have this macro.
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
    ((unsigned)(UByte)(ch0) | ((unsigned)(UByte)(ch1) << 8) |       \
    ((unsigned)(UByte)(ch2) << 16) | ((unsigned)(UByte)(ch3) << 24 ))
#endif // MAKEFOURCC)

struct DDSDescr
{
    UInt32    RGBBitCount;
    UInt32    RBitMask;
    UInt32    GBitMask;
    UInt32    BBitMask;
    UInt32    ABitMask;
    bool      HasAlpha;
    UByte     ShiftR, ShiftG, ShiftB, ShiftA;
    inline DDSDescr()
    {
        RGBBitCount = RBitMask = GBitMask = BBitMask = ABitMask = 0;
        ShiftR = ShiftG = ShiftB = ShiftA;
        HasAlpha = false;                
    }

    static UByte CalcShiftByMask(UInt32 mask)
    {
        unsigned shifts = 0;

        if (mask == 0) return 0;

        if ((mask & 0xFFFFFFu) == 0)
        {
            mask >>= 24;
            shifts += 24;
        }
        else if ((mask & 0xFFFFu) == 0)
        {
            mask >>= 16;
            shifts += 16;
        }
        else if ((mask & 0xFFu) == 0)
        {
            mask >>= 8;
            shifts += 8;
        }
        while((mask & 1) == 0)
        {
            mask >>= 1;
            ++shifts;
        }
        return UByte(shifts);
    }

    void CalcShifts()
    {
        ShiftR = CalcShiftByMask(RBitMask);
        ShiftG = CalcShiftByMask(GBitMask);
        ShiftB = CalcShiftByMask(BBitMask);
        ShiftA = CalcShiftByMask(ABitMask);
    }
};

struct DDSHeaderInfo
{
    UInt32      Width;
    UInt32      Height;
    UInt32      Pitch;
    ImageFormat Format;
    UInt32      MipmapCount;
    DDSDescr    DDSFmt;
    bool        OppositeEndian;   // For X360, if the DDS is little endian, we need to know so we can byte swap the data.

    DDSHeaderInfo() : Width(0), Height(0), Pitch(0), Format(Image_None), MipmapCount(1), DDSFmt(), OppositeEndian(false) {}

};

struct DDSDXT10Info
{
    SInt32      DXGIFormat;
    UInt32      ResourceDimension;
    UInt32      MiscFlag; // see D3D11_RESOURCE_MISC_FLAG
    UInt32      ArraySize;
    UInt32      Reserved;
};

// Temporary DDS image class used as a data source, allowing
// direct initialization of RawImage or Texture with image data.

class DDSFileImageSource : public FileImageSource
{
    UByte           ImageDesc; 
    DDSHeaderInfo   HeaderInfo;
    
public:
    DDSFileImageSource(File* file, ImageFormat format)
        : FileImageSource(file,format), ImageDesc(0)
    {
    }
    virtual      ~DDSFileImageSource(){}
    bool ReadHeader();
    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    virtual unsigned        GetMipmapCount() const { return HeaderInfo.MipmapCount; }
};


// *** DDS Format loading

static const UByte* ParseUInt32(const UByte* buf, UInt32* pval)
{
    *pval = Alg::ByteUtil::LEToSystem(*(UInt32*)buf);    
    return buf + 4;
}



static bool Image_ParseDDSHeader(DDSHeaderInfo* pinfo, const UByte* buf, const UByte** pdata)
{
    ImagePlane p0;
    UInt32 flags;
    UInt32 v;
    
    buf = ParseUInt32(buf, &flags);

    buf = ParseUInt32(buf, &v);
    if (flags & DDSF_HEIGHT)
        pinfo->Height = v;

    buf = ParseUInt32(buf, &v);
    if (flags & DDSF_WIDTH)
        pinfo->Width = v;
    buf = ParseUInt32(buf, &v);
    if (flags & DDSF_PITCH)
        pinfo->Pitch = v;
    else if (flags & DDSF_LINEARSIZE)
        pinfo->Pitch = v/pinfo->Height*4; // Required by D3D10

    buf = ParseUInt32(buf, &v);
    //if (flags & DDSF_DEPTH)
    //    pimage->Depth = v;

    buf = ParseUInt32(buf, &v);
    if (flags & DDSF_MIPMAPCOUNT)
        pinfo->MipmapCount = v;

    //buf = ParseUInt32(buf, &v); // alpha bit count
    //if (flags & DDSF_ALPHABITDEPTH)
    //    pimage->AlphaBitDepth = v;

    buf += 11 * 4;

    if (flags & DDSF_PIXELFORMAT)
    {
        // pixel format (DDPIXELFORMAT)
        buf = ParseUInt32(buf, &v); // dwSize
        if (v != 32) // dwSize should be == 32 
        {
            SF_ASSERT(0);
            return false;
        }
      
        UInt32 pfflags;
        buf = ParseUInt32(buf, &pfflags);   // dwFlags
        buf = ParseUInt32(buf, &v);         // dwFourCC
        if (pfflags & DDSPF_FOURCC)
        {
            if (v == MAKEFOURCC('D','X','T','5'))        // DXT5
                pinfo->Format = Image_DXT5;
            else if (v == MAKEFOURCC('D','X','T','3'))   // DXT3
                pinfo->Format = Image_DXT3;
            else if (v == MAKEFOURCC('D','X','T','1'))   // DXT1
                pinfo->Format = Image_DXT1;
            else if (v == MAKEFOURCC('D','X','T','2')|| 
                     v == MAKEFOURCC('D','X','T','4') )  // DXT2 & DXT4 - not supported.
            {
                SF_DEBUG_WARNING(1, "DXT2 and DXT4 are not supported.\n");
                return false;
            }
            // ATC textures are stored in DDS files.
#ifdef GL_AMD_compressed_ATC_texture
            else if ( v == MAKEFOURCC('A','T','C',' '))
            {
                pinfo->Format = Image_ATCIC;
            }
            else if ( v == MAKEFOURCC('A','T','C','A'))
            {
                pinfo->Format = Image_ATCICA;
            }
            else if ( v == MAKEFOURCC('A','T','C','I'))
            {
                pinfo->Format = Image_ATCICI;
            }
#endif
            else if ( v == MAKEFOURCC('D','X','1','0'))
            {
                pinfo->Format = Image_BC7;
            }
            else
            {
                SF_DEBUG_WARNING4(1, "Unrecognized DDS FourCC code: %c%c%c%c\n", 
                    ((char*)&v)[0], ((char*)&v)[1], ((char*)&v)[2], ((char*)&v)[3] );
                return false;
            }
            buf += 20;  // skip remaining part of PixelFormat
        }
        else if ((pfflags & DDSPF_RGB) || (pfflags & DDSPF_ALPHA)) 
        {
            // uncompressed DDS. Only 32-bit/24-bit RGB formats and alpha only (A8) are supported
            UInt32 bitCount;
            buf = ParseUInt32(buf, &bitCount); // dwRGBBitCount
            pinfo->DDSFmt.RGBBitCount = bitCount;
            switch(bitCount)
            {
                case 32: pinfo->Format = Image_R8G8B8A8; break;
                case 24: pinfo->Format = Image_R8G8B8; break;
                case 8:
                    if (pfflags & DDSPF_ALPHA)
                    {
                        pinfo->Format = Image_A8;
                        break;
                    }
                default:
                    SF_ASSERT(0); // unsupported
                    return false;
            }
            if (!(flags & DDSF_PITCH))
                pinfo->Pitch = pinfo->Width*(bitCount/8);
                //AB: what is the Pitch in DDS for 24-bit RGB?
            
            buf = ParseUInt32(buf, &v); // dwRBitMask
            pinfo->DDSFmt.RBitMask = v;
            buf = ParseUInt32(buf, &v); // dwGBitMask
            pinfo->DDSFmt.GBitMask = v;
            buf = ParseUInt32(buf, &v); // dwBBitMask
            pinfo->DDSFmt.BBitMask = v;
            buf = ParseUInt32(buf, &v); // dwRGBAlphaBitMask
            if (pfflags & DDSPF_ALPHAPIXELS) 
            {
                pinfo->DDSFmt.ABitMask = v;
                pinfo->DDSFmt.HasAlpha = true;
            }

            // check for X8R8G8B8 - need to set alpha to 255
            if (v == 0 && bitCount == 32)
            {
                SF_ASSERT(0); // not supported for now.
                //@TODO - need to have one more Image_<> format for X8R8G8B8
            }
        }
        SF_ASSERT(pinfo->Format != Image_None); // Unsupported format
        if (pinfo->Format == Image_None)
            return false;
    }
    else
        buf += 32;
    buf += 16; // skip ddsCaps
    buf += 4; // skip reserved
    if (pdata) *pdata = buf;
    return true;
}

static bool ProcessUDDSData(UByte* buffer, UPInt size, ImageFormat format, const DDSDescr& ddsFmt)
{   
    if (format == Image_R8G8B8)
    {
        for (unsigned x = 0; x < size; x += 3)
        {
            UInt32 val = buffer[x + 0] | (UInt32(buffer[x + 1]) << 8) | (UInt32(buffer[x + 2]) << 16);
            buffer[x + 2] = UByte((val >> ddsFmt.ShiftB) & 0xFF); // B
            buffer[x + 1] = UByte((val >> ddsFmt.ShiftG) & 0xFF); // G
            buffer[x + 0] = UByte((val >> ddsFmt.ShiftR) & 0xFF); // R
        }
    }
    else if (format == Image_R8G8B8A8)
    {
        for (unsigned x = 0; x < size; x += 4)
        {
            UInt32 val = buffer[x + 0] | (UInt32(buffer[x + 1]) << 8) |
                (UInt32(buffer[x + 2]) << 16) | (UInt32(buffer[x + 3]) << 24);
            buffer[x + 2] = UByte((val >> ddsFmt.ShiftB) & 0xFF); // B
            buffer[x + 1] = UByte((val >> ddsFmt.ShiftG) & 0xFF); // G
            buffer[x + 0] = UByte((val >> ddsFmt.ShiftR) & 0xFF); // R
            if (ddsFmt.HasAlpha)
                buffer[x + 3] = UByte((val >> ddsFmt.ShiftA) & 0xFF); // A
            else
                buffer[x + 3] = 0xFF;
        }
    }

    return true;
}

bool DDSFileImageSource::Decode( ImageData* pdest, CopyScanlineFunc copyScanline, void* arg ) const
{
    if (!seekFileToDecodeStart())
        return false;

    unsigned mipWidth = Size.Width;
    unsigned mipHeight = Size.Height;
    for (unsigned m = 0; m < pdest->GetMipLevelCount(); m++)
    {
        ImagePlane mipPlane;
        if (pdest->HasSeparateMipmaps())
            pdest->GetMipLevelPlane(m, 0, &mipPlane);
        else
            pdest->GetPlaneRef().GetMipLevel(pdest->GetFormat(), m, &mipPlane);

        ImageScanlineBuffer<1024*4> scanline(HeaderInfo.Format, mipWidth, Format);
        int readSize = (int)scanline.GetReadSize();
        if (!scanline.IsValid())
            return false;
        for (unsigned y = 0; y < ImageData::GetFormatScanlineCount(Format, mipHeight); y++)
        {
            if (pFile->Read(scanline.GetReadBuffer(), readSize) != readSize)
                return false;
            if (Format == Image_R8G8B8A8 || Format == Image_R8G8B8)
                ProcessUDDSData(scanline.GetReadBuffer(), readSize, Format, HeaderInfo.DDSFmt);
            UByte* destScanline = mipPlane.GetScanline(y);
            scanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
#ifdef SF_OS_XBOX360
            if ( HeaderInfo.OppositeEndian && (Format >= Image_DXT1 && Format <= Image_DXT5 ))
                XGEndianSwapMemory(destScanline, destScanline, XGENDIAN_8IN16, sizeof(UInt16), readSize/2);
#endif
        }

        mipWidth  = Alg::Max(1u, mipWidth / 2); 
        mipHeight = Alg::Max(1u, mipHeight / 2); 
    }
    return true;

}

bool DDSFileImageSource::ReadHeader()
{
    UInt32  fourcc;
    bool    oppEndian = false;

    // Read without endian swapping.
    pFile->Read((UByte*)&fourcc, sizeof(UInt32));
    if (fourcc != MAKEFOURCC('D','D','S',' '))
    {
        // Perhaps it is just the wrong endianness for this platform.
        fourcc = (UInt32)Alg::ByteUtil::LEToSystem(fourcc);
        oppEndian = true;
    }

    if ( fourcc != MAKEFOURCC('D','D','S',' '))
        return false;

    UInt32 sz = pFile->ReadUInt32();
    if (sz != 124)
        return false;
    UByte buf[256];
    if (pFile->Read(buf, 120) != 120)
        return false;

    if (!Image_ParseDDSHeader(&HeaderInfo, buf, 0))
        return false;
    if (HeaderInfo.Format == Image_BC7)
        pFile->Seek(sizeof(DDSDXT10Info), File::Seek_Cur);
    HeaderInfo.OppositeEndian = oppEndian;
    HeaderInfo.DDSFmt.CalcShifts();

    if (Format == Image_None)
        Format =  HeaderInfo.Format;
    Size = ImageSize(HeaderInfo.Width, HeaderInfo.Height);
    FilePos = pFile->LTell();
    return true;
}
bool FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerArgSize) const
{
    FileHeaderReader<4> header(file, headerArg, headerArgSize);
    if (!header)
        return false;
    if ((header[0] != 'D') || (header[1] != 'D') || (header[2] != 'S') || (header[3] != ' '))
        return false;
    return true;
}

ImageSource* FileReader::ReadImageSource(File* file, const ImageCreateArgs& args) const
{
    if (!file || !file->IsValid())
        return 0;

    DDSFileImageSource* source = SF_NEW DDSFileImageSource(file, args.Format);
    if (source && !source->ReadHeader())
    {
        source->Release();
        return 0;
    }
    return source; 
   
}

// Instance singleton.
FileReader FileReader::Instance;

}}}

