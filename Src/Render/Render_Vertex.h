/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Vertex.h
Content     :   Vertex Element declarations 
Created     :   August 17, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Vertex_H
#define INC_SF_Render_Vertex_H

// This file provides Vertex Element declarations representing different vertex types,
// both on the renderer input and internal sides. 

#include "Kernel/SF_RefCount.h"
#include "Render_Matrix2x4.h"
#include "Render_Color.h"

namespace Scaleform { namespace Render {

// TBD: We might want to have a flag that indicates whether precision
// can be dropped to lower resolution or not (?). For example, it can be
// used to determine whether 16-bit integer indices are ok or not. This
// result depend on matrix scale (similar to mesh quality).

enum VertexElementDataType
{
    VEDT_None,
    VEDT_U8N,
    VEDT_U8,            
    VEDT_S16,            
    VEDT_U16,            
    VEDT_U32,            
    VEDT_F32,             
    VEDT_I8,
    VEDT_I16,
};

enum VertexElementUsageType
{
    VEUT_None,
    VETT_Pos,
    VETT_Color,
    VETT_TexCoord,
    VETT_Instance,
};

enum VertexElementIndexType
{
    VEIT_None,
    VEIT_Index1,
    VEIT_Index2,
};

struct AttributeType
{
    unsigned                Components : 4;     // Number of components in the element.
    VertexElementDataType   DataType   : 4;     // Element data type
    VertexElementUsageType  UsageType  : 4;     // Element usage type
    VertexElementIndexType  IndexType  : 4;     // Element index type
    unsigned                Argument   : 1;     // Element argument
};

enum VertexElementType
{
    // If this flag is set, all the vertex elements of this type will
    // be assigned the same value from the argument type.
    VET_Argument_Flag   = 0x10000,

    VET_Type_Mask       = 0xffff,
    VET_Components_Mask = 0xf,
    VET_CompType_Mask   = 0xf0,
    VET_CompType_Shift  = 4,
    VET_Usage_Mask      = 0xf00,
    VET_Usage_Shift     = 8,
    VET_Index_Mask      = 0xf000,
    VET_Index_Shift     = 12,

    VET_U8N             = 0x10,
    VET_U8              = 0x20,
    VET_S16             = 0x30,
    VET_U16             = 0x40,
    VET_U32             = 0x50,
    VET_F32             = 0x60,
    VET_I8              = 0x70,
    VET_I16             = 0x80,

    VET_None            = 0,
    VET_Pos             = 0x100,
    VET_Color           = 0x200,
    VET_TexCoord        = 0x300,
    VET_Instance        = 0x400,

    VET_Index1          = 0x1000,
    VET_Index2          = 0x2000,

    VET_XY16i           = VET_Pos      | VET_S16 | 2,
    VET_XYZ16i          = VET_Pos      | VET_S16 | 3,
    VET_XY32f           = VET_Pos      | VET_F32 | 2,
    VET_XYZ32f          = VET_Pos      | VET_F32 | 3,
    VET_UV32f           = VET_TexCoord | VET_F32 | 2,
    VET_ColorRGBA8      = VET_Color    | VET_U8N | 4,
    VET_ColorARGB8      = VET_Color    | VET_U32 | 1,
    VET_FactorAlpha8    = VET_Color    | VET_U8N | 1 | (1 << VET_Index_Shift),
    VET_T0Weight8       = VET_Color    | VET_U8N | 1 | (2 << VET_Index_Shift),
    VET_IndexedFactors  = VET_Color    | VET_I8  | 2 | (1 << VET_Index_Shift),

    VET_Instance8       = VET_Instance | VET_U8  | 1 | VET_Argument_Flag
};

static unsigned VertexTypeSizes[] = {1,1,2,2,4,4};

struct VertexElement
{
    unsigned Offset;
    union
    {
        unsigned Attribute;
        AttributeType AttributeNice;
    };

    inline unsigned CompSize() const
    {
        return VertexTypeSizes[((Attribute & VET_CompType_Mask) >> VET_CompType_Shift) - 1];
    }

    inline unsigned Size() const
    {
        return (Attribute & VET_Components_Mask) * CompSize();
    }

    inline bool operator == (const VertexElement& o) const
    {
        return (Offset == o.Offset) && (Attribute == o.Attribute);
    }
    inline bool operator != (const VertexElement& o) const
    {
        return !operator == (o);
    }
};

class SystemVertexFormat : public RefCountBase<SystemVertexFormat, Stat_Default_Mem>
{
public:
    virtual ~SystemVertexFormat();
};


// TBD: Should we support vertex declarations with multiple streams; i.e.
// a color channel being specified in a separate array? Not for now.

// When vertex format is reported right now, it is expected to have global
// lifetime. Perhaps in the future this can have an allocated flag...

struct VertexFormat
{
    unsigned          Size;
    VertexElement*    pElements;
    Ptr<SystemVertexFormat> pSysFormat;

    bool operator == (const VertexFormat& o) const
    {
        const VertexElement *e0, *e1;
        for (e0 = pElements, e1 = o.pElements; ; e0++, e1++)
        {
            // Don't test I8 or I16 in equality, as they do not affect the buffer format.
            if (((e0->Attribute&VET_CompType_Mask) == VET_I8 ) ||
                ((e0->Attribute&VET_CompType_Mask) == VET_I16 ))
            {
                e0++;
            }
            if (((e1->Attribute&VET_CompType_Mask) == VET_I8 ) ||
                ((e1->Attribute&VET_CompType_Mask) == VET_I16 ))
            {
                e1++;
            }
            if ( e0->Attribute == VET_None ||
                 e1->Attribute == VET_None )
            {
                 break;
            }

            if (*e0 != *e1)
                return false;
        }
        return *e0 == *e1;
    }

    const VertexElement* GetElement(unsigned attrValue, unsigned attrMask) const
    {
         const VertexElement *e = pElements;
         while (e->Attribute != VET_None)
         {
             if ((e->Attribute & attrMask) == attrValue)
                 return e;
             e++;
         }
         return 0;
    }
    bool    HasUsage(VertexElementType usage) const
    {
        return GetElement(usage, VET_Usage_Mask) != 0;        
    }
};

// Converts vertices from the source to destination format based on format declarations.
// Each destination element that has VET_Argument_Flag flag set receives as extra
// argument from pargumentData array, which can be used to initialize it.
//   - arguments are consumed in the order of vertex elements, only when the VET_Argument_Flag is set.
void ConvertVertices(const VertexFormat& sourceFormat, void* psource,
                     const VertexFormat& destFormat, void* pdest,
                     unsigned count, void** pargumentData = 0);
// Same conversion as a above, but goes through a temporary buffer to minimize
// destination writes, which may be more efficient for video memory.
void ConvertVertices_Buffered(const VertexFormat& sourceFormat, void* psource,
                              const VertexFormat& destFormat, void* pdest,
                              unsigned count, void** pargumentData = 0);

UPInt ConvertVertices_Buffered(const VertexFormat& sourceFormat, UByte** psource,
                               const VertexFormat& destFormat, UByte* pdest, UPInt destBufferSize,
                               unsigned* count, void** pargumentData);

// Initializes vertex argument channels to specified argument values.
// This function can be used to initialize instance values or other identical value
// elements in an array of vertices efficiently.
void InitializeVertices(const VertexFormat& format, void* pvertices,
                        unsigned count, void** pargs);

// Copies and converts index buffer by adding delta to each index.
// Uses non-cached write to destination if availabe on this platform.
void ConvertIndices(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta);
void ConvertIndices_NonOpt(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta);

void CopyVertexElements(UByte* source, UPInt sourceFormatSize,
                        UByte* dest, UPInt destFormatSize,
                        UPInt elementSize, UPInt count);


// **** Common Vertex Formats

// Declared for convenience here

// Vertex with coordinates only
//------------------------------------------------------------------------
struct VertexXY16i
{
    SInt16  x,y;

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};

// Vertex with coordinates only (floating point)
//------------------------------------------------------------------------
struct VertexXY16f
{
    float x,y;

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};

// Vertex with coordinates (floating point) and alpha
//------------------------------------------------------------------------
struct VertexXY16fAlpha
{
    float   x,y;
    UInt8   Alpha[4];

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};

// Vertex with color
//------------------------------------------------------------------------
struct VertexXY16iC32
{
    SInt16  x,y;
    UInt32  Color;

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};


// Vertex with alpha
//------------------------------------------------------------------------
struct VertexXY16iAlpha
{
    SInt16  x,y;
    UInt8   Alpha[4];

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};

// Vertex with color and factors
//------------------------------------------------------------------------
struct VertexXY16iCF32
{
    SInt16  x,y;
    UInt32  Color;
    UInt8   Factors[4];

    static VertexElement VertexElements[5];
    static VertexFormat  Format;
};

// Vertex with instance index
//------------------------------------------------------------------------
struct VertexXY16iInstance
{
    SInt16  x,y;
    UInt8   Instance[4];

    static VertexElement VertexElements[4];
    static VertexFormat  Format;
};


// Vertex with explicit UVs
//------------------------------------------------------------------------
struct VertexXY16iUV
{
    SInt16  x,y;
    float   UV[2];

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};

}} // Scaleform::Render


// New Vertex formats:
//
//  Plain vertex: XY, i
//  Colored vertex: XY, C, i                    <--+ Merged?
//  Colored vertex with factors: XY, C, f, i    <--+
//
//  Colored vertex with texture: XY, UV, C, i

#endif
