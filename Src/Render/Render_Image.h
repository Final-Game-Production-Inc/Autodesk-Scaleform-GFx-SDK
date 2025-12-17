/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Image.h
Content     :   Image representation.
Created     :   December 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Image_H
#define INC_SF_Render_Image_H

#include "Render/Render_Color.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Matrix2x4.h"
#include "Render/Render_Stats.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Render/Render_ResizeImage.h"
#include "Render/Render_TextureCache.h"

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_AmpInterface.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Threads.h"

namespace Scaleform { namespace Render {

class TextureManagerLocks;
class TextureManager;
class MemoryBufferImage;
class DepthStencilBuffer;
class RenderTargetData;
class TextureCache;
struct ImageFillMode;

// ImageTarget defines a target rendering platform for which this ImageFormat is intended,
// it is stored as part of the format. ImageTarget_Any means that image format is compatible
// with any platform.

enum ImageTarget
{
    ImageTarget_Any       = 0,
    ImageTarget_X360      = 0x2000,
    ImageTarget_PS3       = 0x3000,
    ImageTarget_Wii       = 0x4000,
    ImageTarget_3DS       = 0x5000,
    ImageTarget_PSVita    = 0x6000,
    ImageTarget_WiiU      = 0x7000,  
    ImageTarget_Adreno    = 0x8000,
    ImageTarget_DX11      = 0x9000,
    ImageTarget_Orbis     = 0xA000,

    // Mask returning the 
    ImageTarget_Mask      = 0xF000
};

enum ImageStorage
{
    ImageStorage_Linear   = 0,
    ImageStorage_Swizzle  = 0x10000,
    ImageStorage_Tile     = 0x20000,

    ImageStorage_Mask     = 0x30000
};

// ImageFormats are divided into several groups:
//  1. Generic      - These formats represent directly linearly addressable data;
//                    this data should not have any target bits
//  2. Convertible  - Image should be convertible to a specified format by the 
//                    the Image::Decode function. 
//  3. Target-Specific - Data format is only intended to be interpreted by the
//                     render on the target platform. Usually this will be a
//                     swizzled and byte-converted format.

// Image format lists channels in the order their bytes/words appear in memory,
// so Image_R8G8B8A8 is stored in memory as Red, Green, Blue, Alpha consecutively,
// independent of the format byte order. Note that this doesn't match DWORD
// color representation in either big or little-endian formats.

// D3D compatibility:
// This order of channel assignment matches D3D10 and OpenGL data formats,
// but it is the opposite of D3D9. Our Image_R8G8B8A8 would match D3DFMT_A8B8G8R8
// format in D3D9, DXGI_FORMAT_R8G8B8A8_UNORM under in D3D10, and GL_RGBA in
// OpenGL (TBD - why does Win32 OGL impl do remapping?).
// Note that most common format in D3D9 is D3DFMT_A8R8G8B8, which doesn't
// agree with out format and requires conversion.

// UInt32/DWORD Color compatibility:
// It is common to specify color in 0xAARRGGBB hex format when specifying it
// as a 32-bit integer, a strategy which is used by our Color class, D3DCOLOR,
// HTML color and Flash in both ActionScript and UI. ImageData::SetPixelARGB
// will expect a value in that format.


enum ImageFormat
{
    Image_None          = 0,
    // Generic formats; supported on all platforms.
    Image_R8G8B8A8      = 1,  // Default.
    Image_B8G8R8A8      = 2,  // D3D9 texture compatible; Potentially color DWORD compatible.
    Image_R8G8B8        = 3,
    Image_B8G8R8        = 4,
    Image_A8            = 9,

    // NOTE: If we ever add bit-packed formats, its it intended to name them
    // from low bits to high, as in Image_B5G6R5 for blue being in low bits of word.

    Image_R4G4B4A4      = 20,
    
    // Standard Block-Compressed formats, supported on many platforms.
    Image_DXT1          = 50, // DXT1 compatible (D3D10 -> BC1).
    Image_DXT3          = 51, // DXT3 compatible (D3D10 -> BC2).
    Image_DXT5          = 52, // DXT5 compatible (D3D10 -> BC3).

    Image_BC1           = Image_DXT1,   // Aliases
    Image_BC2           = Image_DXT3,   
    Image_BC3           = Image_DXT5,
    Image_BC7,                // Compressed, D3D11 (+Durango)/GL/Orbis only.

    Image_PVRTC_RGB_4BPP,     // Compressed.
    Image_PVRTC_RGBA_4BPP,    // Compressed.
    Image_PVRTC_RGB_2BPP,     // Compressed.
    Image_PVRTC_RGBA_2BPP,    // Compressed.

    Image_ETC1_RGB_4BPP,      // Compressed, compatible only with Android.
    Image_ETC1_RGBA_8BPP,     // Compressed.
    Image_ETC2_RGB,
    Image_ETC2_RGBA,
    Image_ETC2_RGBA1,         // Punchthrough alpha

    Image_ATCIC,              // Compressed, compatible only with Android (RGB)
    Image_ATCICA,             // Compressed, compatible only with Android (RGB w/explicit A).
    Image_ATCICI,             // Compressed, compatible only with Android (RGB w/interpolated A).

    Image_Compressed_Start = Image_DXT1,
    Image_Compressed_End   = Image_ATCICI,
    
    // Palette8, just 256-color TGA is supported; 
    // being converted to R8G8B8 during loading by default.
    Image_P8            = 100,
    
    // Formats for video textures. Not usable for any other purpose.
    Image_VideoFormat_Start = 200,

    // Image_Y8_U2_V2 is encoded with three separate data planes.
    Image_Y8_U2_V2      = Image_VideoFormat_Start,
    // Image_Y8_U2_V2 is encoded with four separate data planes.
    Image_Y8_U2_V2_A8,
    // Image_Y4_U2_V2 is encoded into one 32bit data plane.
    Image_Y4_U2_V2,

    Image_VideoFormat_End = Image_Y4_U2_V2,

    ImageFormat_Mask    = 0xFFF,
    
    // ** Platform/HW-specific (Swizzled/Packed) formats:

    Image_Begin_HWSpecific     = 0x1000,
                              
    Image_X360_R8G8B8A8        = ImageTarget_X360 | Image_R8G8B8A8 | ImageStorage_Tile,
    Image_X360_A8              = ImageTarget_X360 | Image_A8       | ImageStorage_Tile,
    Image_X360_DXT1            = ImageTarget_X360 | Image_DXT1     | ImageStorage_Tile,
    Image_X360_DXT3            = ImageTarget_X360 | Image_DXT3     | ImageStorage_Tile,
    Image_X360_DXT5            = ImageTarget_X360 | Image_DXT5     | ImageStorage_Tile,
                              
    Image_PS3_R8G8B8A8_SZ      = ImageTarget_PS3 | Image_R8G8B8A8 | ImageStorage_Swizzle,
    Image_PS3_A8_SZ            = ImageTarget_PS3 | Image_A8       | ImageStorage_Swizzle,
    Image_PS3_A8R8G8B8         = ImageTarget_PS3 | Image_R8G8B8A8, //different byte order
    Image_PS3_R8G8B8           = ImageTarget_PS3 | Image_R8G8B8, 
    Image_PS3_A8               = ImageTarget_PS3 | Image_A8,       
    Image_PS3_DXT1             = ImageTarget_PS3 | Image_DXT1,
    Image_PS3_DXT3             = ImageTarget_PS3 | Image_DXT3,
    Image_PS3_DXT5             = ImageTarget_PS3 | Image_DXT5,
                               
    Image_Wii_R8G8B8A8         = ImageTarget_Wii | Image_R8G8B8A8 | ImageStorage_Tile,
    Image_Wii_A8               = ImageTarget_Wii | Image_A8       | ImageStorage_Tile,
    Image_Wii_DXT1             = ImageTarget_Wii | Image_DXT1,
                              
    Image_3DS_R8G8B8A8         = ImageTarget_3DS | Image_R8G8B8A8       | ImageStorage_Swizzle,
    Image_3DS_R8G8B8           = ImageTarget_3DS | Image_B8G8R8         | ImageStorage_Swizzle,
    Image_3DS_A8               = ImageTarget_3DS | Image_A8             | ImageStorage_Swizzle,
    Image_3DS_ETC1             = ImageTarget_3DS | Image_ETC1_RGB_4BPP  | ImageStorage_Tile,
    Image_3DS_ETC1_RGBA        = ImageTarget_3DS | Image_ETC1_RGBA_8BPP | ImageStorage_Tile,
                              
    Image_PSVita_DXT1             = ImageTarget_PSVita | Image_DXT1            | ImageStorage_Swizzle,
    Image_PSVita_DXT3             = ImageTarget_PSVita | Image_DXT3            | ImageStorage_Swizzle,
    Image_PSVita_DXT5             = ImageTarget_PSVita | Image_DXT5            | ImageStorage_Swizzle,

    Image_WiiU_DXT1               = ImageTarget_WiiU   | Image_DXT1            | ImageStorage_Swizzle,
    Image_WiiU_DXT3               = ImageTarget_WiiU   | Image_DXT3            | ImageStorage_Swizzle,
    Image_WiiU_DXT5               = ImageTarget_WiiU   | Image_DXT5            | ImageStorage_Swizzle,

    Image_Orbis_BC1             = ImageTarget_Orbis    | Image_BC1            | ImageStorage_Tile,
    Image_Orbis_BC2             = ImageTarget_Orbis    | Image_BC2            | ImageStorage_Tile,
    Image_Orbis_BC3             = ImageTarget_Orbis    | Image_BC3            | ImageStorage_Tile,
    Image_Orbis_BC7             = ImageTarget_Orbis    | Image_BC7            | ImageStorage_Tile,

    // The desired format can only be obtained through the Decode function;
    // it is not stored directly.
    ImageFormat_Convertible = 0x100000,

    // Defined for convenience.
    Image_Conv_R8G8B8A8     = Image_R8G8B8A8 | ImageFormat_Convertible,
    Image_Conv_B8G8R8A8     = Image_B8G8R8A8 | ImageFormat_Convertible,
    Image_Conv_A8           = Image_A8 | ImageFormat_Convertible,

};

enum ImageFormatSupport
{
    ImageFormats_Standard  = 0x0000,
    ImageFormats_DXT       = 0x0001,
    ImageFormats_PVR       = 0x0008,
    ImageFormats_ATITC     = 0x0010,
    ImageFormats_ETC       = 0x0020,
    ImageFormats_YUV       = 0x0100, // Not reported yet.

    // Platform-specific swizzled compressed formats
    ImageFormats_GXT       = 0x1000,
    ImageFormats_GTX       = 0x2000,
};

typedef Size<UInt32> ImageSize;
typedef Rect<UInt32> ImageRect;

// ImageSize Helper functions.
ImageSize SF_STDCALL  ImageSize_RoundUpPow2(ImageSize sz);
ImageSize SF_STDCALL  ImageSize_NextMipSize(ImageSize sz);
unsigned  SF_STDCALL  ImageSize_MipLevelCount(ImageSize sz);


// ***** ImagePlane

// ImagePlane describes one addressable data plane of an image in memory.
//
// The included DataSize field describes the size of pData buffer in bytes.
// Although for a typical image DataSize == Height * Pitch, that is not so
// for two cases:
//  a) For image with multiple consecutive mip-levels, DataSize will be
//     larger as it includes all mip-levels.
//  b) For a hardware format image, Pitch may be 0, while DataSize describes
//     the size of HW-format image data pointer pointed to by pData.

struct ImagePlane
{
    UInt32      Width, Height;
    UPInt       Pitch;      // Size of one image scan-line, in bytes.
    UPInt       DataSize;   // Size of buffer at pData, in bytes.
    UByte*      pData;

    ImagePlane()
        : Width(0), Height(0), Pitch(0), DataSize(0), pData(0) { }
    ImagePlane(UInt32 width, UInt32 height, UPInt pitch, UPInt dataSize = 0, UByte* pdata = 0)
        : Width(width), Height(height), Pitch(pitch), DataSize(dataSize), pData(pdata) { }
    ImagePlane(const ImageSize &sz, UPInt pitch, UPInt dataSize = 0, UByte* pdata = 0)
        : Width(sz.Width), Height(sz.Height), Pitch(pitch), DataSize(dataSize), pData(pdata) { }
    ImagePlane(const ImagePlane& src)
        : Width(src.Width), Height(src.Height),
          Pitch(src.Pitch), DataSize(src.DataSize), pData(src.pData) { }

    void        SetData(UInt32 w, UInt32 h, UPInt pitch, UPInt dataSize = 0, UByte* pdata = 0)
    {
        Width   = w;
        Height  = h;
        Pitch   = pitch;
        DataSize= dataSize;
        pData   = pdata;
    }
    void        SetData(const ImageSize &sz, UPInt pitch, UPInt dataSize = 0, UByte* pdata = 0)
    {
        Width   = sz.Width;
        Height  = sz.Height;
        Pitch   = pitch;
        DataSize= dataSize;
        pData   = pdata;
    }

    ImageSize   GetSize() const
    {
        return ImageSize(Width, Height);
    }
    void        SetSize(const ImageSize &sz)
    {
        Width   = sz.Width;
        Height  = sz.Height;
    }
    void        SetSize(UInt32 width, UInt32 height)
    {
        Width   = width;
        Height  = height;
    }

    void        SetNextMipSize()
    {
        Width  = Alg::Max<UInt32>(1, Width/2);
        Height = Alg::Max<UInt32>(1, Height/2);
    }

    void        operator = (const ImagePlane& src)
    {
        Width  = src.Width;
        Height = src.Height;
        Pitch  = src.Pitch;
        DataSize=src.DataSize;
        pData  = src.pData;
    }

    bool        MatchSizes(const ImagePlane& other)
    {
        return (Width == other.Width) && (Height == other.Height) &&
               (Pitch == other.Pitch) && (DataSize == other.DataSize);
    }

    // Obtains an ImagePlane for the specified mipmap level (level == 0 refers to us).
    void            GetMipLevel(ImageFormat format, unsigned level, ImagePlane* p, unsigned plane = 0) const;

    // Scan-line access.
    UByte*          GetScanline(unsigned y)         { SF_ASSERT(y < Height); return pData + Pitch * y; }
    const UByte*    GetScanline(unsigned y) const   { SF_ASSERT(y < Height); return pData + Pitch * y; }
};


// ***** ImageUse

// ImageUse describes how image is intended to be used, or what capabilities the
// image and its associated texture should have. Use flags are associated with each
// image and texture, and are passed to Image::Create and TextureManager::CreateTexture.
// For best performance and memory use, the smallest necessary number of Use flags
// should be applied.
//
// For textures, ImageUse flags are also used to determine what capabilities the
// renderer supports for a given texture type, as reported by TextureManager::GetTextureUseCaps.
// If a specified flag is not reported by this function, it should not be used
// to create a texture.
//
// Even if a certain ImageUse flag is not supported directly by the TextureManager, it may
// still be applicable to an Image. ImageUse_MapSimThread flag, for example, can not easily be
// supported by TextureManager implementations together with threaded rendering. It can,
// however, still be applied to an Image, since in that case it will be emulated by
// maintaining an additional system-memory copy of image data.

enum ImageUse
{
    // Image can be wrapped during rendering. If this flag isn't specified, coordinate
    // wrapping will not work for the image. This flag MUST be supported by all renderers.
    ImageUse_Wrap            = 0x0001,

    // Set this flag if texture mipmap pyramid should automatically be created for
    // the image on renderer side. This flag can't be specified together with 
    // ImageUse_PartialUpdate; also MipLevels should be 1.
    // If Update takes place, all of the mipmap levels will be re-generated.
    ImageUse_GenMipmaps      = 0x0002,

    // Update flags specify whether Update() can be called on the image, potentially
    // uploading new data due to changes. If Update flag is not set, image data is
    // assumed not to change.
    ImageUse_Update          = 0x0010,     // Image can be updated in full.
    ImageUse_PartialUpdate   = 0x0020,     // Image can support sub-rectangle update.

    // ImageUse Map flags describe whether Image::Map and Texture::Map can be used to
    // obtain the pointer to image data for updating.
    //
    //  ImageUse_MapSimThread - Indicates that Image::Map can be called from Advance/modify
    //      thread. This flag typically should NOT be set, as it will cause additional
    //      system-memory copy of the image to exist in threaded environments.
    //      ImageUse_MapSimThread may be reported by TextureManager::GetTextureCap only in
    //      single threaded environments.
    //  
    //  ImageUse_MapRenderThread - Indicates that Texture::Map can be called within the
    //      update handler (executed on render thread only). This will typically be useful
    //      for updating font caches, etc; however, some renderers may not support
    //      this and require relying on ImageUse_PartialUpdate instead. 
    ImageUse_MapSimThread    = 0x0040,
    ImageUse_MapRenderThread = 0x0080,
    ImageUse_Map_Mask        = ImageUse_MapSimThread | ImageUse_MapRenderThread,

    // ImageUse_ReadOnly_Mask - Combination of flags that specify that a texture must be
    //      initialized with an image, because it is not modifiable in any other way.
    ImageUse_ReadOnly_Mask   = (ImageUse_Update|ImageUse_PartialUpdate|ImageUse_Map_Mask),

    // ImageUse_InitOnly indicates that Texture for the image should not hold an Image
    // class pointer internally. When reported as a capability, it means that the renderer
    // will never lose texture data, so Image backing store isn't necessary. This is the
    // behavior of textures loaded from the default implementation of GFx::ImageCreator. 
    ImageUse_InitOnly       = 0x0100,
    ImageUse_NoDataLoss     = ImageUse_InitOnly,  // Alias.

    // Texture associated with this image can be used as a render target.
    ImageUse_RenderTarget    = 0x0400
};


// ***** Palette

// Palette class represents a table of colors, also known as a color map. Palette is
// usually allocated for a Image_P8 format. Palette is ref-counted so that it can be easily
// passed and stored in ImageData. After palette is created, the number of colors
// in it can no longer be changed.

class Palette
{
    mutable AtomicInt<int> RefCount;
protected:
    UInt16          ColorCount;
    bool            HasAlphaFlag;
    Color           Colors[1];

public:    
    bool            HasAlpha() const        { return HasAlphaFlag; }
    unsigned        GetColorCount() const   { return ColorCount; }
    const Color*    GetColors() const       { return Colors; }    
    Color&          operator[] (unsigned i)       { SF_ASSERT(i < ColorCount); return Colors[i]; }
    const Color&    operator[] (unsigned i) const { SF_ASSERT(i < ColorCount); return Colors[i]; }

    // Lifetime - separate implementation due to Create function
    void AddRef()   { RefCount ++; }
    void Release()  { if ((RefCount.ExchangeAdd_NoSync(-1) - 1) == 0) SF_FREE(this); }

    bool            operator == (const Palette& other) const
    {
        return (ColorCount == other.ColorCount) &&
               (HasAlpha() == other.HasAlpha()) &&
               (memcmp(Colors, other.Colors, ColorCount * sizeof(Color)) == 0);
    }
    bool            operator != (const Palette& other) const
    {
        return !operator == (other);
    }

    // Creates a new color map of specified size.
    static Palette* SF_STDCALL Create(unsigned colorCount, bool hasAlpha, MemoryHeap* pheap = 0);    
};



// ***** ImageData

// ImageData describes image data in addressable memory that can accessed and modified
// directly. ImageData is usually initialized by Image::Map or Texture::Map functions;
// it can also be created as a temporary on stack.
//
// ImageData can include any number of channel and mip-level planes, addressable
// individually based on plane index. Many standard format images will have only one
// data plane, plane 0. The size of the image is the size of its 0th plane.
//
// Right now, it is assumed that there is a fixed relationship between image size
// and the size of all its extra planes, as reported by GetFormatPlaneSize. In the
// future this may change, which would requre extra Image::Create functions.
//
// Image planes are numbered first by the order of data channels in ImageFormat, then
// by mip-map level index. Image_R8G8B8A8, for example, has only one channel, thus the
// index of a plane and mipmap is the same. Image_Y8_U2_V2, however, has three data planes,
// indexed as {Y = 0, U = 1, V = 2}. If such an image also has mip-map levels, the U plane
// of a second mipmap (first down from the top level), would have an index of 5.
//
// For efficiency, ImageData stores one ImagePlane directly as a data member, allocating
// additional planes only when they are needed. For standard system-memory formats, additional 
// mipmap data is stored immediately following the top level data, so its location and
// size can be computed without keeping additional ImagePlane structure for every level.
// Such compact plane storage is used by default unless "SeparateMipmaps" flag is specified,
// so a call to GetPlane() actually computes the requested memory location based on
// the top-level plan and its format.

class ImageData
{
public:
    ImageFormat Format;         // Format of the image data.
    unsigned    Use;            // ImageUse flags (Wrappable, Mappable?)
    UByte       Flags;          // ImageData flags.
    UByte       LevelCount;     // Number of mipmap levels.
    UInt16      RawPlaneCount;  // Number of indexable ImagePlane structure at pPlanes.
    ImagePlane* pPlanes;        // Array of ImagePlane structures, potentially allocated.
    Ptr<Palette> pPalette;      // Array of colors for a palette-based image, null if not available.
        
    enum {
        Flag_SeparateMipmaps  = 0x01, // Set if mip levels have separately allocated planes.
        Flag_AllocPlanes      = 0x02, // Set if pPlanes were allocated.
        Flag_LocalHeap        = 0x04, // Set of plane mem is to be allocated from local heap.
        // Used by Image classes:
        Flag_ImageMapped      = 0x10
    };

private:
    ImagePlane  Plane0;
    
    bool    allocPlanes(ImageFormat format, unsigned mipLevelCount, bool separateMipmaps = false);
    void    freePlanes();    

public:
    
    ImageData()
        : Format(Image_None), Use(0), Flags(0), LevelCount(0), RawPlaneCount(1),
          pPlanes(&Plane0) { }

    ImageData(ImageFormat format, unsigned mipLevelCount = 1, bool separateMipmaps = false)
        : Format(format), Use(0), Flags(0), LevelCount((UByte)mipLevelCount), RawPlaneCount(1),
          pPlanes(&Plane0)
    {
        allocPlanes(format, mipLevelCount, separateMipmaps);
    }

    ImageData(ImageFormat format, UInt32 width, UInt32 height, UPInt pitch, UByte* pdata = 0)
        : Format(format), Use(0), Flags(0), LevelCount(1), RawPlaneCount(1),
          pPlanes(&Plane0),
          Plane0(width, height, pitch, pitch * height, pdata)
    {
        SF_ASSERT(GetFormatPlaneCount(format) == 1);
    }

    ~ImageData()
    {
        freePlanes();
    }

    ImageData& operator=(const ImageData& rhs);

    // Initialized ImageData to Image_None with no planes.
    void    Clear();


    // *** Initialize

    // Initialize functions used to completely re-initialize ImageData, clearing all
    // values, changing format and potentially allocating the needed number of planes.

    // Initializes the ImageData object to accommodate the specified image format
    // and number of mip-maps. If separateMipmaps is specified, stores all mipmaps
    // in a single raw data plane.
    bool    Initialize(ImageFormat format, unsigned mipLevels,
                       bool separateMipmaps = false)
    {
        Clear();
        return allocPlanes(format, mipLevels, separateMipmaps);
    }
    
    // Initializes the ImageData object to the specified image format and number of 
    // mipmaps, while also initializing it to use pre-allocated raw data planes.
    // User is responsible for the lifetime of pplanes buffer, as Initialize does not
    // copy or free it.
    void    Initialize(ImageFormat format, unsigned mipLevels,
                       ImagePlane* pplanes, unsigned planeCount, bool separateMipmaps = false)
    {
        SF_ASSERT(planeCount == (GetFormatPlaneCount(format) * (separateMipmaps ? mipLevels : 1)));
        SF_ASSERT(mipLevels < 256);
        Clear();
        Format        = format;
        LevelCount    = (UByte)mipLevels;
        pPlanes       = pplanes;
        RawPlaneCount = (UInt16)planeCount;
        if (separateMipmaps)
            Flags |= Flag_SeparateMipmaps;

        // Fill out Plane0 with the first plane, if there is only one.
        if (pplanes && planeCount==1)
            Plane0 = pplanes[0];
    }

    // Initializes the ImageData object to the specified format, size and data. Image format
    // must have only one data plane.
    void    Initialize(ImageFormat format, UInt32 width, UInt32 height, UPInt pitch,
                       UPInt dataSize, UByte* pdata = 0)
    {
        Clear();
        Format        = format;
        LevelCount    = 1;
        RawPlaneCount = 1;
        Plane0.SetData(width, height, pitch, dataSize, pdata);
    }

    void    Initialize(ImageFormat format, UInt32 width, UInt32 height,
                       UPInt pitch, UByte* pdata = 0)
    {
        Initialize(format, width, height, pitch, height * pitch, pdata);
    }

    // Initializes ImageData to a sub-range of levels from source, which should have valid planes.
    // Shouldn't fail unless allocPlanes() does.
    bool    Initialize(const ImageData& source, unsigned levelIndex, unsigned levelCount);

    Palette*  GetColorMap() const { return pPalette.GetPtr(); }
    
    // SetPlaneData sets plane data contents. When separateMipmaps are not used, mipmap
    // planes past 0 can not be indexed.
    void            SetPlane(unsigned plane, const ImagePlane& src)
    {
        SF_ASSERT(plane < RawPlaneCount);
        pPlanes[plane] = src;
    }
    void            SetPlane(unsigned plane, const ImageSize &sz, UPInt pitch,
                             UPInt dataSize = 0, UByte* pdata = 0)
    {
        SF_ASSERT(plane < RawPlaneCount);
        pPlanes[plane].SetData(sz, pitch, dataSize, pdata);
    }

    // Configure planes & mipmaps
    unsigned        GetPlaneCount() const;
    void            GetPlane(unsigned index, ImagePlane* pplane) const;
    
    // Returns reference to the plane. This function can't be used to reference mipmap level > 0
    // if Flag_SeparateMipmaps is not used, since in that case mipmap planes are virtualized.
    ImagePlane&     GetPlaneRef(unsigned plane = 0)          { SF_ASSERT(plane < RawPlaneCount); return pPlanes[plane]; }
    const ImagePlane& GetPlaneRef(unsigned plane = 0) const  { SF_ASSERT(plane < RawPlaneCount); return pPlanes[plane]; }

    unsigned        GetMipLevelCount() const { return LevelCount; }
    void            GetMipLevelPlane(unsigned mipLevel, unsigned plane, ImagePlane* pplane) const
    {
        GetPlane(mipLevel * GetFormatPlaneCount(Format) + plane, pplane);
    }

    ImageFormat     GetFormat() const       { return Format; }
    ImageFormat     GetFormatNoConv() const { return (ImageFormat)(Format & ~ImageFormat_Convertible); }

    bool            IsCompressed() const { return Format >= Image_Compressed_Start && Format <= Image_Compressed_End; }
    bool            IsHWSpecific() const { return GetFormatNoConv() >= Image_Begin_HWSpecific; }
    bool            HasSeparateMipmaps() const { return (Flags & Flag_SeparateMipmaps) != 0; }

    unsigned        GetBitsPerPixel() const    { return GetFormatBitsPerPixel(Format); }
    UPInt           GetPitch() const            { return pPlanes[0].Pitch; }
    UInt32          GetWidth() const            { return pPlanes[0].Width; }
    UInt32          GetHeight() const           { return pPlanes[0].Height; }
    ImageSize       GetSize() const             { return pPlanes[0].GetSize(); }
    UByte*          GetDataPtr() const          { return pPlanes[0].pData; }

    // Scan-line access
    UByte*          GetScanline(unsigned y)         { return pPlanes[0].GetScanline(y); }
    const UByte*    GetScanline(unsigned y) const   { return pPlanes[0].GetScanline(y); }
    
    // Compute a hash code based on image contents.  Can be useful    
    // for comparing images. Will return 0 if pData is null.
    UPInt           ComputeHash() const;

    // Ensures that all aspects of the image match up, including layout.
    bool            operator == (const ImageData& other) const;

    // Set pixel, sets only the appropriate channels
    void            SetPixelAlpha(unsigned x, unsigned y, UByte alpha);    
    void            SetPixelARGB(unsigned x, unsigned y, UInt32 color);
    void            SetPixelInScanline( UByte * pline, unsigned x, UInt32 color );

    void            SetPixelRGBA(unsigned x, unsigned y, UByte r, UByte g, UByte b, UByte a)
    { SetPixelARGB(x,y, ((UInt32)b) | (((UInt32)g)<<8) | (((UInt32)r)<<16) | (((UInt32)a)<<24) ); }

    // Get pixels
    Color           GetPixel(unsigned x, unsigned y) const;
    Color           GetPixelInScanline( const UByte * pline, unsigned x ) const;

    void            Read(File& file, UInt32 version);
    void            Write(File& file, UInt32 version) const;

    static bool     IsFormatCompressed(ImageFormat format)
    { unsigned baseFormat = format & ImageFormat_Mask; return baseFormat >= Image_Compressed_Start && baseFormat <= Image_Compressed_End; }

    static unsigned SF_STDCALL  GetFormatPlaneCount(ImageFormat fmt);
    static unsigned SF_STDCALL  GetFormatBitsPerPixel(ImageFormat fmt, unsigned plane = 0);
    static UPInt  SF_STDCALL    GetFormatPitch(ImageFormat fmt, UInt32 width, unsigned plane = 0);
    static UPInt  SF_STDCALL    GetFormatScanlineCount(ImageFormat fmt, UInt32 height, unsigned plane = 0);
    static ImageSize SF_STDCALL GetFormatPlaneSize(ImageFormat fmt, const ImageSize& sz, unsigned plane = 0);
    static UPInt  SF_STDCALL    GetMipLevelSize(ImageFormat format, const ImageSize& sz, unsigned plane = 0);
    static UPInt  SF_STDCALL    GetMipLevelsSize(ImageFormat format, const ImageSize& sz,
                                                 unsigned levels, unsigned plane = 0);
};

// ***** ImageSwizzler

// ImageSwizzler is a wrapper around ImageData accessor functions so as to abstract
// away certain swizzled texture formats when dealing with DrawableImage functions.
// 
// The default implementation is a simple passthrough to the relevant ImageData
// accessors since in most cases the texel data is mapped linearly in memory.
// On other platforms, however, the texel data may be swizzled, and so additional
// logic is necessary. ImageSwizzlerContext exists so that there is an ImageSwizzler singleton,
// and the context holds the operational data in the location where the operations are performed.
struct ImageSwizzlerContext;

class ImageSwizzler
{
public:
    virtual ~ImageSwizzler() { }
    virtual void Initialize(ImageSwizzlerContext&) { } 
    virtual void CacheScanline(ImageSwizzlerContext& ctx, unsigned y);
    virtual void SetPixelInScanline(ImageSwizzlerContext& ctx, unsigned x, Color c);
    virtual void SetPixelInScanline(ImageSwizzlerContext& ctx, unsigned x, UInt32 c);
    virtual Color GetPixelInScanline(ImageSwizzlerContext& ctx,unsigned x);
};

struct ImageSwizzlerContext
{
    ImageSwizzlerContext(ImageSwizzler& swizzler, ImageData *d) : 
        Swizzler(swizzler), pCurrentScanline(0), pImage(d), CachedBlockY(0), CachedBlockLine(0), pPlane(0) 
    {
        Swizzler.Initialize(*this);
    }

    // Any data that may be required on any platform should be placed here.
    ImageSwizzler&  Swizzler;
    UByte *         pCurrentScanline;
    ImageData *     pImage;    
    unsigned        CachedBlockY;
    const int *     CachedBlockLine;
    ImagePlane *    pPlane;

    void CacheScanline(unsigned y)                  { Swizzler.CacheScanline(*this, y); }
    void SetPixelInScanline(unsigned x, Color c)    { Swizzler.SetPixelInScanline(*this, x, c.ToColor32()); }
    void SetPixelInScanline(unsigned x, UInt32 c)   { Swizzler.SetPixelInScanline(*this, x, c); }
    Color GetPixelInScanline(unsigned x)            { return Swizzler.GetPixelInScanline(*this, x); }

    // Hide warning
    ImageSwizzlerContext& operator= (const struct ImageSwizzlerContext& other) { SF_UNUSED(other); return *this; }
};

// *** TextureManagerLocks contains TextureMutex and other synchronization objects.
// These are separated into a separate object to allow proper synchronization
// when Texture and TextureManager objects die out of order on different threads.

class TextureManagerLocks : public RefCountBase<TextureManagerLocks, StatRender_TextureManager_Mem>
{
public:
    TextureManager* pManager;

    // Lock obtained when pImage within any texture is used/cleared.
    Lock            ImageLock;
    // Mutex obtained when TextureManager::Textures/InitTextureQueue is modified.
    Mutex           TextureMutex;
    // Notified when TextureInitQueue is processed.
    WaitCondition   TextureInitWC;  

    TextureManagerLocks(TextureManager* pmanager) : pManager(pmanager) { }
    ~TextureManagerLocks() { }
};

struct TextureFormat;

// *** MappedTextureBase
// MappedTexture object represents a Texture mapped into memory with Texture::Map() call;
// it is also used internally during updates.
// The key part of this class is the Data object, stored Locked texture level plains.
class Texture;
class MappedTextureBase : public NewOverrideBase<StatRender_TextureManager_Mem>
{
    friend class Texture;
public:
    MappedTextureBase()
        : pTexture(0), StartMipLevel(false), LevelCount(0) { }
    virtual ~MappedTextureBase()
    {
        SF_ASSERT(!IsMapped());
    }

    bool            Reserve()  { AtomicOps<int>::CompareAndSet_Sync(&LevelCount, 0, -1); return LevelCount == -1; }
    bool            IsMapped() { return (LevelCount > 0); }
    virtual bool    Map(Texture* ptexture, unsigned mipLevel, unsigned levelCount) = 0;
    virtual void    Unmap(bool applyUpdate = true);

    Texture*      pTexture;
    // We support mapping sub-range of levels, in which case
    // StartMipLevel may be non-zero.
    unsigned      StartMipLevel;
    int           LevelCount;
    // Pointer data that can be copied to.
    ImageData     Data;

    enum { PlaneReserveSize = 4 };
    ImagePlane    Planes[PlaneReserveSize];
};

// Texture is a hardware version of image that is typically allocated from a HAL-specific
// renderer's TextureManager and is managed by the rendering thread. Before Image data
// can be rendered it needs to be transfered into a Texture, as only textures are accepted
// by the HAL.
//
// Texture can typically be created/destroyed by either advance or rendering thread,
// as its objects memory management is thread-safe. The Map/Unmap and Update operations
// can, however, only be executed by the rendering thread.

class HAL;
class TextureManager;
class ImageBase;
class Image;

class Texture : public RefCountBase<Texture, StatRender_TextureManager_Mem>, public ListNode<Texture>
{
public:
    // Describes the lifetime state that the texture is in.
    enum CreateState
    {
        State_InitPending,
        State_InitFailed,
        State_Valid,
        State_Lost,
        State_Dead  // Texture becomes Dead once Manager is destroyed.
    };

    // Bits stored in TextureFlags (some of these may only apply to certain platforms).
    enum TextureFlagBits
    {
        TF_Rescale     = 0x01,
        TF_SWMipGen    = 0x02,
        TF_UserAlloc   = 0x04,
        TF_NoRefCount  = 0x08,
        TF_DoNotDelete = 0x10,
    };

    Texture(TextureManagerLocks* pmanagerLocks, const ImageSize& size, UByte mipLevels, UInt16 use, ImageBase* pimage, const TextureFormat* pformat) : 
        ListNode<Texture>(),
        pManagerLocks(pmanagerLocks), pImage(pimage), ImgSize(size), State(State_InitPending), 
        MipLevels(mipLevels), TextureCount(1), Use(use), TextureFlags(0), pMap(0), pFormat(pformat) { };

    virtual ~Texture() { };

    TextureManager* GetTextureManager() const { return pManagerLocks ? pManagerLocks->pManager : 0; };

    // Initialize is a platform-specific method, that will actually create the API specific texture object.
    // It will also copy the contents of the Texture's Image into this newly created object. This also supports
    // user created Texture objects, which expect to be provided the API specific objects.
    virtual bool            Initialize() = 0;

    // IsValid determines whether the Initialize call has already been made, and if it was successful.
    virtual bool            IsValid() const = 0;

    // Return Image object associated with texture on initialization, if not null.
    // Although ImageBase is used on texture initialization, only Image objects are kept
    // and accessible through this function.
    virtual Image*          GetImage() const = 0;
    virtual ImageFormat     GetFormat() const = 0;
    virtual ImageFormat     GetImageFormat() const;
    virtual ImageSize       GetTextureSize(unsigned plane =0) const = 0;
    inline  ImageSize       GetSize() const              { return ImgSize; }
    inline  unsigned        GetMipmapCount() const       { return (Use & ImageUse_GenMipmaps) ? 1 : MipLevels; }
    inline  unsigned        GetPlaneCount() const;
    inline  unsigned        GetTextureStageCount() const { return TextureCount; }

    // Fills in a matrix that should be used to convert pixel coordinate space
    // to the UV coordinates used by the renderer. RenderHALs will override this
    // to properly handle cases when texture coordinates are padded and/or scaled.
    virtual void            GetUVGenMatrix(Matrix2F* mat) const
    { ImageSize sz = GetSize();
      *mat = Matrix2F::Scaling(1.0f / (float)sz.Width, 1.0f / (float)sz.Height);}

    // Notifies texture that its image is about to be unavailable. Texture can
    // only access the image before this function is called. This function must be
    // called for any texture created with Image pointer but without the ImageUse_InitOnly
    // flag, or ~Texture will assert.
    virtual void            ImageLost()
    {
        Lock::Locker lock(&pManagerLocks->ImageLock);
        pImage = 0;
    }

    // LoseManager is called from ~TextureManager, cleaning out most of the
    // texture internal state. Clean-out needs to be done during a Lock,
    // in case ~Texture is called due to release in another thread.
    virtual void            LoseManager();

    // Called after texture data has been lost. This only happens on certain platforms,
    // (for example D3D9 when the device is lost), and signals that the texture should
    // free any resources required to continue (ie. dynamic textures in D3D9).
    virtual void            LoseTextureData();

    // Causes the hardware textures associated with this texture to be released.
    // This can be called from either thread; if not called on the Render thread, the
    // image destruction will be queued if required by the platform. If the parameter
    // is false, Staging textures (used with mappable rendertargets), not freed.
    virtual void            ReleaseHWTextures(bool staging = true) = 0;

    // *** RenderThread-only functions

    // Applies a texture to the texturing pipeline. The stageIndex indicates which stage the 
    // texture should be applied to, its exact meaning is platform-specific. The fm parameter
    // determines the texture modes that the texture should be applied with.
    virtual void            ApplyTexture(unsigned stageIndex, const ImageFillMode& fm) = 0;

    // Maps the texture data planes into the process address space so they can be accessed
    // directly; should be called only from the rendering thread. Typically, the mapping
    // is done by the Update packet. On success, image plane data is stored into the provided
    // ImageData structure.
    //  - All of the data planes for the specified mip-level are mapped simultaneously,
    //    it is not possible to map an individual plane.
    //  - For single data-plane images with mipmaps, top mipmap level index is 0, with
    //    subsequent mipmap having an index of 1 and so on.
    //  - HW-specific formats are always mapped as a single plane, with Pitch reported as 0.
    //    It is expected that full DataSize amount of data will be written into the texture.
    //  - Default levelCount value of 0 means that all levels should be mapped.
    //  - Typically called on render thread (legal if ImageUse_MapSimThread was specified);
    //    back end may allow calling from simulation thread id ImageUse_MapSimThread is
    //    provided.
    //  - A render texture can be used as a rendering source while it is Mapped on the
    //    external API level (CPU modification of it may have undefined effect until it
    //    us unmapped). This is allowed for efficient support of multi-threaded rendering
    //    and DrawableImage implementation. Texture can NOT be used as a render target while
    //    it is mapped.    
    virtual bool            Map(ImageData* pdata, unsigned mipLevel = 0, unsigned levelCount = 0);
    virtual bool            Unmap();
    SF_AMP_CODE(
        virtual bool        Copy(ImageData* pdata);
    )

    // Synchronizes RenderTarget and Staging content.
    //  - allows mapped changes to be transferred to HW without explicit Unmap call
    virtual bool            UpdateRenderTargetData(Render::RenderTargetData*, Render::HAL* = 0) { return false; }
    virtual bool            UpdateStagingData(Render::RenderTargetData*) { return false; }

   
    // Update descriptor provides source and destination data for Update.
    struct UpdateDesc
    {
        ImagePlane SourcePlane;   // Source size, pitch and data pointer.
        ImageRect  DestRect;      // Destination rectangle, must fit in source and destination.
        unsigned   PlaneIndex;    // Index of ImageFromat plane; 0 for single-plane formats.
    };

    // Update texture by copying the provided data into it, only one mip-Level
    // can be update at a time.
    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1,
                                   unsigned mipLevel = 0) = 0;        
    // Does a full image update by calling the associated Image::Decode.
    virtual bool            Update();

    // Returns the size in bytes the underlying system uses to store the texture data.
    virtual UPInt           GetBytes(int* memRegion) const;

protected:

    // Determines whether an image needs to be converted in its Update step, and what type of rescaling
    // needs to be performed. 
    virtual void            computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, 
                                                              ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert ) = 0;
    virtual void            uploadImage(ImageData*) { };

    Ptr<TextureManagerLocks> pManagerLocks; // Texture synchronization object.

public:

    ImageBase*               pImage;        // Source image for the texture.
    ImageSize                ImgSize;       // Size of the image.
    CreateState              State;         // The current lifetime state of the texture.
    UByte                    MipLevels;     // Number of mip levels in the texture.
    UByte                    TextureCount;  // Number of hardware textures this texture consists of (multiple with YUV[A] textures).
    UInt16                   Use;           // Image usage flags (see ImageUse).
    UByte                    TextureFlags;  // Texture usage flags (see TextureFlagBits in derived Texture classes).    
    MappedTextureBase*       pMap;          // If texture is currently mapped, it is here.
    const TextureFormat*     pFormat;       // The TextureFormat used by the texture (platform specific derived classes of TextureFormat exist).

};


// ***** DepthStencilSurface

// DepthStencilData represents a depth/specil buffer surface allocatable
// through the TextureManager. This object is usually wrapped in DepthStencilBuffer,
// but is independent from it to allow for system-specific implementation.

class DepthStencilSurface : public RefCountBase<DepthStencilSurface, StatRender_TextureManager_Mem>, public ListNode<DepthStencilSurface>
{
public:    
    DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size) : 
      pManagerLocks(pmanagerLocks), State(Texture::State_InitPending), Size(size) { }

    virtual TextureManager* GetTextureManager() const { return pManagerLocks ? pManagerLocks->pManager : 0; };
    virtual ImageSize       GetSize() const { return Size; }
    virtual bool            Initialize() = 0;

    // Texture synchronization object.
    Ptr<TextureManagerLocks> pManagerLocks;
    Texture::CreateState     State;
    ImageSize                Size;
};


// ***** ImageUpdateSync

// ImageUpdateSync is an abstract interface used to notify rendering Context and/or
// TextureManager about image updates. Rendering context updates are executed immediately
// after the Context::NextCapture is called, allowing image data updates to be
// synchronized with context frames. TextureManager updates are called immediately
// after next renderer frame starts.

class ImageUpdate;
class ImageUpdateSync
{
public:
    virtual ~ImageUpdateSync() {}

    // Queues up a default full-image update implemented by mapping the texture
    // and calling Image::Decode to fully populate it with data.
    virtual void    UpdateImage(Image* pimage)        = 0;

    // Queues up a custom image update, which is AddRefred internally.
    // ImageUpdate::HandleUpdate function will be executed when the update takes place.
    virtual void    UpdateImage(ImageUpdate* pupdate) = 0;
};

// ***** ImageUpdate

// ImageUpdate represent an image update action queued up for the image,
// typically to be executed on a rendering thread. When update takes
// place it is welcome to Map the texture and copy/generate data there.

// There are several image/texture update types:
//  1. Full copy/upload of image into texture by calling Image::Decode.
//  2. Rectangle-based update from source data image (source image was
//     updated by Advance thread).
//  3. Dynamically generated update of a rectangle, typically taking place on
//     a rendering thread.
//  4. Full image dynamically generated update.

class ImageUpdate : public RefCountBase<ImageUpdate, Stat_Default_Mem>
{
protected:
    Ptr<Image> pImage;

public:
    ImageUpdate(Image* pimage);

    Image*       GetImage() { return pImage; }

    // Override this function to update the image and/or its associated texture.
    virtual void HandleUpdate(TextureManager* pmanager) { SF_UNUSED(pmanager); }

    /*
    virtual HandleUpdate()
    {
        // Map all planes and decode image content into them.
        ImageData imagePlanes;
        if (pImage->pTexture->Map(&imagePlanes))
        {
            pImage->Decode(&imagePlanes);
            pImage->pTexture->Unmap();
        }

    // TBD: Make sure above works for updates.
    }
    */
};

// *** ImageUpdateQueue is a simple array-based queue used to record image
// update requests and execute them all at once. Uses the lower pointer
// bit to determine whether this is a full or partial image update. 

struct ImageUpdateQueue
{
    typedef ArrayConstPolicy<4, 4, false> UpdateShrinkPolicy;
    typedef UPInt                         UpdateRef;

    ArrayLH<UpdateRef, StatRender_TextureManager_Mem, UpdateShrinkPolicy>  Queue;

    void Add(Image* pimage);
    void Add(ImageUpdate* pupdate);

    ~ImageUpdateQueue();

    void   ProcessUpdates(Render::TextureManager* pmanager);
};


class MemoryManager ;

class TextureManager : public RefCountBase<TextureManager, StatRender_TextureManager_Mem>,
                       public ImageUpdateSync
{
    friend class Texture;

public:

    TextureManager(ThreadId renderThreadId = 0, ThreadCommandQueue* commandQueue = 0, TextureCache* textureCache = 0);
    virtual ~TextureManager();

    // Creates a texture.
    // Texture is destroyed by its Release() call.
    // An alternative MemoryManager can be provided in case of a RenderTarget,
    // which may used its own memory pool/swapping strategy in Render::TargetManager.
    virtual Texture*        CreateTexture(ImageFormat format, unsigned mipLevels,
                                          const ImageSize& size,
                                          unsigned use, ImageBase* pimage = 0,
                                          MemoryManager* mmanager = 0) = 0; 

    virtual unsigned        GetTextureFormatSupport() const { return ImageFormats_Standard; }

    // Reports ImageUse flags supported for this format.
    virtual unsigned        GetTextureUseCaps(ImageFormat format) = 0;
    virtual bool            IsNonPow2Supported(ImageFormat, UInt16) { return true; }

    // Created DepthStencil surface, called from RenderBufferManafer.
    // Alternative memory manager may be used to swap out resources.
    virtual DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                           MemoryManager* manager = 0) = 0;

    // Created DepthStencil buffer. Alternative memory manager may be used to swap out resources.
    virtual DepthStencilBuffer* CreateDSBuffer(const ImageSize& size,
                                               MemoryManager* manager = 0)
    { SF_UNUSED2(size, manager); return 0; }


    // Reports whether textures can be created on the current thread. Normally this is only false 
    // if using a multi-threaded renderer, and the platform requires that textures be created on the 
    // render thread (default implementation). If a platform does not have these restrictions, it could
    // simply return true from this method.
    virtual bool             CanCreateTextureCurrentThread() const { return RenderThreadId == 0 || GetCurrentThreadId() == RenderThreadId; }

    // Processes work queued up on other threads, including texture creation 
    // and image updates. This function should be called before each frame 
    // on the RenderThread (it is called by TextureManager::BeginFrame).
    virtual void             ProcessQueues();

    // Should be called before each frame on RenderThread.
    virtual void             BeginFrame();
    virtual void             EndFrame();

    // Should be called before/after each scene on RenderThread.
    virtual void             BeginScene()    { }
    virtual void             EndScene()      { }

    // *** ImageUpdateSync
    virtual void             UpdateImage(Image* pimage);
    virtual void             UpdateImage(ImageUpdate* pupdate);

    // Reports the desired format for DrawableImages. These textures must be mappable, and
    // thus this should report a format that is capable of being mapped for the given platform.
    // Valid return values are either Image_R8G8B8A8, or Image_B8G8R8A8.
    virtual ImageFormat      GetDrawableImageFormat() const { return Image_R8G8B8A8; }

    // Determines whether the input format is an acceptable DrawableImage format for this texture manager.
    virtual bool             IsDrawableImageFormat(ImageFormat format) const { return (format == Image_R8G8B8A8); }

    // Returns the ImageSwizzler object compatible with this texture manager. This is used when the CPU accesses
    // GPU textures, which may be tiled and/or swizzled in a platform specific manner.
    virtual ImageSwizzler&   GetImageSwizzler() const;

    // Returns the TextureCache for this texture manager (may be NULL).
    TextureCache* GetTextureCache() const { return pTextureCache; }

protected:

    // This command is pushed to the RenderThread queue when TextureInitQueue needs
    // to be services, allowing texture creation to work as fast as possible. This is
    // required on platforms which require texture creation/destruction to happen only
    // on the render thread.
    class ServiceCommand : public ThreadCommand
    {
        TextureManager* pManager;
    public:
        ServiceCommand(TextureManager* manager) : pManager(manager) { }
        virtual void Execute() { pManager->ProcessQueues(); }
    };
    TextureManager* getThis() { return this; }

    void            processImageUpdates()       { ImageUpdates.ProcessUpdates(this); }
    virtual void    processTextureKillList()    { }
    virtual void    processInitTextures()       { }

    virtual const TextureFormat* getTextureFormat(ImageFormat format) const;
    const TextureFormat*         precreateTexture(ImageFormat format, unsigned use, ImageBase* pimage);
    Render::Texture*             postCreateTexture(Texture* ptexture, unsigned use );
    Render::DepthStencilSurface* postCreateDepthStencilSurface( Render::DepthStencilSurface* pdss );

    // Texture Memory-mapping support.

    // Returns true if the platform format is 'scanline compatible' with the texture format.
    // This means that either the layout is identical, or a simple swizzling of components.
    // By default, assume that the platform uses only scanline compatible images.
    virtual bool                isScanlineCompatible(const TextureFormat*) { return true; }   

    virtual MappedTextureBase*  mapTexture(Texture* p, unsigned mipLevel, unsigned levelCount);
    virtual MappedTextureBase*  mapTexture(Texture* p) { return mapTexture(p, 0, p->MipLevels); }
    virtual void                unmapTexture(Texture *ptexture, bool applyUpdate = true);
    virtual MappedTextureBase&  getDefaultMappedTexture() = 0;
    virtual MappedTextureBase*  createMappedTexture() = 0;

    typedef List<Render::Texture>  TextureList;
    typedef List<Render::DepthStencilSurface> DepthStencilList;

    ServiceCommand            ServiceCommandInstance;   // The command which will service the texture queues (used when the Advance thread blocks on texture creation).
    ThreadId                  RenderThreadId;           // The ThreadId of the render thread.
    ThreadCommandQueue*       pRTCommandQueue;          // Queue for adding rendering commands that can not be executed immediately.
    Ptr<TextureCache>         pTextureCache;            // TextureCache object, which may be NULL. See Render::TextureCache for a description.

    Ptr<TextureManagerLocks>  pLocks;                   // Texture synchronization object.
    ImageUpdateQueue          ImageUpdates;             // Queue of updates pending on image.

    // Texture format table, organized by supported HW features.
    ArrayLH<TextureFormat*>   TextureFormats;

    // Lists protected by TextureManagerLocks::TextureMutex.
    TextureList               Textures;                 // 'Global' list of textures.
    TextureList               TextureInitQueue;         // List of textures that are not initialized currently (not created on the render thread).
    DepthStencilList          DepthStencilInitQueue;    // List of depth stencil surfaces that are not initialized currently (not created on the render thread).
};

// ***** Image

// ImageBase is a base class for image that combines Format, Size information
// with Decode() function. This class is separated from Image to enable initialization
// of textures in TextureManager::Create without requiring a full Image object.
// This class also supports implementation of ImageSource.

// IMPORTANT: if adding new virtual methods to Image or ImageBase, please do the same
// for ImageDelegate!!!
class ImageBase : public RefCountBaseV<Image, Stat_Image_Mem>
{
public:
    enum ImageType
    {
        Type_ImageBase,
        Type_Image,
        Type_RawImage,
        Type_GradientImage,
        Type_TextureImage,
        Type_SubImage,
        Type_DrawableImage,
        Type_Other
    };

    // RTTI For image. See Image::GetImageType. Note, ImageDelegate::GetImageType
    // returns the type of enclosed image! To check if it is delegate or not use IsDelegate.
    // If performing type casting based on GetImageType, use GetAsImage() to get the original
    // image and cast it.
    virtual ImageType       GetImageType() const  { return Type_ImageBase; }
    virtual ImageFormat     GetFormat() const   = 0;
    virtual ImageSize       GetSize() const     = 0;
    virtual ImageRect       GetRect() const { return ImageRect(0, 0, GetSize()); }
    virtual unsigned        GetMipmapCount() const = 0;

    // Returns Format without 'Convertible' flag.
    inline  ImageFormat     GetFormatNoConv() const
    { return (ImageFormat)(GetFormat() & ~ImageFormat_Convertible); }

    // Scan-line copy function type used by Decode. Default implementation relies 
    // on memcpy, but it may also do format conversion (in which case destination
    // size may also be different, but Decode doesn't need to know this).
    typedef void (SF_STDCALL *  CopyScanlineFunc)(UByte* pd, const UByte* ps, UPInt size,
                                                  Palette* psrcColorMap, void* arg);

    static void SF_STDCALL  CopyScanlineDefault(UByte* pd, const UByte* ps,
                                                UPInt size, Palette*, void*);

    // Decodes image data; see Image::Decode.
    virtual bool    Decode(ImageData* pdest, CopyScanlineFunc copyScanline = CopyScanlineDefault,
                           void* arg = 0) const
    { SF_UNUSED3(pdest, copyScanline, arg); return false; }

    virtual Image* GetAsImage() = 0;

    virtual bool IsDelegate() const { return false; }

    SF_AMP_CODE(
        virtual UPInt   GetBytes(int* memRegion) const;
        virtual UInt32  GetImageId() const  { return 0; }
        virtual UInt32  GetBaseImageId() const  { return 0; }
        static UInt32 SF_STDCALL GetNextImageId();
    )
};



/*
    Image lifetime with respect to threads:
      - Image ref-counting is thread-safe. Renderer thread has the right to
        hold additional references to image/texture only for as long as RenderContext
        is alive. Calling RenderContext destructor should also remove all image
        references from its nodes (even if they exist in render queues).
      - This is possible because RenderContext doesn't wait for next capture (but
        it needs to wait for the frame to finish).

*/
// IMPORTANT: if adding new virtual methods to Image or ImageBase, please do the same
// for ImageDelegate!!!
class Image : public ImageBase
{   
protected:
    AtomicPtr<Texture> pTexture;    
    ImageUpdateSync*   pUpdateSync;
    // Image transform matrix; stored in inverted form for efficiency.
    Matrix2F*          pInverseMatrix;

    // Releases texture atomically with proper notification.
    void    releaseTexture();
    // Used to initialize pTexture pointer on creation.
    void    initTexture_NoAddRef(Texture* ptexture);
    
public:

    Image(Texture* ptexture = 0, ImageUpdateSync* psync = 0)
        : pTexture(ptexture), pUpdateSync(psync), pInverseMatrix(0)
    {
        if (ptexture)
            ptexture->AddRef();
    }

    virtual ~Image();

    // Creates image with desired Use capabilities, also considering capabilities of
    // the texture manager.
    static Image* SF_STDCALL    Create(ImageFormat format, unsigned mipLevels, const ImageSize& size,
                                       unsigned use, MemoryHeap* pheap = 0,
                                       TextureManager* pmanager = 0, ImageUpdateSync* pupdateSync = 0);

    // Called to create an Image that references a subregion of this image, as happens
    // when GFx::ImagePackParams are used (or when loading images packed by gfxexport).
    // Default implementation creates SubImage that references this Image for its data.
    virtual Image*          CreateSubImage(const ImageRect& rect,
                                           MemoryHeap* pheap = Memory::GetGlobalHeap());

    // RTTI For image. Note, ImageDelegate::GetImageType
    // returns the type of enclosed image! To check if it is delegate or not use IsDelegate.
    // If performing type casting based on GetImageType, use GetAsImage() to get the original
    // image and cast it.
    virtual ImageType       GetImageType() const   { return Type_Image; }    
    virtual ImageFormat     GetFormat() const  = 0;
    virtual unsigned        GetUse() const = 0;

    // Obtains logical dimensions of images. These are usually the dimensions of the 
    // image that should be displayed even if the image itself is rounded up.
    virtual ImageSize       GetSize() const     = 0;

    // Return sub-rectangle of texture to use; by default full texture rectangle is reported.
    // A smaller packed rectangle can be reported for GSubImageInfo.
    virtual ImageRect       GetRect() const { return ImageRect(0, 0, GetSize()); }
    
    // SetMatrix/GetMatrix methods access an image matrix that may be associated with and
    // image to adjust its size and/or location. Matrix of scale 2.0, for example, will make
    // the image 2x bigger; while translation of 4.0 will shift it 4 pixels to the right.
    // Among other things, image matrix is used to adjust for scaling done by 'gfxexport'.
    // This matrix is Identity dy default and doesn't occupy additional space unless assigned.
    // Matrix as allocated in AUTO heap by default; alternatively, heap can be explicit.
    // The matrix is actually stored as an inverse, so additional inverse accessors are
    // provided for efficiency (an inverse must be applied to UV image matrix).
    virtual void            SetMatrix(const Matrix2F& mat, MemoryHeap* heap = 0);
    virtual void            GetMatrix(Matrix2F* mat) const;
    virtual void            SetMatrixInverse(const Matrix2F& mat, MemoryHeap* heap = 0);
    virtual bool            GetMatrixInverse(Matrix2F* mat) const;

    // Map and Unmap are only available if image has ImageUse_MapSimThread flag.
    // If Map is used user is still responsible for calling Update to notify render thread
    // of changes that took place. All of the data planes of the specified level are mapped
    // simultaneously.
    virtual bool            Map(ImageData* pdata, unsigned levelIndex = 0, unsigned levelCount = 0)
    {
        SF_UNUSED3(pdata, levelIndex, levelCount);
        return false;
    }
    virtual bool            Unmap() { return false; }


    // Decodes image into the destination ImageData, potentially converting data 
    // a scan-line at a time. The following cases can be handled:
    //  For Generic formats:
    //    - Decode should happen a scan-line at a time for each plane by calling
    //      copyScanline if provided.
    //    - Destination format must match Source format with convertible bits
    //      masked on both.
    //    - Destination Size >= Source Size.
    //    - Destination must have at least one mipLevel. In general, planeCounts
    //      should match, but total mipLevels count MAY BE SMALLER in dest, in
    //      case back-end driver decided to drop some bottom-level mipLevels.
    //  For HW formats:
    //   - Decode will copy data as one block of DataSize per plane.
    //   - Destination format must match Source format without convertible bit.
    //   - Size and planeCount must match for both source and destination.
    //  Decode can be called from the rendering thread until releaseTexture() takes place.
    //  This means that Image object should remain thread-safe (have immutable structure
    //  from the CreateTexture/GetTexture call until texture release).

    virtual bool    Decode(ImageData* pdest, CopyScanlineFunc copyScanline = CopyScanlineDefault,
                           void* arg = 0) const
    { SF_UNUSED3(pdest, copyScanline, arg); return false; }

    // Queues up full update for the image; Decode will be called on the rendering
    // thread when update takes place.
    virtual bool    Update()
    {
        if (pUpdateSync)
        {
            pUpdateSync->UpdateImage(this);
            return true;
        }
        return false;
    }
    // Queues up a custom image update, to be executed on the rendering thread.
    virtual bool   Update(ImageUpdate* pupdate)
    {
        if (pUpdateSync)
        {
            pUpdateSync->UpdateImage(pupdate);
            return true;
        }
        return false;
    }


    // *** Render Thread-Only functions

    // These functions will only be called from the rendering thread and can thus
    // access pTexture pointer directly without atomics, since it is known that
    // pTexture can not change during such a call. This is the case since Context
    // entry will AddRef to image, not allowing it to be released until render frame
    // is done.

    // Obtains the texture pointer from the data, for a given renderer.
    virtual Texture*        GetTexture(TextureManager* pmanager)
    { SF_UNUSED(pmanager); return pTexture; }


    // Fills in a matrix that converts image pixel coordinate space {0,0, Width, Height}
    // into the UV coordinate space expected by Render::HAL. This considers all of
    // the following:
    //  - Texture::GetUVGenMatrix, for HAL padding/scale adjustment.
    //  - GetRect() SubImage, including an offset as needed for packing.
    //  - Local transformation Matrix (used by gfxexport).
    virtual void    GetUVGenMatrix(Matrix2F* mat, TextureManager* manager);

    // Fills in a matrix that converts normalized UV (0, 0, 1, 1) coordinate space 
    // to the HAL-specific UV such as packed sub-images, resample, etc.
    virtual void    GetUVNormMatrix(Matrix2F* mat, TextureManager* manager);


    // Notifies Image that texture contents were lost; image can respond by
    // releasing the texture reference if need.
    enum TextureLossReason
    {
        TLR_ManagerDestroyed,
        TLR_DeviceLost,
        TLR_SwappedOut
    };

    virtual void            TextureLost(TextureLossReason reason)
    {
        SF_UNUSED(reason);
        if (reason == TLR_ManagerDestroyed)
            releaseTexture();
    }

    virtual MemoryBufferImage* GetAsMemoryImage() { return NULL; }

    SF_AMP_CODE(
        virtual UPInt   GetBytes(int* memRegion) const;
    )
};

// A wrapper around an Image to enable image substitution.
class ImageDelegate : public Image
{
public:
    Ptr<Image>  pImage;

    ImageDelegate() {}
    ImageDelegate(Image* img) : pImage(img) {}

    // RTTI For image. See Image::GetImageType. Note, ImageDelegate::GetImageType
    // returns the type of enclosed image! To check if it is delegate or not use IsDelegate.
    // If performing type casting based on GetImageType, use GetAsImage() to get the original
    // image and cast it.
    virtual ImageType       GetImageType() const  { return pImage->GetImageType(); }
    virtual ImageFormat     GetFormat() const     { return pImage->GetFormat(); }
    virtual ImageSize       GetSize() const       { return pImage->GetSize(); }
    virtual ImageRect       GetRect() const       { return pImage->GetRect(); }
    virtual unsigned        GetMipmapCount() const{ return pImage->GetMipmapCount(); }

    virtual void    SetMatrix(const Matrix2F& mat, MemoryHeap* heap = 0) 
    { pImage->SetMatrix(mat, heap); }
    virtual void        GetMatrix(Matrix2F* mat) const { pImage->GetMatrix(mat); }
    virtual void    SetMatrixInverse(const Matrix2F& mat, MemoryHeap* heap = 0) 
    { pImage->SetMatrixInverse(mat, heap); }
    virtual bool    GetMatrixInverse(Matrix2F* mat) const { return pImage->GetMatrixInverse(mat); }
    virtual void    GetUVGenMatrix(Matrix2F* mat, TextureManager* manager) 
    { pImage->GetUVGenMatrix(mat, manager); }
    virtual void    GetUVNormMatrix(Matrix2F* mat, TextureManager* manager)
    { pImage->GetUVNormMatrix(mat, manager); }

    // Returns Format without 'Convertible' flag.
    inline  ImageFormat     GetFormatNoConv() const
    { return pImage->GetFormatNoConv(); }

    // Decodes image data; see Image::Decode.
    virtual bool  Decode(ImageData* pdest, 
                         CopyScanlineFunc copyScanline = CopyScanlineDefault,
                         void* arg = 0) const
    { return pImage->Decode(pdest, copyScanline, arg); }

    // Image virtual methods
    virtual unsigned GetUse() const { return pImage->GetUse(); }
    virtual Image*   CreateSubImage(const ImageRect& rect,
        MemoryHeap* pheap = Memory::GetGlobalHeap()) { return pImage->CreateSubImage(rect, pheap); }
    virtual bool     Map(ImageData* pdata, 
        unsigned levelIndex = 0, unsigned levelCount = 0)
    { return pImage->Map(pdata, levelIndex, levelCount); }
    virtual bool     Unmap() { return pImage->Unmap(); }
    virtual Texture* GetTexture(TextureManager* pmanager)  
    { return pImage->GetTexture(pmanager); }
    virtual void     TextureLost(TextureLossReason reason) 
    { pImage->TextureLost(reason); }

    virtual bool     Update() { return pImage->Update(); }
    virtual bool     Update(ImageUpdate* pupdate) { return pImage->Update(pupdate); }

    virtual Image*   GetAsImage() { return pImage; }
    virtual MemoryBufferImage* GetAsMemoryImage() { return pImage->GetAsMemoryImage(); }

    virtual bool     IsDelegate() const { return true; }

    SF_AMP_CODE(
        virtual UPInt   GetBytes(int* memRegion) const    { return pImage->GetBytes(memRegion); }
        virtual UInt32  GetImageId() const  { return pImage->GetImageId(); }
        virtual UInt32  GetBaseImageId() const  { return pImage->GetBaseImageId(); }
    )
};

// ***** ImageSource

struct ImageCreateArgs
{
    unsigned          Use;
    MemoryHeap*       pHeap;    
    TextureManager*   pManager;
    ImageUpdateSync*  pUpdateSync;
    ImageFormat       Format;

    ImageCreateArgs(ImageFormat format = Image_None,
                    unsigned use = 0, MemoryHeap* heap = 0,
                    TextureManager* manager = 0, ImageUpdateSync* updateSync = 0)
        : Use(use), pHeap(heap),
          pManager(manager), pUpdateSync(updateSync), Format(format)
    { }
    ImageCreateArgs(TextureManager* manager)
        : Use(0), pHeap(0),
          pManager(manager), pUpdateSync(0), Format(Image_None)
    { }

    MemoryHeap*      GetHeap() const { return pHeap ? pHeap : Memory::GetGlobalHeap(); }
    ImageUpdateSync* GetUpdateSync() const { return pUpdateSync ? pUpdateSync : pManager; }
};


class ImageSource : public ImageBase
{
public:

    // Determines if Decode-only image is compatible with the argument set.
    // Intended for use by CreateCompatibleImage implementation.
    //  - If true, MemoryBufferFileImage may be created.
    //  - If false, base class CreateCompatibleImage is called,
    //    to create TextureImage/RawImage instead.
    virtual bool IsDecodeOnlyImageCompatible(const ImageCreateArgs& args);

    // Creates an image out of source that is compatible with argument Use,
    // updateSync and other settings.
    virtual Image* CreateCompatibleImage(const ImageCreateArgs& args);
};


class WrapperImageSource : public Render::ImageSource
{
protected:
    Ptr<Image> pDelegate;
public:
    WrapperImageSource(Image* pdelegate) : pDelegate(pdelegate) 
    {}

    virtual ImageType       GetImageType() const  { return pDelegate->GetImageType(); }
    virtual ImageFormat     GetFormat() const     { return pDelegate->GetFormat();    }
    virtual ImageSize       GetSize() const       { return pDelegate->GetSize();      }
    virtual unsigned        GetMipmapCount() const { return pDelegate->GetMipmapCount(); }

    // Decodes image data; see Image::Decode.
    virtual bool    Decode(ImageData* pdest, CopyScanlineFunc copyScanline = CopyScanlineDefault,
        void* arg = 0) const
    { return pDelegate->Decode(pdest, copyScanline, arg); }

    virtual Image* CreateCompatibleImage(const ImageCreateArgs& args)
    {
        if (IsDecodeOnlyImageCompatible(args))
        {
            pDelegate->AddRef();
            return pDelegate;
        }
        return Render::ImageSource::CreateCompatibleImage(args);
    }

    virtual Render::Image* GetAsImage() { return pDelegate; }
    SF_AMP_CODE(
        virtual UPInt   GetBytes(int* memRegion) const { return pDelegate->GetBytes(memRegion); }
        virtual UInt32  GetImageId() const { return pDelegate->GetImageId(); }
        virtual UInt32  GetBaseImageId() const { return pDelegate->GetBaseImageId(); }
    )
};

// ***** Raw Image

// RawImage actually allocates image content in system memory,
// allowing it to be accessed directly through GetImageData.
//  

class RawImage : public Image
{
    ImageData Data;    
    SF_AMP_CODE(UInt32 ImageId;)

protected:
    RawImage()
    {
        SF_AMP_CODE(ImageId = ImageBase::GetNextImageId();)
    }
public:
    
    ~RawImage();

    void freeData();
    bool hasData() const;

    // Override original Create (?)
    // Or no TextureManager? Base Create is hidden
    static RawImage* SF_STDCALL    Create(ImageFormat format, unsigned mipLevels,
                                          const ImageSize& size, unsigned use,
                                          MemoryHeap* pheap = 0,
                                          ImageUpdateSync* pupdateSync = 0);

    // Fills in the ImageData data structure, that can be used to access image bits.
    // For convertible images, pData may be null.
    bool                    GetImageData(ImageData* pdata);
    void                    SetImageData(ImageData* pdata);

    // *** Image Implementation
    virtual ImageType       GetImageType() const    { return Type_RawImage; }
    virtual ImageFormat     GetFormat() const       { return Data.Format; }  
    virtual unsigned        GetUse() const          { return Data.Use; }
    virtual ImageSize       GetSize() const         { return Data.GetSize(); }
    virtual unsigned        GetMipmapCount() const  { return Data.GetMipLevelCount();}
    virtual bool            Map(ImageData* pdata, unsigned planeIndex = 0, unsigned planeCount = 0);
    virtual bool            Unmap();
    virtual bool            Decode(ImageData* pdest,
                                   CopyScanlineFunc copyScanline = CopyScanlineDefault,
                                   void* arg = 0) const;
    virtual Texture*        GetTexture(TextureManager* pmanager);

    virtual Image* GetAsImage() { return this; }
    SF_AMP_CODE(
        virtual UInt32  GetImageId() const { return ImageId; }
        virtual UInt32  GetBaseImageId() const { return 0; }
    )
};

// TextureImage is a version of image that delegates all of its implementation to
// a Texture, not storing any additional plane information in memory.
// TextureImage typically can't be mapped locally, but it may be updatable
// based on the requested capabilities.

class TextureImage : public Image
{     
    // Cache Format& size to avoid thread-unsafe access to pTexture,
    // which could be released at any time if TextureManager/HAL is destroyed.
    ImageFormat Format;
    ImageSize   Size;
    unsigned    Use;
    SF_AMP_CODE(UInt32 ImageId;)

public:
    TextureImage(ImageFormat format, const ImageSize& size, unsigned use,
                 Texture* ptexture = 0, ImageUpdateSync* psync = 0)
        : Image(ptexture, psync),
          Format(format), Size(size), Use(use)
    {
        SF_AMP_CODE(ImageId = ImageBase::GetNextImageId();)
    }

    void                    RefreshCachedFormat()
    {
        if (pTexture)
        {
            Format = pTexture->GetFormat();
            Size = pTexture->GetSize();
        }
    }

    virtual ImageType       GetImageType() const    { return Type_TextureImage; }
    virtual ImageFormat     GetFormat() const       { return Format; }
    virtual unsigned        GetUse() const          { return Use; }
    virtual ImageSize       GetSize() const         { return Size; }
    virtual unsigned        GetMipmapCount() const  { return pTexture ?  pTexture->GetMipmapCount() : 1; }
    // Map/Unmap may only be allowed in single threaded-mode.
    virtual bool            Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount);
    virtual bool            Unmap();

    // TextureImage can't be restored by default.
    virtual void            TextureLost(TextureLossReason)
    { releaseTexture(); }

    virtual Image* GetAsImage() { return this; }
    SF_AMP_CODE(
        virtual bool    Decode(ImageData* pdest, CopyScanlineFunc copyScanline = CopyScanlineDefault, void* arg = 0) const;
        virtual UPInt   GetBytes(int* memRegion) const { if (memRegion) *memRegion = 0; return pTexture ?  pTexture->GetBytes(memRegion) : Image::GetBytes(memRegion); }
        virtual UInt32  GetImageId() const { return ImageId; }
        virtual UInt32  GetBaseImageId() const { return 0; }
    )
};



// SubImage is an implementation of Image that references a sub-rectangle
// of an original image. Objects of this class are created by Image::CreateSubImage
// to support image packing.

class SubImage : public Image
{
    Ptr<Image>  pImage; 
    ImageRect   SubRect;
    SF_AMP_CODE(UInt32 ImageId;)

public:
    SubImage(Image* pimage, const ImageRect& rect)
        : pImage(pimage), SubRect(rect)
    {
        SF_AMP_CODE(ImageId = ImageBase::GetNextImageId();)
    }

    virtual ImageType       GetImageType() const            { return Type_SubImage; }
    virtual ImageFormat     GetFormat() const               { return pImage->GetFormat(); }
    virtual unsigned        GetUse() const                  { return pImage->GetUse(); }
    virtual ImageSize       GetSize() const                 { return pImage->GetSize(); }
    virtual unsigned        GetMipmapCount() const          { return pImage->GetMipmapCount(); }
    virtual ImageRect       GetRect() const                 { return SubRect; }

    virtual bool            GetMatrixInverse(Matrix2F* mat) const;

    // Map and Unmap are not allowed for SubImage.
    virtual bool            Map(ImageData*, unsigned, unsigned)     { return false; }
    virtual bool            Unmap()                         { return false; }
    virtual bool            Decode(ImageData* pdest, CopyScanlineFunc csf = CopyScanlineDefault,
                                   void* arg = 0) const
    { return pImage->Decode(pdest, csf, arg); }
    virtual Texture*        GetTexture(TextureManager* pm)  { return pImage->GetTexture(pm); }
    virtual void            TextureLost(TextureLossReason reason) { pImage->TextureLost(reason); }
    Image*                  GetBaseImage() { return pImage; }

    virtual Image* GetAsImage() { return this; }
    SF_AMP_CODE(
    virtual UPInt   GetBytes(int* memRegion) const { if (memRegion) *memRegion = 0; return 0; }
    virtual UInt32  GetImageId() const { return ImageId; }
    virtual UInt32  GetBaseImageId() const { return pImage ? pImage->GetImageId() : 0; }
    )
};


// ***** Wrapping / Sampling modes used during image rendering

enum WrapMode
{
    Wrap_Repeat  = 0x00,
    Wrap_Clamp   = 0x01,
    Wrap_Mask    = 0x01,
    Wrap_Count   = 2
};
enum SampleMode
{
    Sample_Point = 0x00,
    Sample_Linear= 0x02,
    Sample_Mask  = 0x02,
    Sample_Count = 2,
    Sample_Shift = 1
};
enum MipmapMode
{
    Mipmap_None  = 0x00,
    Mipmap_2D    = 0x04, // -0.5 lod bias
    Mipmap_Count = 2,
    Mipmap_Shift = 2
};
enum SamplerStateCountType
{
    SamplerStateObjectCount = 2<<Mipmap_Shift
};

struct ImageFillMode
{
    UByte Fill;
    ImageFillMode() : Fill(Wrap_Repeat|Sample_Point) { }
    ImageFillMode(WrapMode wrap, SampleMode sample) : Fill ((UByte)(wrap|sample)) { }
    ImageFillMode(const ImageFillMode& s) : Fill(s.Fill) { }
    void operator = (const ImageFillMode& s) { Fill = s.Fill; }
    bool operator == (const ImageFillMode& s) const { return Fill == s.Fill; }
    bool operator != (const ImageFillMode& s) const { return Fill != s.Fill; }

    void       Set(WrapMode wrap, SampleMode sample) { Fill = (UByte)(wrap|sample); }
    WrapMode   GetWrapMode() const          { return (WrapMode) (Fill & Wrap_Mask); }
    void       SetWrapMode(WrapMode wm)     { Fill = (UByte) ((Fill &~Wrap_Mask) | wm); }
    SampleMode GetSampleMode() const        { return (SampleMode) (Fill & Sample_Mask); }
    void       SetSampleMode(SampleMode sm) { Fill = (UByte)((Fill &~Sample_Mask) | sm); }
};

// *** TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat from which 
// texture is supposed to be initialized. Replacement structs for each backend exist.
struct TextureFormat : public NewOverrideBase<StatRender_TextureManager_Mem>
{
    virtual ~TextureFormat() { }

    virtual ImageFormat             GetImageFormat() const = 0;
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const = 0;
    virtual Image::CopyScanlineFunc GetScanlineUncopyFn() const { return 0; };

    unsigned                GetPlaneCount() const
    { return ImageData::GetFormatPlaneCount(GetImageFormat()); }
};

inline unsigned Texture::GetPlaneCount() const
{
    return ImageData::GetFormatPlaneCount(GetFormat()) * GetMipmapCount();
}

inline ImageFormat Texture::GetImageFormat() const
{ 
    return pFormat ? pFormat->GetImageFormat() : Image_None; 
}



}}; // namespace Scaleform::Render

#endif // INC_SF_Render_Image_H
