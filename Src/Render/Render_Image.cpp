/**************************************************************************

Filename    :   Render_Image.cpp
Content     :   Image implementation
Created     :   December 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Image.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Render_DrawableImage.h"
#include "Render/Render_TextureUtil.h"

namespace Scaleform { namespace Render {


// **** ImageSize Helpers

ImageSize SF_STDCALL ImageSize_RoundUpPow2(ImageSize sz)
{
    ImageSize size(1);
    while (size.Width  < sz.Width)  { size.Width <<= 1; }
    while (size.Height < sz.Height) { size.Height <<= 1; }
    return size;
}

ImageSize SF_STDCALL ImageSize_NextMipSize(ImageSize sz)
{    
    sz.Width  = Alg::Max<UInt32>(1, sz.Width/2);
    sz.Height = Alg::Max<UInt32>(1, sz.Height/2);
    return sz;
}

unsigned SF_STDCALL ImageSize_MipLevelCount(ImageSize sz)
{
    unsigned levels = 1;
    while((sz.Width > 1) && (sz.Height > 1))
    {
        sz = ImageSize_NextMipSize(sz);
        levels++;
    }
    return levels;
}


// ***** ImagePlane

    
void    ImagePlane::GetMipLevel(ImageFormat format, unsigned level,
                                ImagePlane* p, unsigned plane) const
{
    *p = *this;
    UPInt levelSize = 0, totalLevelSize = 0;

    for(unsigned lv = level; lv != 0; lv--)
    {
        UPInt mipLevelSize = ImageData::GetMipLevelSize(format, p->GetSize(), plane);        
        p->pData += mipLevelSize;
        p->SetNextMipSize();
        p->Pitch  = ImageData::GetFormatPitch(format, p->Width, plane);

        totalLevelSize += mipLevelSize;
        SF_ASSERT(totalLevelSize <= DataSize);
    }
    p->DataSize = DataSize - totalLevelSize + levelSize;
}

// ***** Palette

Palette* SF_STDCALL Palette::Create(unsigned colorCount,
                                                bool hasAlpha, MemoryHeap* pheap)
{
    if (!pheap)
        pheap = Memory::GetGlobalHeap();
    SF_ASSERT(colorCount < 0x10000);
    Palette* pmap = (Palette*)
        SF_HEAP_ALLOC(pheap, sizeof(Palette) + (colorCount-1)*sizeof(Color),
                      Stat_Default_Mem);
    if (pmap)
    {
        pmap->RefCount    = 1;
        pmap->ColorCount  = (UInt16)colorCount;
        pmap->HasAlphaFlag= hasAlpha;
        memset(pmap->Colors, 0, sizeof(Color)*colorCount);
    }
    return pmap;
}


//--------------------------------------------------------------------
// ***** ImageData

bool    ImageData::allocPlanes(ImageFormat format, unsigned mipLevelCount,
                               bool separateMipmaps)
{
    SF_ASSERT((pPlanes == &Plane0) && (!(Flags & Flag_AllocPlanes)));
    SF_ASSERT(mipLevelCount < 256); // LevelCount is UByte.

    // GetFormat plane count
    unsigned fmtPlanes = GetFormatPlaneCount(format);
    unsigned planeCount= fmtPlanes * (separateMipmaps ? mipLevelCount : 1);
    
    if (planeCount > 1)
    {
        MemoryHeap* pheap = (Flags & Flag_LocalHeap) ?
                            Memory::GetHeapByAddress(this) : Memory::GetGlobalHeap();
        pPlanes = (ImagePlane*)
            SF_HEAP_ALLOC(pheap, sizeof(ImagePlane)* planeCount, Stat_Default_Mem);
        if (!pPlanes)
        {
            RawPlaneCount = 1;
            pPlanes    = &Plane0;
            return false;
        }
        memset(pPlanes, 0, sizeof(ImagePlane)* planeCount);
        Flags |= Flag_AllocPlanes;
    }

    Format        = format;
    RawPlaneCount = (UInt16)planeCount;
    LevelCount    = (UByte)mipLevelCount;
    if (separateMipmaps)
        Flags |= Flag_SeparateMipmaps;
    return true;
}

void    ImageData::freePlanes()
{
    if (Flags & Flag_AllocPlanes)
    {
        SF_ASSERT(pPlanes && (pPlanes != &Plane0));
        Flags &= ~Flag_AllocPlanes;
        SF_FREE(pPlanes);        
    }
    pPlanes = &Plane0;
}

ImageData& ImageData::operator=(const ImageData& rhs)
{
    freePlanes();

    // Copy fields
    Format = rhs.Format;
    Use = rhs.Use;
    Flags = rhs.Flags;
    LevelCount = rhs.LevelCount;
    RawPlaneCount = rhs.RawPlaneCount;
    pPalette = rhs.pPalette;
    Plane0 = rhs.Plane0;

    if (rhs.Flags & Flag_AllocPlanes)
    {
        Flags &= ~Flag_AllocPlanes;
        allocPlanes(Format, LevelCount, (Flags & Flag_SeparateMipmaps) != 0);
        for (UInt16 p = 0; p < rhs.RawPlaneCount; p++)
        {
            SetPlane(p, ImageSize(rhs.pPlanes[p].Width, rhs.pPlanes[p].Height),
                rhs.pPlanes[p].Pitch, rhs.pPlanes[p].DataSize, rhs.pPlanes[p].pData);
        }
    }
    else
    {
        pPlanes = &Plane0;
    }
    return *this;
}


void ImageData::Clear()
{
    freePlanes(); // Sets pPlanes.
    Format      = Image_None;
    Use         = 0;
    Flags       = (UByte)(Flags & (~Flag_LocalHeap));
    LevelCount  = 0;
    RawPlaneCount = 1;
    pPalette.Clear();
    Plane0.SetData(0, 0, 0);
}



// Initializes ImageData to a sub-range of levels from source, which should have valid planes.
// Shouldn't fail unless allocPlanes() does.
bool ImageData::Initialize(const ImageData& source,
                           unsigned levelIndex, unsigned levelCount)
{
    SF_ASSERT((levelIndex < source.LevelCount) &&
              (levelIndex + levelCount <= source.LevelCount));
    if (levelCount == 0)
        levelCount = source.LevelCount - levelIndex;

    // If we are indexing base level or have separate mipmaps, just 
    // initialize the data planes pointer to internal planes what we already have.
    if ((levelIndex == 0) || (source.Flags & ImageData::Flag_SeparateMipmaps))
    {
        unsigned formatPlaneCount = ImageData::GetFormatPlaneCount(source.Format);
        unsigned planeCount       = ((source.Flags & ImageData::Flag_SeparateMipmaps) ? levelCount : 1)
                                    * formatPlaneCount;

        Initialize(source.Format, levelCount,
                   source.pPlanes + levelIndex * formatPlaneCount,
                   planeCount, (source.Flags & ImageData::Flag_SeparateMipmaps) != 0);
        return true;
    }

    // If we are indexing non-base mipmap level and mipmaps are not separate,
    // new planes need to be initialized.
    if (!Initialize(source.Format, levelCount, false))
        return false;

    SF_ASSERT(source.RawPlaneCount == RawPlaneCount);   
    for (unsigned i = 0; i< source.RawPlaneCount; i++)
    {
        const ImagePlane& splane = source.GetPlaneRef(i);
        ImagePlane& dplane = GetPlaneRef(i);
        splane.GetMipLevel(source.Format, levelIndex, &dplane, i);
    }

    return true;
}


// Configure planes & mipmaps
unsigned ImageData::GetPlaneCount() const
{
    return RawPlaneCount * ((Flags & Flag_SeparateMipmaps) ? LevelCount : 1);
}

void    ImageData::GetPlane(unsigned index, ImagePlane* pplane) const
{
    if (index < RawPlaneCount)
    {
        *pplane = GetPlaneRef(index);
        return;
    }

    // We get here only when indexing "virtual" non-separated mip-map planes;
    // i.e. the default case where all mipmaps represented by one raw plane.
    SF_ASSERT(!(Flags & Flag_SeparateMipmaps));
    
    unsigned level    = index / RawPlaneCount;
    unsigned rawPlane = index % RawPlaneCount;
    const ImagePlane& plane = GetPlaneRef(rawPlane);
    plane.GetMipLevel(Format, level, pplane, rawPlane);
}


// Set pixel, sets only the appropriate channels
void    ImageData::SetPixelARGB(unsigned x, unsigned y, UInt32 color)
{
    // Bounds check
    if ((x >= GetWidth()) || (y >= GetHeight()))
        return;

    UByte *pline = GetScanline(y);
    SetPixelInScanline(pline, x, color);
}

void ImageData::SetPixelInScanline( UByte * pline, unsigned x, UInt32 color )
{
    switch(GetFormatNoConv())
    {
    case Image_B8G8R8A8:
        *(pline + x * 4 + 0)    = (UByte) color & 0xFF;        // B
        *(pline + x * 4 + 1)    = (UByte) (color>>8) & 0xFF;   // G
        *(pline + x * 4 + 2)    = (UByte) (color>>16) & 0xFF;  // R
        *(pline + x * 4 + 3)    = (UByte) (color>>24);         // A
        break;
    case Image_R8G8B8A8:
        *(pline + x * 4 + 0)    = (UByte) (color>>16) & 0xFF;  // R
        *(pline + x * 4 + 1)    = (UByte) (color>>8) & 0xFF;   // G
        *(pline + x * 4 + 2)    = (UByte) color & 0xFF;        // B
        *(pline + x * 4 + 3)    = (UByte) (color>>24);         // A
        // Old:
        //*(((UInt32*)pline) + x) = Alg::ByteUtil::LEToSystem(color);
        break;

    case Image_R8G8B8:
        // Data order is packed 24-bit, RGBRGB...,
        // regardless of the endian-ness of the CPU.
        *(pline + x * 3 + 0)    = (UByte) (color>>16) & 0xFF; // R
        *(pline + x * 3 + 1)    = (UByte) (color>>8) & 0xFF;  // G
        *(pline + x * 3 + 2)    = (UByte) color & 0xFF;       // B
        break;
    case Image_B8G8R8:
        // Data order is packed 24-bit, RGBRGB...,
        // regardless of the endian-ness of the CPU.
        *(pline + x * 3 + 0)    = (UByte) color & 0xFF;       // B
        *(pline + x * 3 + 1)    = (UByte) (color>>8) & 0xFF;  // G
        *(pline + x * 3 + 2)    = (UByte) (color>>16) & 0xFF; // R
        break;
    case Image_A8:
        *(pline + x) = (UByte) (color >> 24);
        break;
    case Image_PS3_A8R8G8B8:
        *(pline + x * 4 + 0)    = (UByte) (color>>24);         // A
        *(pline + x * 4 + 1)    = (UByte) (color>>16) & 0xFF;  // R
        *(pline + x * 4 + 2)    = (UByte) (color>>8) & 0xFF;   // G
        *(pline + x * 4 + 3)    = (UByte) color & 0xFF;        // B
        break;
    default:
        SF_DEBUG_ASSERT1(0, "Attempting SetPixelInScaline on unsupported image format (%d)", Format);
        break;
    }
}

Color   ImageData::GetPixel(unsigned x, unsigned y) const
{
    // Bounds check
    if ((x >= GetWidth()) || (y >= GetHeight()))
        return 0;

    const UByte *pline = GetScanline(y);
    return GetPixelInScanline(pline, x);
}

Color ImageData::GetPixelInScanline( const UByte * pline, unsigned x ) const
{
    Color color;
    switch(GetFormatNoConv())
    {
    case Image_B8G8R8A8:
        color.SetColor( 
            *(pline + x * 4 + 2),   // B
            *(pline + x * 4 + 1),   // G
            *(pline + x * 4 + 0),   // R
            *(pline + x * 4 + 3));  // A
        break;
    case Image_R8G8B8A8:
        color.SetColor( 
            *(pline + x * 4 + 0),   // R
            *(pline + x * 4 + 1),   // G
            *(pline + x * 4 + 2),   // B
            *(pline + x * 4 + 3));  // A
        break;

    case Image_R8G8B8:
        color.SetColor( 
            *(pline + x * 4 + 0),   // R
            *(pline + x * 4 + 1),   // G
            *(pline + x * 4 + 2),   // B
            0xFF);                  // A
        break;
    case Image_B8G8R8:
        color.SetColor( 
            *(pline + x * 4 + 2),   // B
            *(pline + x * 4 + 1),   // G
            *(pline + x * 4 + 0),   // R
            0xFF);                  // A
    case Image_A8:
        color.SetColor( 
            0xFF,                   // R
            0xFF,                   // G
            0xFF,                   // B
            *(pline + x * 4 + 0));  // A
        break;
    case Image_PS3_A8R8G8B8:
        color.SetColor( 
            *(pline + x * 4 + 1),   // A
            *(pline + x * 4 + 2),   // R
            *(pline + x * 4 + 3),   // G
            *(pline + x * 4 + 0));  // B
            
        break;

    default:
        SF_DEBUG_ASSERT1(0, "Attempting GetPixelInScaline on unsupported image format (%d)", Format);
        break;
    }
    return color;
}

void    ImageData::SetPixelAlpha(unsigned x, unsigned y, UByte alpha)
{
    if ((x >= GetWidth()) || (y >= GetHeight()))
        return;

    UByte *pline = GetScanline(y);

    switch(Format)
    {
    case Image_R8G8B8A8:
        // Target is always little-endian
        *(pline + x * 4 + 3)   = alpha;
        break;      
    case Image_A8:
        *(pline + x) = alpha;
        break;      
    case Image_PS3_A8R8G8B8:
        *(pline + x * 4 + 0)   = alpha;
    default:
        SF_DEBUG_ASSERT1(0, "Attempting SetPixelAlpha on unsupported image format (%d)", Format);
        break;
    }
}

// plane data need to be freed by caller
void ImageData::Read(File& file, UInt32 version)
{
    SF_UNUSED(version);
    freePlanes();

    Format = static_cast<ImageFormat>(file.ReadSInt32());
    Use = file.ReadUInt32();
    Flags = UByte((file.ReadUByte() & (~Flag_AllocPlanes)));
    LevelCount = file.ReadUByte();
    RawPlaneCount = file.ReadUInt16();

    allocPlanes(Format, LevelCount, (Flags & Flag_SeparateMipmaps) != 0);
    for (UInt16 i = 0; i < RawPlaneCount; ++i)
    {
        UInt32 width = file.ReadUInt32();
        UInt32 height = file.ReadUInt32();
        ImageSize planeSize(width, height);
        UInt32 pitch = file.ReadUInt32();
        UInt32 dataSize = file.ReadUInt32();
        UByte* pdata = (UByte*)SF_ALLOC(dataSize, Stat_Image_Mem);// plane data need to be freed by caller
        SetPlane(i, planeSize, pitch, dataSize, pdata);
        for (UPInt j = 0; j < pPlanes[i].DataSize; ++j)
        {
            pPlanes[i].pData[j] = file.ReadUByte();
        }
    }

    UInt16 colorCount = file.ReadUInt16();
    if (colorCount > 0)
    {
        bool hasAlpha = (file.ReadUByte() != 0);
        pPalette = Palette::Create(colorCount, hasAlpha);
        for (UInt16 i = 0; i < colorCount; ++i)
        {
            (*pPalette)[i] = Color::FromColor32(file.ReadUInt32());
        }
    }
    else
    {
        pPalette = NULL;
    }
}

void ImageData::Write(File& file, UInt32 version) const
{
    SF_UNUSED(version);

    file.WriteSInt32(Format);
    file.WriteUInt32(Use);
    file.WriteUByte(Flags);
    file.WriteUByte(LevelCount);
    file.WriteUInt16(RawPlaneCount);
    for (UInt16 i = 0; i < RawPlaneCount; ++i)
    {
        file.WriteUInt32(pPlanes[i].Width);
        file.WriteUInt32(pPlanes[i].Height);
        file.WriteUInt32((UInt32)pPlanes[i].Pitch);
        file.WriteUInt32((UInt32)pPlanes[i].DataSize);
        for (UPInt j = 0; j < pPlanes[i].DataSize; ++j)
        {
            file.WriteUByte(pPlanes[i].pData[j]);
        }
    }

    if (pPalette)
    {
        file.WriteUInt16(static_cast<UInt16>(pPalette->GetColorCount()));
        file.WriteUByte(pPalette->HasAlpha() ? 1 : 0);
        for (unsigned i = 0; i < pPalette->GetColorCount(); ++i)
        {
            file.WriteUInt32((*pPalette)[i].ToColor32());
        }
    }
    else
    {
        file.WriteUInt16(0);
    }
}


// Computes a hash of the given data buffer.
// Hash function suggested by http://www.cs.yorku.ca/~oz/hash.html
// Due to Dan Bernstein.  Allegedly very good on strings.
//
// One problem with this hash function is that e.g. if you take a
// bunch of 32-bit ints and hash them, their hash values will be
// concentrated toward zero, instead of randomly distributed in
// [0,2^32-1], because of shifting up only 5 bits per byte.
SF_INLINE UPInt GImageBase_BernsteinHash(const void* pdataIn, UPInt size, UPInt seed = 5381)
{
    const UByte*    pdata = (const UByte*) pdataIn;
    UPInt           h    = seed;

    while (size > 0)
    {
        size--;
        h = ((h << 5) + h) ^ (unsigned) pdata[size];
    }

    return h;
}


// Compute a hash code based on image contents.  Can be useful
// for comparing images. Will return 0 if pData is null.
UPInt   ImageData::ComputeHash() const
{
    UPInt h = GImageBase_BernsteinHash(&RawPlaneCount, sizeof(RawPlaneCount));

    for (unsigned i = 0; i < RawPlaneCount; i++)
    { 
        ImagePlane& plane = pPlanes[i];
        h = GImageBase_BernsteinHash(&plane.Width, sizeof(plane.Width), h);
        h = GImageBase_BernsteinHash(&plane.Height, sizeof(plane.Height), h);
        h = GImageBase_BernsteinHash(&plane.Pitch, sizeof(plane.Pitch), h);
        h = GImageBase_BernsteinHash(&plane.DataSize, sizeof(plane.DataSize), h);
        
        if (!plane.pData)
            return 0;
        h = GImageBase_BernsteinHash(plane.pData, plane.DataSize, h);
    }
    return h;
}

// Ensures that all aspects of the image match up, including layout.
bool    ImageData::operator == (const ImageData& other) const
{
    if ((Format != other.Format) ||
        (Use != other.Use) ||
      //  (Flags != other.Flags) ||   // It seems that comparing flags is not useful here. TBD.
        (LevelCount != other.LevelCount) ||
        (RawPlaneCount != other.RawPlaneCount))
        return false;

    unsigned plane;

    if (pPalette)
    {
        if (!other.pPalette || (*pPalette != *other.pPalette))
            return false;
    }
    else
    {
        if (other.pPalette)
        return false;
    }

    for (plane = 0; plane < RawPlaneCount; plane++)
    {   
        if (!pPlanes[plane].MatchSizes(other.pPlanes[plane]))
            return false;
    }
    
    // Compare data bits last, only if all other data matched.
    for (plane = 0; plane < RawPlaneCount; plane++)
    {   
        if (memcmp(pPlanes[plane].pData, other.pPlanes[plane].pData, 
                   pPlanes[plane].DataSize))
           return false;
    }
    return true;
}


// *** Image Static functions

unsigned ImageData::GetFormatPlaneCount(ImageFormat fmt)
{
    switch(fmt & ImageFormat_Mask)
    {
    case Image_None:
        return 0;
    case Image_Y8_U2_V2:
        return 3;
    case Image_Y8_U2_V2_A8:
        return 4;
    default:
        return 1;
    }
}

unsigned  ImageData::GetFormatBitsPerPixel(ImageFormat fmt, unsigned plane)
{
    SF_ASSERT(plane < GetFormatPlaneCount(fmt));
    SF_UNUSED(plane);

    switch(fmt & ImageFormat_Mask)
    {
    case Image_R8G8B8A8:
    case Image_B8G8R8A8:
        return 32;
    case Image_R8G8B8:
    case Image_B8G8R8:
        return 24;
    case Image_DXT1:
        return 16;
    case Image_A8:
    case Image_P8:
    case Image_Y8_U2_V2:
    case Image_Y8_U2_V2_A8:
        return 8;
    case Image_DXT3:
    case Image_DXT5:
    case Image_BC7:
        return 32;
    case Image_PVRTC_RGB_2BPP:
    case Image_PVRTC_RGBA_2BPP:
        return 2;
    case Image_PVRTC_RGB_4BPP:
    case Image_PVRTC_RGBA_4BPP:
    case Image_ETC1_RGB_4BPP:
        return 4;
            
    case Image_None:
    default:
        break;
    }
    return 0;
}

UPInt  ImageData::GetFormatPitch(ImageFormat fmt, UInt32 width, unsigned plane)
{
    switch(fmt & ImageFormat_Mask)
    {
    case Image_A8:    
    case Image_P8:
    case Image_Y8_U2_V2:
    case Image_Y8_U2_V2_A8:
        SF_UNUSED(plane);
        return width;
    case Image_R8G8B8A8:
    case Image_B8G8R8A8:
        return width * 4;
    case Image_R8G8B8:
    case Image_B8G8R8:
        return (width * 3 + 3) & ~3;
    case Image_DXT1:
        return (8 * ((width+3) / 4)); // 64 bits per 4x4 texel.
    case Image_DXT5:
    case Image_DXT3:
    case Image_BC7:
        return (16 * ((width+3) / 4)); // 128 bits per 4x4 texel.
    case Image_PVRTC_RGB_2BPP:
    case Image_PVRTC_RGBA_2BPP:
            return width/4;
    case Image_PVRTC_RGB_4BPP:
    case Image_PVRTC_RGBA_4BPP:
    case Image_ETC1_RGB_4BPP:
    case Image_ETC2_RGB:
    case Image_ETC2_RGBA1:
    case Image_ATCIC:
            return width/2;
    case Image_ATCICA:
    case Image_ETC2_RGBA:
            return width;
    default:
        break;
    }
    // Some platforms depend on their formats having 0 pitch in this function.
    return 0;
}

UPInt  ImageData::GetFormatScanlineCount(ImageFormat fmt, UInt32 height, unsigned plane)
{
    SF_UNUSED(plane);
    switch(fmt & ImageFormat_Mask)
    {
    case Image_DXT1:
    case Image_DXT3:
    case Image_DXT5:
    case Image_BC7:
        return (height + 3) / 4;
    default:
        break;    
    }
    return height;
}

ImageSize  ImageData::GetFormatPlaneSize(ImageFormat fmt, const ImageSize& sz, unsigned plane)
{
    switch(fmt & ImageFormat_Mask)
    {
    case Image_Y8_U2_V2:
    case Image_Y8_U2_V2_A8:
        if ((plane == 1) || (plane == 2))
            return ImageSize(sz.Width / 2, sz.Height / 2);
        break;
    default:
        break;
    }
    return sz;
}


UPInt ImageData::GetMipLevelSize(ImageFormat format, const ImageSize& sz, unsigned plane)
{
    UPInt levelSize;
    unsigned fmt = unsigned(format) & ImageFormat_Mask;
    if (fmt == Image_DXT1)
        levelSize = Alg::Max<UInt32>(1u, (sz.Width+3) / 4) *
                    Alg::Max<UInt32>(1u, (sz.Height+3) / 4) * 8;
    else if ((fmt >= Image_DXT3 && fmt <= Image_BC7) ||(fmt >= Image_ATCICA && fmt <= Image_ATCICI))
        levelSize = Alg::Max<UInt32>(1u, (sz.Width+3) / 4) *
                    Alg::Max<UInt32>(1u, (sz.Height+3) / 4) * 16;
    else
        levelSize = GetFormatPitch(format, sz.Width, plane) * sz.Height;
    return levelSize;
}

UPInt  ImageData::GetMipLevelsSize(ImageFormat format, const ImageSize& sz,
                                   unsigned levels, unsigned plane)
{
    UPInt     levelsSize = 0;
    ImageSize size(sz);

    for(unsigned lv = levels; lv != 0; lv--)
    {
        levelsSize += ImageData::GetMipLevelSize(format, size, plane);
        size = ImageSize_NextMipSize(size);        
    }
    return levelsSize;
}

//--------------------------------------------------------------------
// *** ImageSwizzler
void ImageSwizzler::CacheScanline(ImageSwizzlerContext& ctx, unsigned y)
{
    ctx.pCurrentScanline = ctx.pImage->GetScanline(y);
}

void ImageSwizzler::SetPixelInScanline(ImageSwizzlerContext& ctx, unsigned x, Color c)
{
    SetPixelInScanline(ctx, x, c.ToColor32());
}

void ImageSwizzler::SetPixelInScanline(ImageSwizzlerContext& ctx, unsigned x, UInt32 c)
{
    ctx.pImage->SetPixelInScanline(ctx.pCurrentScanline, x, c);
}

Color ImageSwizzler::GetPixelInScanline(ImageSwizzlerContext& ctx,unsigned x)
{
    return ctx.pImage->GetPixelInScanline(ctx.pCurrentScanline, x);
}

//--------------------------------------------------------------------
void MappedTextureBase::Unmap(bool)
{
    pTexture->pMap = 0;
    pTexture       = 0;
    StartMipLevel  = 0;
    LevelCount     = 0;
}

//--------------------------------------------------------------------

void    ImageBase::CopyScanlineDefault(UByte* pd, const UByte* ps, UPInt size,
                                       Palette*, void*)
{
    memcpy(pd, ps, size);
}


#ifdef SF_AMP_SERVER

UPInt ImageBase::GetBytes(int* memRegion) const
{
    if (memRegion != NULL)
    {   
        *memRegion = 0;
    }
    return ImageData::GetMipLevelSize(GetFormat(), GetSize());
}

UInt32 ImageBase::GetNextImageId()
{
    static Lock staticLock;
    static UInt32 nextImageId = 0;

    Lock::Locker locker(&staticLock);
    return (++nextImageId);
}


#endif  // GFX_AMP_SERVER


//--------------------------------------------------------------------
// ***** Image

Image::~Image()
{
    releaseTexture();
    if (pInverseMatrix)
        SF_FREE(pInverseMatrix);
}

void Image::releaseTexture()
{
    Texture* ptexture = pTexture.Exchange_NoSync(0);
    if (ptexture)
    {
     //   if (pTexture->GetTextureManager() == pUpdateSync)
     //       pUpdateSync = 0;  // NTS?? What if user is calling Update() now?
        ptexture->ImageLost();
        ptexture->Release();
    }
}

void Image::initTexture_NoAddRef(Texture* ptexture)
{
    SF_ASSERT(pTexture == 0);
    pTexture = ptexture;    
}

Image* Image::Create(ImageFormat format, unsigned mipLevels, const ImageSize& size,
                     unsigned use, MemoryHeap* pheap,
                     TextureManager* pmanager, ImageUpdateSync* pupdateSync)
{
    if (!pheap)
        pheap = Memory::GetGlobalHeap();
    if (!pupdateSync)
        pupdateSync = pmanager;

    Image* pimage = 0;

    if (use & ImageUse_GenMipmaps)
    {
        if (mipLevels != 1)
        {
            SF_DEBUG_ERROR(1,
                "Image::Create failed - ImageUse_GenMipmaps requires mipLevels=1");
            return 0;
        }
    }

    if (pmanager && !pmanager->CanCreateTextureCurrentThread())
        pmanager = 0;

    if (pmanager)
    {
        unsigned useCaps = pmanager->GetTextureUseCaps(format);

        // If texture data can't be lost (ImageUse_InitOnly) and is
        // agreeable to our Mapping settings, create a TextureImage directly.
        if ((useCaps & ImageUse_InitOnly) &&
            ((use & ImageUse_Map_Mask) == (useCaps & ImageUse_Map_Mask)))
        {
            pimage = SF_HEAP_NEW(pheap) TextureImage(format, size, use, 0, pupdateSync);
        }
    }

    if (!pimage)
    {
        pimage = RawImage::Create(format, mipLevels, size, use,
                                  pheap, pupdateSync);
        if (!pimage)
            return 0;
    }

    if (pmanager)
    {
        Texture* ptexture = pmanager->CreateTexture(format, mipLevels, size, use,
                                                    pimage);
        if (!ptexture)
        {
            pimage->Release();
            return 0;
        }

        pimage->initTexture_NoAddRef(ptexture);
    }

    return pimage;
}


Image* Image::CreateSubImage(const ImageRect& rect, MemoryHeap* pheap)
{
    return SF_HEAP_NEW(pheap) SubImage(this, rect);
}


// Matrices
void Image::SetMatrix(const Matrix2F& mat, MemoryHeap* heap)
{
    if (!pInverseMatrix)
    {
        pInverseMatrix = (Matrix2F*)SF_HEAP_MEMALIGN(
            heap ? heap : Memory::GetHeapByAddress(this),
            sizeof(Matrix2F), 16, Stat_Image_Mem);
    }
    *pInverseMatrix = mat;
    pInverseMatrix->Invert();
}
void Image::SetMatrixInverse(const Matrix2F& mat, MemoryHeap* heap)
{
    if (!pInverseMatrix)
    {
        pInverseMatrix = (Matrix2F*)SF_HEAP_MEMALIGN(
            heap ? heap : Memory::GetHeapByAddress(this),
            sizeof(Matrix2F), 16, Stat_Image_Mem);
    }
    *pInverseMatrix = mat;
}

void Image::GetMatrix(Matrix2F* mat) const
{
    if (pInverseMatrix)
    {
        *mat = *pInverseMatrix;
        mat->Invert();
    }
    else
    {
        mat->SetIdentity();
    }
}
bool Image::GetMatrixInverse(Matrix2F* mat) const
{
    if (pInverseMatrix)
        *mat = *pInverseMatrix;
    return pInverseMatrix != 0;
}

void Image::GetUVGenMatrix(Matrix2F* mat, TextureManager* manager)
{
    Texture* texture = GetTexture(manager);
    if (texture)
    {
        Matrix2F invImageMatrix(Matrix2F::NoInit);
        if (GetMatrixInverse(&invImageMatrix))
        {
            Matrix2F uvGenMatrix(Matrix2F::NoInit);
            texture->GetUVGenMatrix(&uvGenMatrix);
            mat->SetToAppend(invImageMatrix, uvGenMatrix);
        }
        else
        {
            texture->GetUVGenMatrix(mat);
        }

        ImageRect r = GetRect();
        mat->PrependTranslation((float)r.x1, (float)r.y1);
    }
    else
    {
        mat->SetIdentity();
    }
}

void Image::GetUVNormMatrix(Matrix2F* mat, TextureManager* manager)
{
    Texture* texture = GetTexture(manager);
    if (texture)
    {
        texture->GetUVGenMatrix(mat);
        ImageRect r = GetRect();

        // We need to apply the inverse image matrix, only if this is a sub-image (not the entire area of the texture).
        if (r.GetSize() != texture->GetSize())
        {
            Matrix2F invImageMatrix(Matrix2F::NoInit);
            if (GetMatrixInverse(&invImageMatrix))
                mat->Prepend(invImageMatrix);
        }
        mat->PrependTranslation((float)r.x1, (float)r.y1);
    }
    else
    {
        mat->SetIdentity();
    }

    ImageSize s = GetRect().GetSize();
    mat->PrependScaling((float)s.Width, (float)s.Height);
}

#ifdef SF_AMP_SERVER
UPInt Image::GetBytes(int* memRegion) const
{
    if (pTexture)
    {
        return pTexture->GetBytes(memRegion);
    }
    return ImageBase::GetBytes(memRegion);
}
#endif

//--------------------------------------------------------------------
// ***** ImageSource

// Utility functions for correct initialization of image.
bool ImageSource::IsDecodeOnlyImageCompatible(const ImageCreateArgs& args)
{
    // We don't need to keep Image in memory if:
    //  1. Texture manager supports NoDataLoss initialization and matches
    //     our map flags, since we'll create an init-only texture.
    //  2. If map flags requested, as we'll create RawImage anyway.
    
    if (args.pManager)
    {
        unsigned textureCaps = args.pManager->GetTextureUseCaps(GetFormat());
        if ((args.Use & ImageUse_Map_Mask) == (args.Use & textureCaps & ImageUse_Map_Mask))
        {
            // If we have a texture cache, do not decode the image, it will happen in the update,
            // and then the decompressed texture data can be discarded (and re-decoded if required again).
            if (textureCaps & ImageUse_NoDataLoss && !args.pManager->GetTextureCache())
                return false;
        }
    }
    if (args.Use & ImageUse_Map_Mask)
        return false;

    return true;
}

Image* ImageSource::CreateCompatibleImage(const ImageCreateArgs& args)
{
    ImageFormat       format = (args.Format == Render::Image_None) ? GetFormat() : args.Format;
    ImageUpdateSync*  updateSync = args.GetUpdateSync();
    MemoryHeap*       heap = args.pHeap ? args.pHeap : Memory::GetGlobalHeap();

    // If there is a TextureManager, try to either create an TextureImage directly,
    // or associate it with the image.    
    unsigned imageUse = args.Use;
    unsigned mipmapCount = GetMipmapCount();
    if (mipmapCount > 1)
        imageUse &= (~ImageUse_GenMipmaps);


    if (args.pManager)
    {
        unsigned textureCaps = args.pManager->GetTextureUseCaps(format);

        // If texture data can't be lost (ImageUse_NoDataLosts) and is
        // agreeable to our Mapping settings, create a TextureImage directly.
        if ((args.Use & ImageUse_Map_Mask) == (args.Use & textureCaps & ImageUse_Map_Mask))
        {
            if ((textureCaps & ImageUse_NoDataLoss))
            {
                if ( args.pManager->CanCreateTextureCurrentThread() )
                {
                    // If texture can't be lost 
                    unsigned textureUse = imageUse|ImageUse_InitOnly;
                    Ptr<Texture> ptexture = 
                        *args.pManager->CreateTexture(format, mipmapCount, GetSize(),
                        textureUse, this);
                    if (ptexture)
                    {
                        return SF_HEAP_NEW(heap)
                            TextureImage(format, GetSize(), textureUse, ptexture, updateSync);
                    }
                    return 0;
                }
                else
                {
                    // If the texture could not be created in the renderer immediately because it must
                    // happen on the rendering thread, queue the image data for freeing once the texture 
                    // has been created.
                    imageUse |= ImageUse_InitOnly;
                }
            }
        }
    }

    // We are either Mappable, or temporary was passed -
    // Create Raw image, decoding into it.
    RawImage* rawImage = RawImage::Create(format, mipmapCount, GetSize(), imageUse,
                                          heap, updateSync);
    if (!rawImage) return 0;

    // Read / copy TGA data into RawImage buffer.
    ImageData rawData;
    rawImage->GetImageData(&rawData);
    if (!Decode(&rawData, &Image::CopyScanlineDefault))
    {
        rawImage->Release();
        return 0;
    }
    return rawImage;
}




//--------------------------------------------------------------------
// ***** Raw Image

RawImage::~RawImage()
{
    // Destructor still needs to handle partially-initialized
    // image since it may exist in Create.
    freeData();
}

bool RawImage::hasData() const
{
    for (unsigned plane = 0; plane < Data.RawPlaneCount; plane++)
    {
        if (Data.pPlanes[plane].pData)
        {
            return true;
        }
    }
    return false;
}

void RawImage::freeData()
{
    for (unsigned plane = 0; plane < Data.RawPlaneCount; plane++)
    {
        if (Data.pPlanes[plane].pData)
        {
            SF_FREE(Data.pPlanes[plane].pData);
            Data.pPlanes[plane].pData = 0;
        }
    }
}


RawImage* RawImage::Create(ImageFormat format, unsigned mipLevelCount,
                           const ImageSize& size, unsigned use,
                           MemoryHeap* pheap,
                           ImageUpdateSync* pupdateSync)
{
    if (!pheap)
        pheap = Memory::GetGlobalHeap();

    if (use & ImageUse_GenMipmaps)
    {
        if (mipLevelCount != 1)
        {
            SF_DEBUG_ERROR(1,
                "RawImage::Create failed - ImageUse_GenMipmaps requires mipLevels=1");
            return 0;
        }
    }

    RawImage* pimage = SF_HEAP_NEW(pheap) RawImage;
    if (!pimage)
    {
rawimage_create_memory_error:
        SF_DEBUG_ERROR(1, "RawImage::Create failed - Allocation failure");
        return 0;
    }

    if (!pimage->Data.Initialize(format, mipLevelCount))
    {
        pimage->Release();
        goto rawimage_create_memory_error;
    }

    SF_ASSERT((use & 0xFFFF) == use);
    pimage->Data.Use     = (UInt16) use;
    //pimage->TextureArena = textureArena;
    pimage->pUpdateSync  = pupdateSync;
        
    // Allocate planes.
    for (unsigned plane = 0; plane < pimage->Data.RawPlaneCount; plane++)
    {
        ImageSize planeSize= ImageData::GetFormatPlaneSize(format, size, plane);
        UPInt     pitch    = ImageData::GetFormatPitch(format, planeSize.Width, plane);
        UPInt     dataSize = ImageData::GetMipLevelsSize(format, planeSize,
                                                         mipLevelCount, plane);
        UByte*    pdata = (UByte*)SF_HEAP_ALLOC(pheap, dataSize, Stat_Image_Mem);
        if (!pdata)
        {
            // Free all remaining planes, release image.
            pimage->Release();
            goto rawimage_create_memory_error;
        }
        pimage->Data.SetPlane(plane, planeSize, pitch, dataSize, pdata);
    }

    return pimage;
}

bool RawImage::GetImageData(ImageData* pdata)
{
    // TBD: Should we have copy-constructor like Initialize() instead?
//     pdata->Initialize(Data.GetFormat(), Data.GetMipLevelCount(),
//         Data.pPlanes, Data.RawPlaneCount, Data.HasSeparateMipmaps());
//     pdata->Use = pdata->Use;    
//     pdata->pPalette  = Data.pPalette;
    *pdata = Data;
    return true;
}


void RawImage::SetImageData(ImageData* pdata)
{
    freeData();
    Data = *pdata;
}


bool RawImage::Map(ImageData* pdata, unsigned levelIndex, unsigned levelCount)
{
    if (pdata->Initialize(Data, levelIndex, levelCount))
    {
        Data.Flags |= ImageData::Flag_ImageMapped;
        return true;
    }

    return false;    
}

bool RawImage::Unmap()
{    
    SF_ASSERT(Data.Flags & ImageData::Flag_ImageMapped);
    Data.Flags &= ~ImageData::Flag_ImageMapped;
    return true;
}


// *** Decode

bool RawImage::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
#if defined (SF_AMP_SERVER)
    // If there are no data in system memory, read from the GPU
    if (pTexture && !hasData())
    {
        return pTexture->Copy(pdest);
    }
#endif //SF_AMP_SERVER
    ImagePlane  splane, dplane;
    ImageFormat format           = Data.GetFormat();
    unsigned    formatPlaneCount = ImageData::GetFormatPlaneCount(format);
    unsigned    planeCount       = Data.GetPlaneCount(),
                destPlaneCount   = pdest->GetPlaneCount();
    unsigned    i;

    if (Data.IsHWSpecific())
    {
        // HW-Specific formats are handled as a unit; they must have 
        // have matching size.
        SF_ASSERT((Data.GetFormatNoConv() == pdest->GetFormat()) &&
                  (Data.GetSize() == pdest->GetSize()) &&
                  (planeCount == destPlaneCount));

        for (i = 0; i< planeCount; i++)
        {
            Data.GetPlane(i, &splane);
            pdest->GetPlane(i, &dplane);
            memcpy(dplane.pData, splane.pData, splane.DataSize);
        }
        return true;
    }

    // Generic format have looser constraints as they support data
    // conversion by relying on the provided copyScanline function.
    SF_ASSERT(//(Data.GetFormatNoConv() == pdest->GetFormatNoConv()) && ???
              (Data.GetWidth() <= pdest->GetWidth()) &&
              (Data.GetHeight() <= pdest->GetHeight()) &&
              (formatPlaneCount <= destPlaneCount));
    
    planeCount = Alg::Min(planeCount, destPlaneCount);
        
    for (i = 0; i< planeCount; i++)
    {
        unsigned planeIndex    = planeCount % formatPlaneCount;

        // If there are separate mipmaps, or they only have one level, do a simple scanline copy.
        if ( (Data.HasSeparateMipmaps() ||  Data.GetMipLevelCount() == 1) &&
             (pdest->HasSeparateMipmaps() || pdest->GetMipLevelCount() == 1) )
        {
            Data.GetPlane(i, &splane);
            pdest->GetPlane(i, &dplane);

            // Get number of scan-lines. Typically this is the same
            // as Height but is different for compressed formats.
            UByte*   psource = splane.pData;
            UByte*   pdestin = dplane.pData;
            UPInt    scanlineCount = ImageData::GetFormatScanlineCount(format, splane.Height, planeIndex);
            unsigned linesize = (splane.Width * Data.GetBitsPerPixel()) / 8;
                
            for (UPInt j = 0; j < scanlineCount; j++, psource += splane.Pitch, pdestin += dplane.Pitch)
                copyScanline(pdestin, psource, linesize, Data.pPalette.GetPtr(), arg);
        }
        else
        {
            UByte*   psource = 0;
            UByte*   pdestin = 0;

            // There mip maps are all contained on one plane, copy all of them to the destination plane.
            for (unsigned level =0; level < Alg::Min(Data.LevelCount,pdest->LevelCount); level++ )
            {
                if ( Data.HasSeparateMipmaps() )
                {
                    Data.GetPlane(planeIndex + formatPlaneCount * level, &splane);
                    psource = psource ? psource : splane.pData;
                }
                else
                {
                    Data.GetPlane(formatPlaneCount * level + i, &splane);
                    psource = splane.pData;
                }

                if ( pdest->HasSeparateMipmaps() )
                {
                    pdest->GetPlane(planeIndex + formatPlaneCount * level, &dplane);
                    pdestin = pdestin ? pdestin : dplane.pData;
                }
                else
                {
                    pdest->GetPlane(formatPlaneCount * level + i, &dplane);
                    pdestin = dplane.pData;
                }

                // Get number of scan-lines. Typically this is the same
                // as Height but is different for compressed formats.
                UPInt    scanlineCount = ImageData::GetFormatScanlineCount(format, splane.Height, planeIndex);

                unsigned linesize = (splane.Width * Data.GetBitsPerPixel()) / 8;
                for (UPInt j = 0; j < scanlineCount; j++, psource += splane.Pitch, pdestin += dplane.Pitch)
                    copyScanline(pdestin, psource, linesize, Data.pPalette.GetPtr(), arg);
            }
        }
    }
    return true;
}


Texture* RawImage::GetTexture(TextureManager* pmanager)
{
    if (pTexture && pTexture->GetTextureManager() == pmanager)
    {
        return pTexture;
    }

    if (!pmanager)
        return 0;

    pTexture = 0;

    // Since RawImage does its own mapping, don't pass Map use flags to texture creation.
    Texture* ptexture = 
        pmanager->CreateTexture(Data.Format, Data.LevelCount,
                                Data.GetSize(), (Data.Use &~ImageUse_Map_Mask), this);
    initTexture_NoAddRef(ptexture);
    return ptexture;
}


//--------------------------------------------------------------------
// *** TextureImage
#ifdef SF_AMP_SERVER
bool TextureImage::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    SF_UNUSED2(copyScanline, arg);
    if ( !pTexture )
        return false;

    return pTexture->Copy(pdest);
}
#endif

// TextureImage mapping should only be allowed in single-threaded rendering
// environments, in which case ImageUse_MapSimThread is set.
// Such mapping may, for example, be possible on PSP.
// Note that if mapping is necessary on threaded platforms, Image::Create
// will create RawImage with an attached texture instead.

bool    TextureImage::Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT(pTexture && (Use & ImageUse_MapSimThread));
    return pTexture->Map(pdata, mipLevel, levelCount);
}
bool    TextureImage::Unmap()
{
    return pTexture->Unmap();
}


//--------------------------------------------------------------------
// ***** SubImage

bool SubImage::GetMatrixInverse( Matrix2F* mat ) const
{
    bool baseInverse = pImage->GetMatrixInverse(mat);

    // No inverse matrices set.
    if (!baseInverse && !pInverseMatrix)
        return false;

    // Only subimage inverse is set, use it.
    if (!baseInverse)
        *mat = *pInverseMatrix;

    // Both subimage and base image have an inverse. Append them.
    if (pInverseMatrix && baseInverse)
        mat->Append(*pInverseMatrix);

    // baseInverse && !pInverseMatrix already handled.

    // matrix is non-identity.
    return true; 
}


//--------------------------------------------------------------------
// ***** ImageUpdateQueue

ImageUpdate::ImageUpdate( Image* pimage ) : pImage(pimage)
{

}

ImageUpdateQueue::~ImageUpdateQueue()
{
    for (unsigned i = 0; i<Queue.GetSize(); i++)
    {
        if (Queue[i] & 1)
        {
            Image* pimage = (Image*)(Queue[i] & ~(UpdateRef)1);
            pimage->Release();
        }
        else
        {
            ImageUpdate *pupdate = (ImageUpdate*) Queue[i];
            pupdate->Release();
        }
    }
}

void    ImageUpdateQueue::ProcessUpdates(Render::TextureManager* pmanager)
{
    for (unsigned i = 0; i<Queue.GetSize(); i++)
    {
        if (Queue[i] & 1)
        {
            Image*           pimage   = (Image*)(Queue[i] & ~(UpdateRef)1);
            Render::Texture* ptexture = pimage->GetTexture(pmanager);
            if (ptexture)
                ptexture->Update();
            pimage->Release();
        }
        else
        {
            ImageUpdate *pupdate = (ImageUpdate*) Queue[i];
            pupdate->HandleUpdate(pmanager);
            pupdate->Release();
        }
    }
    Queue.Clear();
}

void ImageUpdateQueue::Add( Image* pimage )
{
    SF_ASSERT(!(((UPInt)pimage)&1));
    Queue.PushBack(((UpdateRef)pimage)|1);
    pimage->AddRef();
}

void ImageUpdateQueue::Add( ImageUpdate* pupdate )
{
    Queue.PushBack((UpdateRef)pupdate);
    pupdate->AddRef();
}

//--------------------------------------------------------------------
// ***** TextureManager
TextureManager::TextureManager(ThreadId renderThreadId, ThreadCommandQueue* commandQueue, TextureCache* textureCache) :
    ServiceCommandInstance(getThis()),
    RenderThreadId(renderThreadId),
    pRTCommandQueue(commandQueue),
    pTextureCache(textureCache),
    pLocks(0)
{
    SF_DEBUG_ASSERT((RenderThreadId == 0 || pRTCommandQueue != 0),
                    "TextureManager - if a RenderThreadId is provided, then a ThreadCommandQueue must also be provided.");

    pLocks = *SF_HEAP_AUTO_NEW(this) TextureManagerLocks(this);
}

TextureManager::~TextureManager()
{
    // Clear out texture formats.
    for (unsigned tf = 0; tf < TextureFormats.GetSize(); ++tf)
        delete TextureFormats[tf];
    TextureFormats.Clear();
}

void TextureManager::UpdateImage(Image* pimage)
{
    Mutex::Locker lock(&pLocks->TextureMutex);
    ImageUpdates.Add(pimage);
}

void TextureManager::UpdateImage(ImageUpdate* pupdate)
{
    Mutex::Locker lock(&pLocks->TextureMutex);
    ImageUpdates.Add(pupdate);
}

void TextureManager::ProcessQueues()
{
    Mutex::Locker lock(&pLocks->TextureMutex);
    processTextureKillList();
    processInitTextures();
    processImageUpdates();
}

void TextureManager::BeginFrame()
{
    ProcessQueues();
    if (pTextureCache)
        pTextureCache->BeginFrame();
}

void TextureManager::EndFrame()
{
    if (pTextureCache)
        pTextureCache->EndFrame();
}

const Render::TextureFormat* TextureManager::precreateTexture(ImageFormat format, unsigned use, ImageBase* pimage)
{
    if (pimage) 
    {
        if ((format & ImageFormat_Mask) != (pimage->GetFormatNoConv() & ImageFormat_Mask))
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - source image format mismatch");
            return 0;
        }
        if ((pimage->GetImageType() == Image::Type_ImageBase) && !(use & ImageUse_InitOnly))
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - ImageUse_InitOnly expected when using ImageBase class");
            return 0;
        }
    }
    else
    {
        if (!(use & (ImageUse_ReadOnly_Mask|ImageUse_RenderTarget)))
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - no source image for read-only ImageUse");
            return 0;
        }
    }

    const Render::TextureFormat* ptformat = getTextureFormat(format);
    if (!ptformat)
    {
        SF_DEBUG_ERROR1(1, "CreateTexture failed - unsupported format %d", format);
        return 0;
    }
    if (ImageData::IsFormatCompressed(format) &&
        (use & (ImageUse_GenMipmaps|ImageUse_Map_Mask|ImageUse_PartialUpdate|ImageUse_RenderTarget)))
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - unsupported Use flags for compressed format");
        return 0;
    }
    if (((use & ImageUse_Map_Mask) != 0) && !isScanlineCompatible(ptformat))
    {
        SF_DEBUG_ERROR1(1, "CreateTexture failed - format %d not mappable", format);
        return 0;
    }
    return ptformat;
}

#if defined( SF_OS_WIN32 ) && !defined(_DURANGO)
#define PUMP_MESSAGES_HACK
#endif

#ifdef PUMP_MESSAGES_HACK
static void NullPumpMessages() {}
typedef void (*MessagePumpFunc)();
MessagePumpFunc FnPumpMessages = &NullPumpMessages;
#endif // PUMP_MESSAGES_HACK

Render::Texture* TextureManager::postCreateTexture( Render::Texture* ptexture, unsigned use )
{
    if (!ptexture)
        return 0;
    if (!ptexture->IsValid())
    {
        ptexture->Release();
        return 0;
    }

    Mutex::Locker lock(&pLocks->TextureMutex);

    if (CanCreateTextureCurrentThread())
    {
        // Before initializing texture, process previous requests, if any.
        processTextureKillList();
        processInitTextures();

        if (ptexture->Initialize())
            Textures.PushBack(ptexture);
    }
    else
    {
        TextureInitQueue.PushBack(ptexture);

        SF_DEBUG_ASSERT(pRTCommandQueue, "Must provide a ThreadCommandQueue to TextureManager constructor. "
            "This platform does not support creating resources, except on the Render thread. It must queue"
            "Texture creation from the Advance thread, and wait for it to be processed on the Render thread.");

        pLocks->TextureMutex.Unlock();
        pRTCommandQueue->PushThreadCommand(&ServiceCommandInstance);
        pLocks->TextureMutex.DoLock();

#ifdef PUMP_MESSAGES_HACK
        while(ptexture->State == Texture::State_InitPending)
		{
            pLocks->TextureInitWC.Wait(&pLocks->TextureMutex, 100 );
			FnPumpMessages();
		}
#else
		while(ptexture->State == Texture::State_InitPending)
			pLocks->TextureInitWC.Wait(&pLocks->TextureMutex);
#endif
    }

    // Clear out 'pImage' reference if it's not supposed to be kept. It is safe to do this
    // without ImageLock because texture hasn't been returned yet, so this is the only
    // thread which has access to it. Also free the data if it is a RawImage.
    if (use & ImageUse_NoDataLoss)
    {
        if ( ptexture->pImage && ptexture->pImage->GetImageType() == Image::Type_RawImage )
            ((Render::RawImage*)ptexture->pImage)->freeData();
        ptexture->pImage = 0;
    }

    // If texture was properly initialized, it would've been added to list.
    if (ptexture->State == Texture::State_InitFailed)
    {
        ptexture->pImage = 0;
        ptexture->Release();
        return 0;
    }

    return ptexture;
}


Render::DepthStencilSurface* TextureManager::postCreateDepthStencilSurface( Render::DepthStencilSurface* pdss )
{
    if (!pdss)
        return 0;

    Mutex::Locker lock(&pLocks->TextureMutex);
    if (CanCreateTextureCurrentThread())
    {
        // Before initializing texture, process previous requests, if any.
        processTextureKillList();
        processInitTextures();
        pdss->Initialize();
    }
    else
    {
        DepthStencilInitQueue.PushBack(pdss);
        if (pRTCommandQueue)
        {
            pLocks->TextureMutex.Unlock();
            pRTCommandQueue->PushThreadCommand(&ServiceCommandInstance);
            pLocks->TextureMutex.DoLock();
        }
        while(pdss->State == Texture::State_InitPending)
            pLocks->TextureInitWC.Wait(&pLocks->TextureMutex);
    }
    return pdss;
}

const Render::TextureFormat* TextureManager::getTextureFormat(ImageFormat format) const
{
    for (unsigned i = 0; i< TextureFormats.GetSize(); i++)    
    {
        if (TextureFormats[i] && TextureFormats[i]->GetImageFormat() == format)
            return (Render::TextureFormat*)TextureFormats[i];
    }
    return 0;
}

MappedTextureBase* TextureManager::mapTexture(Texture* ptexture, unsigned mipLevel, unsigned levelCount)
{
    MappedTextureBase* pmap;
    MappedTextureBase& mappedTexture0 = getDefaultMappedTexture();

    if (mappedTexture0.Reserve())
        pmap = &mappedTexture0;
    else
    {
        pmap = createMappedTexture();
        if (!pmap) return 0;
    }

    if (pmap->Map(ptexture, mipLevel, levelCount))
        return pmap;
    if (pmap != &mappedTexture0)
        delete pmap;
    return 0;
}

void TextureManager::unmapTexture(Texture *ptexture, bool applyUpdate)
{
    MappedTextureBase *pmap = ptexture->pMap;
    pmap->Unmap(applyUpdate);
    MappedTextureBase& mappedTexture0 = getDefaultMappedTexture();
    if (pmap != &mappedTexture0)
        delete pmap;
}


ImageSwizzler& TextureManager::GetImageSwizzler() const
{
    static ImageSwizzler swizzler;
    return swizzler;
}

// ***** Texture

bool Texture::Initialize()
{
    TextureCache* texCache = GetTextureManager()->GetTextureCache();
    if (State == State_Valid && texCache)
        texCache->TextureCreation(this);
    return true;
}

void Texture::LoseManager()
{
    Lock::Locker lock(&pManagerLocks->ImageLock);

    SF_DEBUG_ASSERT(pMap == 0, "Texture::LoseManager Cannot be called while a texture is mapped.");

    RemoveNode();
    pNext = pPrev = 0;

    ReleaseHWTextures();
    State   = State_Dead;    

    // Users can't access Format any more.
    pFormat = 0; 

    if (pImage)
    {
        // TextureLost may release 'this' Texture, so do it last.
        SF_ASSERT(pImage->GetImageType() != Image::Type_ImageBase);
        Image* pimage = (Image*)pImage;
        pImage = 0;
        pimage->TextureLost(Image::TLR_ManagerDestroyed);
    }
}

void Texture::LoseTextureData()
{
    // If we have a DrawableImage, we must unmap it here.
    if (pImage && pImage->GetImageType() == Image::Type_DrawableImage)
    {
        DrawableImage* pdi = reinterpret_cast<DrawableImage*>(pImage);
        pdi->unmapTextureRT();
    }

    // Check that the texture is not user-mapped.
    SF_DEBUG_ASSERT(pMap == 0, "Cannot LoseTextureData while a texture is mapped.");

    // Note: We do not release staging textures, as we assume that they cannot be lost.
    Lock::Locker lock(&pManagerLocks->ImageLock);
    ReleaseHWTextures(false);
    State   = State_Lost;

    if (pImage)
    {
        // TextureLost may release 'this' Texture, so do it last.
        SF_ASSERT(pImage->GetImageType() != Image::Type_ImageBase);
        Image* pimage = (Image*)pImage;
        pimage->TextureLost(Image::TLR_DeviceLost);
    }
}

void Texture::ReleaseHWTextures(bool)
{
    // Notify the TextureCache that this texture's data is gone (if available)
    TextureCache* texCache = GetTextureManager()->GetTextureCache();
    if (texCache)
        texCache->TextureEvicted(this);
    
}

void Texture::ApplyTexture(unsigned stage, const ImageFillMode& fillMode)
{
    if (State == State_Lost)
    {
        bool reinit = Initialize(); SF_UNUSED(reinit);
        SF_DEBUG_WARNING(!reinit, "Texture was lost and could not be reinitialized.");
    }

    SF_UNUSED2(stage, fillMode);
    TextureCache* texCache = GetTextureManager()->GetTextureCache();
    if (texCache)
        texCache->TextureReference(this);
}

bool Texture::Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT((Use & ImageUse_Map_Mask) != 0);
    SF_ASSERT(!pMap);

    if (levelCount == 0)
        levelCount = MipLevels - mipLevel;

    if (!GetTextureManager()->mapTexture(this, mipLevel, levelCount))
    {
        SF_DEBUG_WARNING(1, "Texture::Map failed - couldn't map texture");
        return false;
    }

    pdata->Initialize(GetImageFormat(), levelCount,
        pMap->Data.pPlanes, pMap->Data.RawPlaneCount, true);
    pdata->Use = Use;
    return true;
}

bool Texture::Unmap()
{
    if (!pMap) return false;
    GetTextureManager()->unmapTexture(this);
    return true;
}

#ifdef SF_AMP_SERVER
bool Texture::Copy(ImageData* pdata)
{
    Image::CopyScanlineFunc puncopyFunc = pFormat->GetScanlineUncopyFn();
    if ( !GetTextureManager() || GetFormat() != pdata->Format || !puncopyFunc)
    {
        // - No texture manager, OR 
        // - Output format is different from the source input format of this texture (unexpected, because
        //   we should be copying back into the image's original source format) OR
        // - We don't know how to uncopy this format.
        return false;
    }

    // Map the texture.
    bool alreadyMapped = (pMap != 0);
    unsigned mipCount = GetMipmapCount();
    if (!alreadyMapped && !GetTextureManager()->mapTexture(this, 0, mipCount))
    {
        SF_DEBUG_WARNING(1, "Texture::Copy failed - couldn't map texture");
        return false;
    }
    SF_ASSERT(pMap);

    // Copy the planes into pdata, using the reverse copy function.
    SF_ASSERT( pdata->GetPlaneCount() == pMap->Data.GetPlaneCount() );
    int ip;
    for ( ip = 0; ip < pdata->RawPlaneCount; ip++ )
    {
        ImagePlane splane, dplane;
        pdata->GetPlane(ip, &dplane);
        pMap->Data.GetPlane(ip, &splane);

        ConvertImagePlane(dplane, splane, GetFormat(), ip, puncopyFunc, 0);
    }

    // Unmap the texture, if we mapped it.
    if ( !alreadyMapped )
        GetTextureManager()->unmapTexture(this);

    return true;
}
#endif

bool Texture::Update()
{
    ImageFormat     format   = GetImageFormat();
    TextureManager* pmanager = GetTextureManager();
    bool            rescale  = (TextureFlags & TF_Rescale) ? true : false;
    bool            swMipGen = (TextureFlags & TF_SWMipGen) ? true : false;
    bool            convert  = false;
    ImageData       *psource;
    ImageData       *pdecodeTarget = 0, *prescaleTarget = 0;
    ImageData       imageData1, imageData2;
    Ptr<RawImage>   pimage1, pimage2;
    unsigned        sourceMipLevels = GetMipmapCount(); // May be different from MipLevels

    // Determine whether an image needs to be converted.
    ImageFormat      rescaleBuffFromat = GetImageFormat();
    ImageRescaleType rescaleType = ResizeNone;
    computeUpdateConvertRescaleFlags(rescale, swMipGen, format, rescaleType, rescaleBuffFromat, convert);

    // Texture update proceeds in four (optional) steps (not all platforms are capable of all these steps however):
    //   1. Image::Decode - Done unless rescaling directly from RawImage.
    //   2. Rescale       - Done if non-pow2 textures are not supported as necessary.
    //   3. Convert       - Needed if conversion wasn't applied in Decode.
    //   4. SW Mipmap Gen - Loop to generate SW mip-maps, may also have a convert step.

    // *** 1. Decode from source pImage to Image1/MappedTexture

    Lock::Locker  imageLock(&pManagerLocks->ImageLock);

    // If we have no image, or we are a user allocated texture, then we can't decode.
    if (!pImage || (TextureFlags & TF_UserAlloc))
        return false;

    // Decode is not needed if RawImage is used directly as a source for rescale.
    if (! ((pImage->GetImageType() == Image::Type_RawImage) && rescale) )
    {
        // Determine decodeTarget -> Image1 if rescale / convert will take place
        if (rescale || convert || !pmanager->isScanlineCompatible(pFormat) || !pmanager->mapTexture(this))
        {
            pimage1 = *RawImage::Create(rescaleBuffFromat, sourceMipLevels, ImgSize, 0);
            if (!pimage1) return false;
            pimage1->GetImageData(&imageData1);
            imageData1.Format = (ImageFormat)(format | ImageFormat_Convertible);
            pdecodeTarget = &imageData1;
        }
        else
        {
            pdecodeTarget = &pMap->Data;
        }

        // Decode to decode_Target (Image1 or MappedTexture)
        if (!pImage->Decode(pdecodeTarget, convert ? &Image::CopyScanlineDefault : pFormat->GetScanlineCopyFn()))
        {
            // Note: if decoding failed, still return true, but do not apply the update.
            // This doesn't necessarily mean there is an issue with the image, for instance,
            // with video, a new frame might not be available immediately.
            if (pdecodeTarget == &pMap->Data)
                pmanager->unmapTexture(this, false);
            return true;
        }
        psource = pdecodeTarget;
    }
    else
    {
        ((RawImage*)pImage->GetAsImage())->GetImageData(&imageData1);
        psource = &imageData1;
    }

    // *** 2. Rescale - from source to Image2/MappedTexture
    if (rescale)
    {
        if (convert || !pmanager->isScanlineCompatible(pFormat) || !pmanager->mapTexture(this))
        {
            pimage2 = *RawImage::Create(rescaleBuffFromat, sourceMipLevels, GetTextureSize(), 0);
            if (!pimage2) return false;
            pimage2->GetImageData(&imageData2);
            prescaleTarget = &imageData2;
        }
        else
        {
            prescaleTarget = &pMap->Data;
        }

        if (rescaleType == ResizeNone)
        {
            rescaleType = GetImageFormatRescaleType(format);
            SF_ASSERT(rescaleType != ResizeNone);
        }
        RescaleImageData(*prescaleTarget, *psource, rescaleType);
        psource = prescaleTarget;
    }

    // *** 3. Convert - from temp source to MappedTexture
    unsigned formatPlaneCount = ImageData::GetFormatPlaneCount(format);

    if (convert || !pmanager->isScanlineCompatible(pFormat))
    {
        if (pmanager->isScanlineCompatible(pFormat))
        {
            ImageData* pconvertTarget = 0;
            if (pmanager->mapTexture(this))
                pconvertTarget = &pMap->Data;
            else if (prescaleTarget)
                pconvertTarget = prescaleTarget;
            else
            {
                pimage2 = *RawImage::Create(rescaleBuffFromat, sourceMipLevels, GetTextureSize(), 0);
                if (!pimage2) return false;
                pimage2->GetImageData(&imageData2);
                pconvertTarget = &imageData2;
            }
            ConvertImageData(*pconvertTarget, *psource, pFormat->GetScanlineCopyFn());
        }
        else
        {
            for (unsigned iplane=0; iplane < formatPlaneCount; iplane++)
            {
                ImagePlane splane, tplane;
                UpdateDesc desc;
                psource->GetMipLevelPlane(0, iplane, &splane);

                desc.SourcePlane = splane;
                desc.DestRect = ImageRect(splane.GetSize());
                desc.PlaneIndex = iplane;

                SF_ASSERT(splane.GetSize() == GetTextureSize(iplane));

                Update(&desc);
            }
        }
    }

    // *** 4. Generate Mip-Levels

    if (swMipGen)
    {
        SF_ASSERT(sourceMipLevels == 1);

        // For each level, generate next mip-map from source to target.
        // Source may be either RawImage, Image1/2, or even MappedTexture itself.
        // Target will be Image1/2 if conversion is needed, MappedTexture otherwise.

        for (unsigned iplane=0; iplane < formatPlaneCount; iplane++)
        {
            ImagePlane splane, tplane;
            psource->GetMipLevelPlane(0, iplane, &splane);

            for (unsigned level = 1; level < MipLevels; level++)
            {
                pMap->Data.GetMipLevelPlane(level, iplane, &tplane);

                if (!convert)
                {
                    GenerateMipLevel(tplane, splane, format, iplane);
                    // If we generated directly into MappedTexture,
                    // texture will be used as source for next iteration.
                    splane = tplane;
                }
                else
                {
                    // Extra conversion step means, source has only one level.
                    // We reuse it through GenerateMipLevel, which allows source
                    // and destination to be the same.
                    ImagePlane dplane(splane);
                    dplane.SetNextMipSize();
                    GenerateMipLevel(dplane, splane, format, iplane);
                    ConvertImagePlane(tplane, dplane, format, iplane,
                        pFormat->GetScanlineCopyFn(), psource->GetColorMap());
                    splane.Width  = dplane.Width;
                    splane.Height = dplane.Height;
                }
            }
        }
    }

    if (psource == &pMap->Data)
        pmanager->unmapTexture(this);
    else
        uploadImage(psource);
    return true;

}

UPInt Texture::GetBytes(int* memRegion) const
{
    if (memRegion)
        *memRegion = 1;
    return ImageData::GetMipLevelSize(GetFormat(), GetSize());
}

}}; // namespace Scaleform::Render

