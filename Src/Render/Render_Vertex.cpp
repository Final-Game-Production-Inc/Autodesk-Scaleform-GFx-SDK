/**************************************************************************

Filename    :   Render_Vertex.cpp
Content     :   Vertex Element declarations 
Created     :   August 17, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// This file provides Vertex Element declarations representing different vertex types,
// both on the renderer input and internal sides. 

#include "Render/Render_Vertex.h"

// 1400 -> MSVC 2005
#if defined(SF_CPU_X86) && defined(_MSC_VER)
#if (_MSC_VER >= 1400)
    #define SF_SSE2_SUPPORTED
#endif
#endif


namespace Scaleform { namespace Render {

void    ConvertVertexData_XY16i_XY32f(const VertexElement* psourceElement, const VertexElement* pdestElement,
                                      UByte* psource, unsigned sourceSize, unsigned sourceOffset,
                                      UByte* pdest, unsigned destSize, unsigned destOffset,
                                      unsigned count, const void*)
{
    SF_UNUSED2(psourceElement,pdestElement);

    UByte* psourceEnd = psource + sourceSize * count;

    while(psource < psourceEnd)
    {
        SInt16* ps = (SInt16*)(psource + sourceOffset);
        float*  pd = (float*)(pdest + destOffset);
        pd[0] = (float)ps[0];
        pd[1] = (float)ps[1];
        pdest   += destSize;
        psource += sourceSize;
    }
}

void    ConvertVertexData_XY16i_XYZ16i(const VertexElement* psourceElement, const VertexElement* pdestElement,
                                       UByte* psource, unsigned sourceSize, unsigned sourceOffset,
                                       UByte* pdest, unsigned destSize, unsigned destOffset,
                                       unsigned count, const void*)
{
    SF_UNUSED2(psourceElement,pdestElement);

    UByte* psourceEnd = psource + sourceSize * count;

    while(psource < psourceEnd)
    {
        SInt16* ps = (SInt16*)(psource + sourceOffset);
        SInt16* pd = (SInt16*)(pdest + destOffset);
        pd[0] = ps[0];
        pd[1] = ps[1];
        pd[2] = 0; // May need to pass an argument constant here.
        pdest   += destSize;
        psource += sourceSize;
    }
}

void    ConvertVertexData_ARGB_RGBA(const VertexElement* psourceElement, const VertexElement* pdestElement,
                                    UByte* psource, unsigned sourceSize, unsigned sourceOffset,
                                    UByte* pdest, unsigned destSize, unsigned destOffset,
                                    unsigned count, const void*)
{
    SF_UNUSED2(psourceElement,pdestElement);

    UByte* psourceEnd = psource + sourceSize * count;

    while(psource < psourceEnd)
    {
        UInt32* ps = (UInt32*)(psource + sourceOffset);
        UInt32* pd = (UInt32*)(pdest + destOffset);
#if (SF_BYTE_ORDER == SF_BIG_ENDIAN)
        *pd = (*ps >> 24) | ((*ps & 0xffffff) << 8);
#else
        *pd = (*ps & 0xff00ff00) | ((*ps >> 16) & 0xff) | ((*ps & 0xff) << 16);
#endif
        pdest   += destSize;
        psource += sourceSize;
    }
}

// Initializes vertex data from argument, ignoring source
void    InitVertexData_U8(const VertexElement* psourceElement, const VertexElement* pdestElement,
                          UByte*, unsigned, unsigned, 
                          UByte* pdest, unsigned destSize, unsigned destOffset,
                          unsigned count, const void* parg)
{
    SF_UNUSED2(psourceElement,pdestElement);

    UByte* pdestEnd = pdest + destSize * count;
    UByte  arg      = *(const UByte*)parg;

    while(pdest < pdestEnd)
    {
        *(pdest + destOffset) = arg;
        pdest   += destSize;
    }    
}


// TBD: Can some of the converters take scale factors, such as the shape scale
//      that was applied when computing stroke?
// We need to think through how this conversion could affect the 


typedef void  (*ConvertVertexDataFunc)(const VertexElement* psourceElement, const VertexElement* pdestElement,
                                       UByte* psource, unsigned sourceSize, unsigned sourceOffset,
                                       UByte* pdest, unsigned destSize, unsigned destOffset,
                                       unsigned count, const void* parg);

struct ConvertTarget
{
    VertexElementType     SourceType;
    VertexElementType     TargetType;
    unsigned              TargetMask;
    ConvertVertexDataFunc pConvertFunc;
};

ConvertTarget   Convert_None[] =
{    
    { VET_None,         VET_None,       VET_Type_Mask, 0 }
};
ConvertTarget   Convert_Pos[] = 
{
    { VET_XY16i,        VET_XYZ16i,     VET_Type_Mask, ConvertVertexData_XY16i_XYZ16i },
    { VET_XY16i,        VET_XY32f,      VET_Type_Mask, ConvertVertexData_XY16i_XY32f },
    { VET_None,         VET_None,       VET_Type_Mask, 0 }
};
ConvertTarget   Convert_Color[] = 
{
    { VET_ColorARGB8,   VET_ColorRGBA8, VET_CompType_Mask|VET_Usage_Mask|VET_Components_Mask, ConvertVertexData_ARGB_RGBA },
    { VET_ColorRGBA8,   VET_ColorARGB8, VET_CompType_Mask|VET_Usage_Mask|VET_Components_Mask, ConvertVertexData_ARGB_RGBA },
    { VET_None,         VET_None,       VET_Type_Mask, 0 }
};
ConvertTarget   Convert_Instance[] = 
{
    { VET_Instance,     VET_Instance8,  VET_Type_Mask|VET_Argument_Flag,  InitVertexData_U8 },    
    { VET_None,         VET_None,       VET_Type_Mask, 0 }
};

static ConvertTarget* VertexConvertTable[] = 
{
    Convert_None,
    Convert_Pos,
    Convert_Color,
    Convert_None,       // TexCoord
    Convert_Instance,
    Convert_None
};

// This copy special-cases typical sizes, as memcpy() in a loop for
// small size is highly inefficient.
void CopyVertexElements(UByte* source, UPInt sourceFormatSize,
                        UByte* dest, UPInt destFormatSize,
                        UPInt elementSize, UPInt count)
{
    UByte* sourceEnd = source + sourceFormatSize * count;

    switch(elementSize)
    {
    case 1:
        while(source < sourceEnd)
        {
            *dest = *source;
            dest   += destFormatSize;
            source += sourceFormatSize;
        }
        break;
    case 2:
        while(source < sourceEnd)
        {
            *(UInt16*)dest = *(UInt16*)source;
            dest   += destFormatSize;
            source += sourceFormatSize;
        }
        break;
    case 4:
        while(source < sourceEnd)
        {
            *(UInt32*)dest = *(UInt32*)source;
            dest   += destFormatSize;
            source += sourceFormatSize;
        }
        break;

    default:
        while(source < sourceEnd)
        {
            memcpy(dest, source, elementSize);
            dest   += destFormatSize;
            source += sourceFormatSize;
        }
        break;
    }
}


void ConvertVertices(const VertexFormat& sourceFormat, void* psource,
                     const VertexFormat& destFormat, void* pdest,
                     unsigned count, void** pargs)
{
    const VertexElement* pdestElement;
    const VertexElement* psourceElement;

    // We could do an iteration here for a sub-range of vertices to be efficient.

    for (pdestElement = destFormat.pElements; pdestElement->Attribute != VET_None; pdestElement++)
    {
        unsigned destUsage = pdestElement->Attribute & (VET_Usage_Mask|VET_Index_Mask);
        unsigned destType  = pdestElement->Attribute & VET_CompType_Mask;

        // Do not attempt to convert instancing data.
        if ( destType == VET_I8 || destType == VET_I16 )
            continue;

        // For all conversion functions available for this destination,
        // check if a conversion is available from the source type.
        ConvertTarget* pconvert = VertexConvertTable[(pdestElement->Attribute & VET_Usage_Mask) >> VET_Usage_Shift];

        // Find the matching usage in the source declaration element.
        for (psourceElement = sourceFormat.pElements; psourceElement->Attribute != VET_None; psourceElement++)
        {
            if ((psourceElement->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == destUsage)
                break;
        }

        while(pconvert->TargetType != VET_None)
        {
            VertexElementType sourceAttribute = (VertexElementType)psourceElement->Attribute;

            if (((pdestElement->Attribute & VET_Argument_Flag) || (sourceAttribute == pconvert->SourceType))
                && (pdestElement->Attribute & pconvert->TargetMask) == (unsigned)pconvert->TargetType)
            {
                // We found the conversion function.
                pconvert->pConvertFunc(psourceElement, pdestElement,
                    (UByte*)psource, sourceFormat.Size, psourceElement->Offset,
                    (UByte*)pdest, destFormat.Size, pdestElement->Offset,
                    count, pargs ? *pargs : 0);
                goto conversion_done;
            }

            pconvert++;
        }

        // Copy from matching source element
        SF_ASSERT(psourceElement->Attribute != VET_None);
        SF_ASSERT(psourceElement->Attribute == pdestElement->Attribute);

        UByte* pdestPtr, *psourcePtr;
        pdestPtr = ((UByte*)pdest) + pdestElement->Offset;
        psourcePtr = ((UByte*)psource)+ psourceElement->Offset;
        CopyVertexElements(psourcePtr, sourceFormat.Size,
                           pdestPtr, destFormat.Size, psourceElement->Size(), count);

conversion_done:
        ;
    }
}

void ConvertVertices_Buffered(const VertexFormat& sourceFormat, void* psource,
                              const VertexFormat& destFormat, void* pdest,
                              unsigned count, void** pargumentData)
{
#if defined(SF_OS_3DS) || defined(SF_OS_WII)
    ConvertVertices(sourceFormat, psource, destFormat, pdest, count, pargumentData);

#else
    union {
        UByte TempBuffer[1024 * 8];
        UPInt Align;
    };
    SF_UNUSED(Align);

    unsigned destVertexSize = destFormat.Size;
    unsigned copyVertices   = sizeof(TempBuffer) / destVertexSize;
    UByte *ps = (UByte*)psource, *pd = (UByte*)pdest;

    while(count)
    {
        if (copyVertices > count)
            copyVertices = count;
        ConvertVertices(sourceFormat, ps, destFormat, TempBuffer,
                        copyVertices, pargumentData);
        memcpy(pd, TempBuffer, copyVertices * destVertexSize);
        ps += copyVertices * sourceFormat.Size;
        pd += copyVertices * destVertexSize;
        count -= copyVertices;
    }
#endif
}

UPInt ConvertVertices_Buffered(const VertexFormat& sourceFormat, UByte** psource,
                               const VertexFormat& destFormat, UByte* pdest, UPInt destBufferSize,
                               unsigned* count, void** pargumentData)
{
    unsigned destVertexSize = destFormat.Size;
    unsigned copyVertices   = (unsigned)(destBufferSize / destVertexSize);

    if (copyVertices > *count)
        copyVertices = *count;
    ConvertVertices(sourceFormat, *psource, destFormat, pdest, copyVertices, pargumentData);
    *psource += copyVertices * sourceFormat.Size;
    *count -= copyVertices;
    return copyVertices * destVertexSize;
}

// Initializes vertex argument channels to specified values.

void InitializeVertices(const VertexFormat& format, void* pvertices,
                        unsigned count, void** pargs)
{
    const VertexElement* pelement;    

    // We could do an iteration here for a sub-range of vertices to be efficient.
    for (pelement = format.pElements; pelement->Attribute != VET_None; pelement++)
    {
        if (pelement->Attribute & VET_Argument_Flag)
        {
            ConvertTarget* pconvert = 
                VertexConvertTable[(pelement->Attribute & VET_Usage_Mask) >> VET_Usage_Shift];

            while(pconvert->SourceType != VET_None)
            {
                if ((pelement->Attribute & pconvert->TargetMask) == (unsigned)pconvert->TargetType)
                {
                    // We found the conversion function.
                    pconvert->pConvertFunc(0, pelement, 0,0,0,
                        (UByte*)pvertices, format.Size, pelement->Offset,
                        count, *pargs);
                    break;
                }

                pconvert++;
            }
        }
    }
}


// Copies and converts index buffer by adding 'delta' to each index.
// Used non-cached write to destination if possible.
#ifdef SF_ENABLE_SIMD
void ConvertIndices_SIMD(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta)
{
    using namespace Scaleform::SIMD;

    // We can handle misalignment, but not misalignment between the two input buffers.
    if ( (reinterpret_cast<size_t>(pdest)&15) != (reinterpret_cast<size_t>(psource)&15))
    {
        ConvertIndices_NonOpt(pdest, psource, count, delta);
        return;
    }

    UInt16* sourceEnd = psource + count;
    UPInt   destAlignBegin = (((UPInt)pdest) + 15) & ~(UPInt)15;
    UPInt   destAlignEnd   = ((UPInt)(pdest + count)) & ~(UPInt)15;

    if (destAlignBegin < destAlignEnd)
    {
        // Copy heading bytes before alignment.
        while(pdest < (UInt16*)destAlignBegin)
            *(pdest++) = (*psource++) + delta;

        // Expand delta into each word.
        Vector4i mdelta = IS::Set1( delta );

        do
        {
            Vector4i val = IS::LoadAligned((Vector4i*)psource);
            // Streaming improves performance here over store,
            // as we typically write to HW index buffer.
            IS::StreamAligned((Vector4i*)pdest, IS::Add16(val, mdelta));
            psource += sizeof(Vector4i) / sizeof(UInt16);
            pdest   += sizeof(Vector4i) / sizeof(UInt16);
        } while(pdest < (UInt16*)destAlignEnd);
    }

    // Copy tail remainder.
    while(psource < sourceEnd)
        *(pdest++) = (*psource++) + delta;
}
#endif

void ConvertIndices_NonOpt(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta)
{
    // Generic slightly unrolled implementation.
    switch(count & 3)
    {
    case 3: *(pdest++) = *(psource++) + delta;
    case 2: *(pdest++) = *(psource++) + delta;
    case 1: *(pdest++) = *(psource++) + delta;
    }
    count &= ~3;    
    for(UInt16* sourceEnd = psource + count; psource < sourceEnd; psource+=4, pdest+=4)
    {
        pdest[0] = psource[0] + delta;
        pdest[1] = psource[1] + delta;
        pdest[2] = psource[2] + delta;
        pdest[3] = psource[3] + delta;
    }
}

void ConvertIndices(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta)
{
#ifdef SF_ENABLE_SIMD
    // Certain platforms have dynamic detection for support of integer intrinsics. (Win32-SSE2 support)
    // ConvertIndices uses these heavily, if they are not available, use the unoptimized version.
    if ( SIMD::IS::SupportsIntegerIntrinsics() )
        ConvertIndices_SIMD(pdest, psource, count, delta);
    else
        ConvertIndices_NonOpt(pdest, psource, count, delta);
#else
    ConvertIndices_NonOpt(pdest, psource, count, delta);
#endif
}

SystemVertexFormat::~SystemVertexFormat()
{
}

// **** Common Vertex Formats

VertexElement VertexXY16i::VertexElements[] =
{
    { 0, {VET_XY16i} },
    { 0, {VET_None} }
};
VertexFormat VertexXY16i::Format = 
{ sizeof(VertexXY16i), VertexXY16i::VertexElements };

// Vertex with coordinates only (floating point)
//------------------------------------------------------------------------
VertexElement VertexXY16f::VertexElements[] =
{
    { 0, {VET_XY32f} },
    { 0, {VET_None} }
};

VertexFormat VertexXY16f::Format = 
{ sizeof(VertexXY16f), VertexXY16f::VertexElements };

VertexElement VertexXY16fAlpha::VertexElements[] =
{
    { 0, {VET_XY32f} },
    { 8, {VET_FactorAlpha8} },
    { 0, {VET_None} }
};

VertexFormat VertexXY16fAlpha::Format = 
{ sizeof(VertexXY16fAlpha), VertexXY16f::VertexElements };

VertexElement VertexXY16iC32::VertexElements[] =
{
    { 0, {VET_XY16i} },
    { 4, {VET_ColorARGB8} },
    { 0, {VET_None} }
};
VertexFormat VertexXY16iC32::Format = 
{ sizeof(VertexXY16iC32), VertexXY16iC32::VertexElements };


VertexElement VertexXY16iAlpha::VertexElements[] =
{
    { 0, {VET_XY16i} },
    { 4, {VET_FactorAlpha8} },
    { 0, {VET_None} }
};
VertexFormat VertexXY16iAlpha::Format = 
{ sizeof(VertexXY16iAlpha), VertexXY16iAlpha::VertexElements };


VertexElement VertexXY16iCF32::VertexElements[] =
{
    { 0, {VET_XY16i} },
    { 4, {VET_ColorARGB8} },
    { 8, {VET_FactorAlpha8} },
    { 9, {VET_T0Weight8} },
    { 0, {VET_None} }
};
VertexFormat VertexXY16iCF32::Format = 
{ sizeof(VertexXY16iCF32), VertexXY16iCF32::VertexElements };


VertexElement VertexXY16iInstance::VertexElements[] =
{
    { 0, {VET_XY16i} },
    { 4, {VET_Instance8} },
    { 0, {VET_None} }
};
VertexFormat VertexXY16iInstance::Format = 
{ sizeof(VertexXY16iInstance), VertexXY16iInstance::VertexElements };

VertexElement VertexXY16iUV::VertexElements[] =
{
    { 0, {VET_XY16i} },
    { 4, {VET_UV32f} },
    { 0, {VET_None} }
};
VertexFormat VertexXY16iUV::Format = 
{ sizeof(VertexXY16iUV), VertexXY16iUV::VertexElements };

}} // Scaleform::Render



