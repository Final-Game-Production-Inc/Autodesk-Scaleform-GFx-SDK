/**************************************************************************

Filename    :   GFx_ShapeSwf.cpp
Content     :   SWF (Shockwave Flash) shape logic
Created     :   Sep 1, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx_ShapeSwf.h"
#include "GFx/GFx_StreamContext.h"
//#include "GFx/GFx_Styles.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_ResourceHandle.h"
#include "GFx/GFx_DisplayObject.h"
#include "Render/Render_Math2D.h"
#include "Render/Render_HitTest.h"
#include "Kernel/SF_Debug.h"
#include "Render/Render_ShapeDataFloat.h"
#include "Render/Render_BoundBox.h"

namespace Scaleform { namespace GFx {

typedef Array<FillStyleType, StatMD_ShapeData_Mem> FillStyleArrayTemp;
typedef Array<StrokeStyleType, StatMD_ShapeData_Mem> StrokeStyleArrayTemp;

#ifdef SF_BUILD_DEBUG
//#define GFX_SHAPE_MEM_TRACKING
#endif  //SF_BUILD_DEBUG

#ifdef GFX_SHAPE_MEM_TRACKING
unsigned GFx_MT_SwfGeomMem = 0;
unsigned GFx_MT_PathsCount = 0;
unsigned GFx_MT_GeomMem = 0;
unsigned GFx_MT_ShapesCount = 0;
#endif //GFX_SHAPE_MEM_TRACKING


//////////////////////////////////////////////////////////////////////////
// Shape I/O helper functions.
//
// Read fill styles, and push them onto the given style Array.
// Reader for SWF fill styles. Reads to Render::FillStyleType
class FillStyleSwfReader
{
    enum FillFlagConstants
    {
        FF_LinearRGB        = 0x01,
        FF_HasAlpha         = 0x02
    };

    FillStyleType& FillStyle;
public:
    FillStyleSwfReader(FillStyleType& fillStyle) : FillStyle(fillStyle) {}

    bool HasTextureFill() const { return FillStyle.pFill && (FillStyle.pFill->pImage || FillStyle.pFill->BindIndex != ~0u); }
    bool NeedsResolving() const { return FillStyle.pFill && FillStyle.pFill->BindIndex != ~0u; }

    void    Read(LoadProcess* p, TagType tagType)
    {
        UInt8 type        = p->ReadU8();
        unsigned fillFlags= 0;

        p->LogParse("  FillStyle read type = 0x%X\n", type);

        if (type == Fill_Solid)
        {
            // Read solid fill color.
            Color color;
            p->ReadRgbaTag(&color, tagType);

            p->LogParse("  color: ");
            p->GetStream()->LogParseClass(color);

            if (color.GetAlpha() < 255)
                fillFlags |= FF_HasAlpha;

            FillStyle.Color = color.Raw;
            FillStyle.pFill = NULL;
        }

        else if (type & Fill_Gradient_TestBit)
        {
//            SF_ASSERT(0);

            // 0x10: linear gradient fill
            // 0x12: radial gradient fill
            // 0x13: focal-point gradient fill for SWF 8

            Matrix2F imageMatrix;
            p->GetStream()->ReadMatrix(&imageMatrix);

            // convert swf matrix to texture UV one
            Matrix2F textureMatrix;
            if (type == Fill_LinearGradient)
            {
                textureMatrix.AppendScaling(1.0f / 32768.F);
                textureMatrix.AppendTranslation(0.5F, 0.F);
            }
            else
            {
                textureMatrix.AppendScaling(1.0f / 32768.F);
                textureMatrix.AppendTranslation(0.5F, 0.5F);
                // gradient's square size is 32768x32768 (-16,384 - 16,384)
            }

            // When using scale9grid with gradients or images, 
            // the image matrix has to be adjusted according to the 
            // "logical difference" between the world matrix and the 
            // central part of the scale9grid. The s9gImgAdjust (if not null)
            // represents this adjustment matrix.
            //imgMtx = ImageMatrix;
            //if (s9gImgAdjust)
            //    imgMtx.Append(*s9gImgAdjust);

            textureMatrix.Prepend(imageMatrix.GetInverse());

            // GRADIENT
            UInt16 numGradients = p->ReadU8();

            // For DefineShape5 gradient records are interpreted differently.
            // Note that Flash 8 can include these extra bits even for old tag types.
            if (numGradients & 0x10)
                fillFlags |= FF_LinearRGB;
            // TBD: Other bits may be set here, such as 0x80 for focal point
            // Do they need custom processing?
            numGradients &= 0xF;

            p->LogParse("  gradients: numGradients = %d\n", numGradients);

            // SWF 8 allows up to 15 points.
            SF_ASSERT(numGradients >= 1 && numGradients <= 15);

            FillStyle.pFill = *SF_NEW ComplexFill(); // heap? stat id? @TODO
            GradientType gradType;
            switch (type)
            {
            case Fill_LinearGradient: gradType = GradientLinear; break;
            case Fill_RadialGradient: gradType = GradientRadial; break;
            case Fill_FocalPointGradient: gradType = GradientFocalPoint; break;
            default: gradType = GradientLinear; SF_ASSERT(0); //?
            }
            FillStyle.pFill->pGradient = *SF_NEW GradientData(gradType, numGradients, (fillFlags & FF_LinearRGB) != 0);

            for (int i = 0; i < numGradients; i++)
            {
                UInt8 ratio = p->ReadU8();
                Color color;
                p->ReadRgbaTag(&color, tagType);

                FillStyle.pFill->pGradient->At(i).ColorV = color;
                FillStyle.pFill->pGradient->At(i).Ratio  = ratio;
                // Flag if the gradient has alpha channel.
                if (color.GetAlpha() < 255)
                     fillFlags |= FF_HasAlpha;
            }

            // Record focal point.          
            if (type == Fill_FocalPointGradient)           
                FillStyle.pFill->pGradient->SetFocalRatio((float) (p->ReadS16()) / 256.0f);

            FillStyle.pFill->ImageMatrix = textureMatrix;
        }

        else if (type & Fill_Image_TestBit)
        {
            // 0x40: tiled bitmap fill
            // 0x41: clipped bitmap fill
            // 0x42: non-smoothed tiled fill
            // 0x43: non-smoothed clipped bitmap fill

            ResourceId bitmapResourceId = ResourceId(p->ReadU16());
            p->LogParse("  BitmapChar = %d\n", bitmapResourceId.GetIdIndex());

            Matrix2F imageMatrix;
            p->GetStream()->ReadMatrix(&imageMatrix);

            // Always 1 for now, should probably check formats in the future.
            fillFlags |= FF_HasAlpha;        
            p->GetStream()->LogParseClass(imageMatrix);

            //      
            ResourcePtr<ImageResource> imageRes;
            Image* pimage = 0;
            // convert swf matrix to texture one
            // the matrix in the swf transforms bitmap's pixels into shape coordinates.
            // Renderer needs textureMatrix that transforms shape coordinates into texture ones.
            // Actually, texture coordinates should be in UV, but ShapeMeshProvider::GetFillMatrix
            // takes care about this scaling; here, we just need to get inverse matrix (that
            // transforms shape coords into bitmap's pixels).
            Matrix2F textureMatrix(imageMatrix.GetInverse());
            FillStyle.pFill = *SF_NEW ComplexFill(); // heap? stat id? @TODO
            FillStyle.pFill->ImageMatrix = textureMatrix;
            switch( type )
            {
            case Fill_TiledImage:           FillStyle.pFill->FillMode.Fill = ( Wrap_Repeat | Sample_Point ); break;
            case Fill_TiledSmoothImage:     FillStyle.pFill->FillMode.Fill = ( Wrap_Repeat | Sample_Linear ); break;
            case Fill_ClippedImage:         FillStyle.pFill->FillMode.Fill = ( Wrap_Clamp | Sample_Point ); break;
            case Fill_ClippedSmoothImage:   FillStyle.pFill->FillMode.Fill = ( Wrap_Clamp | Sample_Linear ); break;
            }

            if (!p->GetResourceHandle(&imageRes, bitmapResourceId) || !imageRes.GetResourcePtr())
            {
                // Should not happen..
                // New image ??
                FillStyle.Color = 0xFFA7DD39; //???

                if (imageRes.IsIndex())
                {
                    FillStyle.pFill->BindIndex = imageRes.GetBindIndex();
                    TempBindData* bd = p->GetTempBindData();
                    if (bd && (type == Fill_TiledImage || type == Fill_TiledSmoothImage))
                        bd->FillStyleImageWrap.Add(FillStyle.pFill->BindIndex);
                }
                else
                {
                    FillStyle.pFill = 0;
                    if (bitmapResourceId.GetIdValue() != 0xFFFF) // weird value that is something like "new bitmap" flag
                    {
                        LogDebugMessage(Log_Warning, "An image with resource id %d is not found in resource table.",
                            bitmapResourceId.GetIdValue());
                    }

                    //@TODO. FillStyle holds a pointer to Image, but in the case of dynamic binding (when KeepBindingData
                    // flag is set) we can't get a pointer to the Image at the loading stage; instead, it is 
                    // necessary to resolve it during GetFillStyle call of ShapeInterface interface.
                    // If the index is detected as a resource then new ShapeMeshProvider should be created for such
                    // shapes (do not use one from ShapeDef/do not create one in ShapeDef). The ShapeMeshProvider 
                    // should contain info enough to resolve the resource by index (resource binding, defimpl). This
                    // data should be passed to GetFillStyle/GetStrokeStyle methods. Fill style stored inside the shape
                    // should contain a resource index instead of pImage pointer, but the methods will return 
                    // a fillstyle with resolved image.
                    // To avoid creation of separate ShapeMeshProviders for each instance of such shape we would need
                    // to hold a hash table of all shapemeshproviders inside the defimpl: (ShapeInterface,ShapeMeshProvider) pair.
                }
            }
            else
            {
                if (imageRes.GetResourcePtr() && imageRes.GetResourcePtr()->GetResourceType() == Resource::RT_Image)
                {
                    pimage = static_cast<Image*>(static_cast<ImageResource*>(imageRes.GetResourcePtr())->GetImage());
                    FillStyle.pFill->pImage = pimage;
                }
            }
        }
    }
private:
    FillStyleSwfReader& operator=(const FillStyleSwfReader&) { return *this; }
};

// Reader for SWF stroke styles. Reads to Render::StrokeStyleType
class StrokeStyleSwfReader
{
    enum Style
    {
        LineFlag_StrokeHinting  = 0x0001,
        // Scaling
        LineScaling_Normal      = 0x0000,
        LineScaling_Horizontal  = 0x0002,
        LineScaling_Vertical    = 0x0004,
        LineScaling_None        = 0x0006,
        LineScaling_Mask        = 0x0006,

        // Complex fill flag
        LineFlag_ComplexFill    = 0x0008,

        // Joins
        LineJoin_Round          = 0x0000,
        LineJoin_Bevel          = 0x0010,
        LineJoin_Miter          = 0x0020,
        LineJoin_Mask           = 0x0030,

        // Line start caps
        LineCap_Round           = 0x0000,
        LineCap_None            = 0x0040,
        LineCap_Square          = 0x0080,
        LineCap_Mask            = 0x00C0,

        // Line end caps (internal use only)
        LineEndCap_Round        = 0x0000,
        LineEndCap_None         = 0x0100,
        LineEndCap_Square       = 0x0200,
        LineEndCap_Mask         = 0x0300,
        LineEndCap_ShiftToCap   = 2
    };

    StrokeStyleType& StrokeStyle;
public:
    StrokeStyleSwfReader(StrokeStyleType& strokeStyle) : StrokeStyle(strokeStyle) {}

    bool HasTextureFill() const { return StrokeStyle.pFill && (StrokeStyle.pFill->pImage || StrokeStyle.pFill->BindIndex != ~0u); }
    bool NeedsResolving() const { return StrokeStyle.pFill && StrokeStyle.pFill->BindIndex != ~0u; }

    static unsigned ConvertSwfLineStyles(unsigned swfLineStyle)
    {
        unsigned flags = 0;
        // convert SWF styles to StrokeStyleType flags
        if (swfLineStyle & LineFlag_StrokeHinting)    flags |= StrokeFlag_StrokeHinting;
        if (swfLineStyle & LineScaling_Horizontal)    flags |= StrokeScaling_Horizontal;
        if (swfLineStyle & LineScaling_Vertical)      flags |= StrokeScaling_Vertical;
        if ((swfLineStyle & LineScaling_None) == LineScaling_None)          
        {
            flags = StrokeScaling_None;
        }

        SF_ASSERT(LineJoin_Round == 0 && StrokeJoin_Round == 0);
        //if (swfLineStyle & LineJoin_Round)            flags |= StrokeJoin_Round;
        if (swfLineStyle & LineJoin_Bevel)            flags |= StrokeJoin_Bevel;
        if (swfLineStyle & LineJoin_Miter)            flags |= StrokeJoin_Miter;

        SF_ASSERT(LineCap_Round == 0 && StrokeCap_Round == 0);
        //if (swfLineStyle & LineCap_Round)             flags |= StrokeCap_Round;
        if (swfLineStyle & LineCap_None)              flags |= StrokeCap_None;
        if (swfLineStyle & LineCap_Square)            flags |= StrokeCap_Square;

        SF_ASSERT(LineEndCap_Round == 0 && StrokeEndCap_Round == 0);
        //if (swfLineStyle & LineEndCap_Round)          flags |= StrokeEndCap_Round;
        if (swfLineStyle & LineEndCap_None)           flags |= StrokeEndCap_None;
        if (swfLineStyle & LineEndCap_Square)         flags |= StrokeEndCap_Square;

        if (swfLineStyle & LineFlag_ComplexFill)      flags |= StrokeFlag_ComplexFill;
        return flags;
    }

    void    Read(LoadProcess* p, TagType tagType)
    {
        // the width is in twips!
        unsigned width = p->ReadU16(); 
        float miterSize = Default_MiterSize;

        UInt16 styleFlags = 0;

        // Extra values come here for DefineShape5.
        if (tagType == Tag_DefineShape4)
        {
            styleFlags = p->ReadU16();

            // Miter is followed by a special 16.16 value
            if (styleFlags & LineJoin_Miter)
            {
                miterSize = (float)(p->ReadU16()) / 256.0f;
            }
        }

        // convert SWF styles to StrokeStyleType flags
        unsigned flags = ConvertSwfLineStyles(styleFlags);

        // If we have complex SWF 8 fill style, read it in.
        if (flags & StrokeFlag_ComplexFill)
        {
            //SF_ASSERT(0);
            FillStyleType      fs;
            FillStyleSwfReader fsr(fs);
            fsr.Read(p, tagType);

            StrokeStyle.Color = fs.Color;
            StrokeStyle.Miter = miterSize;
            StrokeStyle.Width = (float)width; // *0.05f

            StrokeStyle.pFill = fs.pFill;

            // Avoid ASSERT in GetColor() for gradient strokes.
            if (!fs.pFill)
                StrokeStyle.Color = fs.Color;
            else if (fs.pFill->pGradient)
            {
                GradientData *pgdata = fs.pFill->pGradient;
                if (pgdata && pgdata->GetRecordCount() > 0)
                    StrokeStyle.Color = pgdata->At(0).ColorV.ToColor32();
            }
        }
        else
        {
            Color color;
            // Usual case: solid color line.
            p->ReadRgbaTag(&color, tagType);

            StrokeStyle.Color = color.ToColor32();
            StrokeStyle.Miter = miterSize;
            StrokeStyle.pFill = NULL;
            StrokeStyle.Width = (float)width; // *0.05f
        }
        StrokeStyle.Units = 0.05f;
        StrokeStyle.Flags = flags;
    }

private:
    StrokeStyleSwfReader& operator=(const StrokeStyleSwfReader&) { return *this; }
};

// for external use (from MorphShape, for example).
unsigned ConvertSwfLineStyles(unsigned swfLineStyle)
{
    return StrokeStyleSwfReader::ConvertSwfLineStyles(swfLineStyle);
}


#if 0 //defined(SF_BUILD_DEBUG)
#define LOG_SHAPE0(s)                   printf(s)
#define LOG_SHAPE1(s,p1)                printf(s,p1)
#define LOG_SHAPE2(s,p1,p2)             printf(s,p1,p2)
#define LOG_SHAPE3(s,p1,p2,p3)          printf(s,p1,p2,p3)
#define LOG_SHAPE4(s,p1,p2,p3,p4)       printf(s,p1,p2,p3,p4)
#define LOG_SHAPE5(s,p1,p2,p3,p4,p5)    printf(s,p1,p2,p3,p4,p5)
#define LOG_SHAPE6(s,p1,p2,p3,p4,p5,p6) printf(s,p1,p2,p3,p4,p5,p6)
#else
#define LOG_SHAPE0(s)                   
#define LOG_SHAPE1(s,p1)                
#define LOG_SHAPE2(s,p1,p2)             
#define LOG_SHAPE3(s,p1,p2,p3)          
#define LOG_SHAPE4(s,p1,p2,p3,p4)       
#define LOG_SHAPE5(s,p1,p2,p3,p4,p5)    
#define LOG_SHAPE6(s,p1,p2,p3,p4,p5,p6) 
#endif

// A decoder fro swf shape data. Decodes the data was read by SwfShapeReader.
class SwfShapeDecoder
{
    StreamContext   Stream;
    ShapePosInfo*   Pos;
public:
    enum { Signature = 0x07654321 };

    SwfShapeDecoder(ShapePosInfo* pos, const UByte* shapeData, float sfactor)
        : Stream(shapeData), Pos(pos)
    {
        if (!Pos->Initialized)
        {
            // read beginning of shape record and store data in Pos
#ifdef SF_BUILD_DEBUG
            // Read a 4-bytes signature to ensure we use correct path format.
            UInt32 sig;
            sig = Stream.ReadU32();
            SF_UNUSED(sig);
            SF_ASSERT(sig == Signature);
#endif

            Pos->Sfactor    = sfactor; // *0.05f

            Pos->FillBase = Pos->StrokeBase = 0;
            if (pos->Pos == 0)
            {
                Pos->NumFillBits = Stream.ReadUInt4();
                Pos->NumStrokeBits = Stream.ReadUInt4();
            }
            else
            {
                // numfillbits and numstrokebits are stored as part of position
                Pos->NumFillBits   = (pos->Pos >> 7) & 0xF;
                Pos->NumStrokeBits = (pos->Pos >> 3) & 0xF;
                Stream.CurByteIndex = (pos->Pos >> 11);
                Stream.CurBitIndex  = (pos->Pos & 7);
            }
            Pos->LastX = Pos->LastY = 0;
            Pos->Fill0 = Pos->Fill1 = Pos->Stroke = 0;
            Pos->Initialized = true;
        }
        else
        {
            Stream.CurByteIndex = (pos->Pos >> 11);
            Stream.CurBitIndex  = (pos->Pos & 7);
        }
    }
    ~SwfShapeDecoder()
    {
        SF_ASSERT(Stream.CurByteIndex < (1L << 21)); // only 2MB shape data is allowed
        Pos->Pos = (unsigned(Stream.CurByteIndex) << 11) | ((Pos->NumFillBits & 0xF) << 7) | 
                   ((Pos->NumStrokeBits & 0xF) << 3) | (Stream.CurBitIndex & 7);
    }
    bool IsEdgeNext() const 
    {
        return (Stream.pData[Stream.CurByteIndex] & (1 << (7 - Stream.CurBitIndex))) != 0; 
    }

    struct Edge
    {
        int Cx, Cy, Ax, Ay;
        bool  Curve;
    };
    PathEdgeType ReadEdge(Edge *edge)
    {
        SF_ASSERT(IsEdgeNext());
        SF_ASSERT(edge);
        bool typeFlag = Stream.ReadUInt1(); // skip typeFlag
        SF_ASSERT(typeFlag);
        SF_UNUSED(typeFlag);

        // EDGERECORD
        int edgeFlag = Stream.ReadUInt1();
        if (edgeFlag == 0)
        {
            // curved GFxEdge
            unsigned numbits = 2 + Stream.ReadUInt4();

            int cx = Stream.ReadSInt(numbits);
            int cy = Stream.ReadSInt(numbits);
            int ax = Stream.ReadSInt(numbits);
            int ay = Stream.ReadSInt(numbits);
            
            edge->Cx = cx + Pos->LastX;
            edge->Cy = cy + Pos->LastY;
            Pos->LastX += cx + ax;
            Pos->LastY += cy + ay;
            edge->Ax = Pos->LastX;
            edge->Ay = Pos->LastY;
            edge->Curve = true;

            LOG_SHAPE5("  SHAPE decode: curved edge, numbits = %d,  %d %d - %d %d\n", 
                numbits, edge->Cx, edge->Cy, edge->Ax, edge->Ay);
            return Edge_QuadTo;
        }
        else
        {
            // straight GFxEdge
            unsigned numbits = 2 + Stream.ReadUInt4();
            int lineFlag = Stream.ReadUInt1();
            edge->Curve = false;
            int dx = 0, dy = 0;
            if (lineFlag)
            {
                // General line.
                dx = Stream.ReadSInt(numbits);
                dy = Stream.ReadSInt(numbits);
            }
            else
            {
                int vertFlag = Stream.ReadUInt1();
                if (vertFlag == 0) {
                    // Horizontal line.
                    dx = Stream.ReadSInt(numbits);
                } else {
                    // Vertical line.
                    dy = Stream.ReadSInt(numbits);
                }
            }

            edge->Ax = dx + Pos->LastX;
            edge->Ay = dy + Pos->LastY;
            Pos->LastX += dx;
            Pos->LastY += dy;
            LOG_SHAPE4("  SHAPE decode: straight edge, numbits = %d, lineFl = %d, Dx = %d, Dy = %d\n", 
                numbits, lineFlag, edge->Ax, edge->Ay);
            return Edge_LineTo;
        }
    }

    // the parameter retVal is the value that will be returned id neither NewShape or NewLayer
    // conditions are detected.
    ShapePathType ReadNonEdgeRec(ShapePathType retVal = Shape_NewPath)
    {
        SF_ASSERT(!IsEdgeNext());

        // These are state variables that keep the
        // current position & style of the shape
        // outline, and vary as we read the GFxEdge data.
        //
        // At the moment we just store each GFxEdge with
        // the full necessary info to render it, which
        // is simple but not optimally efficient.

        // SHAPERECORDS
        do // while(0)
        {
            bool typeFlag = Stream.ReadUInt1(); // skip typeFlag
            SF_ASSERT(!typeFlag);
            SF_UNUSED(typeFlag);

            // Parse the record.
            int flags = Stream.ReadUInt5();
            if (flags == 0) 
            {
                // End of shape records.
                retVal = Shape_EndShape;
                LOG_SHAPE0("  SHAPE decode: end-of-shape\n\n");
                break;
            }
            SF_ASSERT((flags & 0x1F) != 0);
            if (flags & 0x01)
            {
                // MoveTo = 1;
                unsigned numMoveBits = Stream.ReadUInt5();
                Pos->LastX = Stream.ReadSInt(numMoveBits);
                Pos->LastY = Stream.ReadSInt(numMoveBits);

                LOG_SHAPE3("  SHAPE decode: numbits = %d, moveto %d %d\n", 
                    numMoveBits, Pos->LastX, Pos->LastY);
            }
            if ((flags & 0x02) && Pos->NumFillBits > 0)
            {
                // FillStyle0_change = 1;
                unsigned style = Stream.ReadUInt(Pos->NumFillBits);
                if (style > 0)
                {
                    style += Pos->FillBase;
                }
                Pos->Fill0 = style;

                LOG_SHAPE1("  SHAPE decode: fill0 = %d\n", Pos->Fill0);
            }
            if ((flags & 0x04) && Pos->NumFillBits > 0)
            {
                // FillStyle1_change = 1;
                int style = Stream.ReadUInt(Pos->NumFillBits);
                if (style > 0)
                {
                    style += Pos->FillBase;
                }
                Pos->Fill1 = style;

                LOG_SHAPE1("  SHAPE decode: fill1 = %d\n", Pos->Fill1);
            }
            if ((flags & 0x08) && Pos->NumStrokeBits > 0)
            {
                // StrokeStyleChange = 1;
                int style = Stream.ReadUInt(Pos->NumStrokeBits);
                if (style > 0)
                {
                    style += Pos->StrokeBase;
                }
                Pos->Stroke = style;

                LOG_SHAPE1("  SHAPE decode: line = %d\n", Pos->Stroke);
            }
            if (flags & 0x10)
            {
                retVal = Shape_NewLayer;

                Pos->Fill0 = Pos->Fill1 = Pos->Stroke = 0;

                Stream.Align();

                unsigned fsn = Stream.ReadU8();
                if (fsn != 0)
                {
                    fsn =  unsigned(Stream.ReadU8()) << 16;
                    fsn |= unsigned(Stream.ReadU8()) << 8;
                    fsn |= unsigned(Stream.ReadU8());
                }
                unsigned lsn = Stream.ReadU8();
                if (lsn == 0xFF) // ? PathFlags & Flags_HasExtendedFillNum ?
                {
                    lsn =  unsigned(Stream.ReadU8()) << 16;
                    lsn |= unsigned(Stream.ReadU8()) << 8;
                    lsn |= unsigned(Stream.ReadU8());
                }
                Pos->FillBase = fsn;
                Pos->StrokeBase = lsn;

                Pos->NumFillBits = Stream.ReadUInt4();
                Pos->NumStrokeBits = Stream.ReadUInt4();

                LOG_SHAPE2("  SHAPE decode: more fill styles, numfillbits = %d, numlinebint = %d\n", 
                    Pos->NumFillBits, Pos->NumStrokeBits);
            }
            break;
        } while(0);   
        return retVal;
    }
};

// A reader for SWF shape into ShapeDataBase
class ShapeSwfReader
{
    ShapeDataBase&       Shape;
    PathAllocator*       pAllocator;
    FillStyleArrayTemp   FillStyles;
    StrokeStyleArrayTemp StrokeStyles;

private:
    int ReadFillStyles(LoadProcess* p, TagType tagType);
    int ReadStrokeStyles(LoadProcess* p, TagType tagType);
public:
    ShapeSwfReader(ShapeDataBase& shape, PathAllocator* pallocator) 
        : Shape(shape), pAllocator(pallocator) {}

    bool Read(LoadProcess* p, TagType tagType, unsigned lenInBytes, bool withStyle);
    unsigned GetFillStyleCount() const   { return (unsigned)FillStyles.GetSize(); }
    unsigned GetStrokeStyleCount() const { return (unsigned)StrokeStyles.GetSize(); }

    const FillStyleType*   GetFillStyle(UPInt i) const   { return &FillStyles[i];   }
    const StrokeStyleType* GetStrokeStyle(UPInt i) const { return &StrokeStyles[i]; }

private:
    ShapeSwfReader& operator=(const ShapeSwfReader&) { return *this; }
};

int    ShapeSwfReader::ReadFillStyles(LoadProcess* p, TagType tagType)
{
    // Get the count.
    unsigned fillStyleCount = p->ReadU8();
    if (tagType > Tag_DefineShape)
    {
        if (fillStyleCount == 0xFF)
            fillStyleCount = p->ReadU16();
    }
    int off = p->Tell();

    p->LogParse("  GFx_ReadFillStyles: count = %d\n", fillStyleCount);

    // Read the styles.
    unsigned baseIndex = (unsigned)FillStyles.GetSize();
    if (fillStyleCount)
        FillStyles.Resize(baseIndex + fillStyleCount);
    for (unsigned i = 0; i < fillStyleCount; i++)
    {
        p->AlignStream(); //!AB
        FillStyleSwfReader fr(FillStyles[baseIndex + i]);
        fr.Read(p, tagType);
        if (fr.HasTextureFill())
            Shape.Flags |= ShapeDataBase::Flags_TexturedFill;
        if (fr.NeedsResolving())
            Shape.Flags |= ShapeDataBase::Flags_NeedsResolving;
    }
    return off;
}

int    ShapeSwfReader::ReadStrokeStyles(LoadProcess* p, TagType tagType)
{
    // Get the count.
    unsigned lineStyleCount = p->ReadU8();

    p->LogParse("  GFx_ReadStrokeStyles: count = %d\n", lineStyleCount);

    // @@ does the 0xFF flag apply to all tag types?
    // if (TagType > Tag_DefineShape)
    // {
    if (lineStyleCount == 0xFF)
    {
        lineStyleCount = p->ReadU16();
        p->LogParse("  GFx_ReadStrokeStyles: count2 = %d\n", lineStyleCount);
    }
    // }
    int off = p->Tell();

    // Read the styles.
    unsigned baseIndex = (unsigned)StrokeStyles.GetSize();
    StrokeStyles.Resize(baseIndex + lineStyleCount);

    for (unsigned i = 0; i < lineStyleCount; i++)
    {
        p->AlignStream(); //!AB
        StrokeStyleSwfReader sr(StrokeStyles[baseIndex + i]);
        sr.Read(p, tagType);
        if (sr.HasTextureFill())
            Shape.Flags |= ShapeDataBase::Flags_TexturedFill;
        if (sr.NeedsResolving())
            Shape.Flags |= ShapeDataBase::Flags_NeedsResolving;
    }
    return off;
}

bool    ShapeSwfReader::Read(LoadProcess* p, TagType tagType, unsigned lenInBytes, bool withStyle)
{
    PathAllocator* ppathAllocator = pAllocator? pAllocator : p->GetPathAllocator();
    Stream*  in = p->GetStream();
    SF_ASSERT(ppathAllocator);

    Shape.Flags = 0;

    int stylesLen;
    if (withStyle)
    {
        int curOffset = in->Tell();

        Shape.SetValidBoundsFlag(true);
        RectF bounds;
        in->ReadRect(&bounds); 
        Shape.SetBoundsLocal(bounds);

        // SWF 8 contains bounds without a stroke: MovieClip.getRect()
        if ((tagType == Tag_DefineShape4) || (tagType == Tag_DefineFont3))
        {
            RectF rectBound;
            in->ReadRect(&rectBound);
            Shape.SetRectBoundsLocal(rectBound);
            // MA: What does this byte do?
            // I've seen it take on values of 0x01 and 0x00
            //UByte mysteryByte =
            in->ReadU8();
        }
        else
            Shape.SetRectBoundsLocal(bounds);

        ReadFillStyles(p, tagType);
        ReadStrokeStyles(p, tagType);
        stylesLen = in->Tell() - curOffset;
        SF_ASSERT((int)stylesLen >= 0);
        SF_ASSERT((int)(lenInBytes - stylesLen) >= 0);
    }
    else
        stylesLen = 0;

    // Format of shape data is as follows:
    // 1 byte - PathFlags (see SwfPathData::Flags_<>)
    // 4 bytes - signature (in DEBUG build only)
    // N bytes [1..4] - size of whole shape data memory block (N depends on Mask_NumBytesInMemCnt)
    // X bytes - original SWF shape data
    // Y bytes [1..4] - number of shapes in swf shape data (Y depends on Mask_NumBytesInGeomCnt)
    // Z bytes [1..4] - number of paths in swf shape data (Z depends on Mask_NumBytesInGeomCnt)

    unsigned shapeLen = (unsigned)lenInBytes - stylesLen;
    unsigned memBlockSize = shapeLen;
    unsigned originalShapeLen = shapeLen;

#ifdef SF_BUILD_DEBUG
    // put a 4-bytes signature at the beginning for debugging purposes
    // to catch improper iterator usage.
    memBlockSize += 4;
#endif

    UByte *pmemBlock = ppathAllocator->AllocRawPath(memBlockSize);
    if (!pmemBlock)
    {
        // corrupted shape; just make an empty shape and display warning message.
        in->LogWarning("Corrupted shape detected in file %s", 
            in->GetFileName().ToCStr());
        //Shape.Paths = 
        return false;
    }
    UByte *ptr = pmemBlock;

#ifdef SF_BUILD_DEBUG
    // put a 4-bytes signature at the beginning for debugging purposes
    // to catch improper iterator usage. Put it in little endian format
    // like all swf's integers.
    *ptr++ = SwfShapeDecoder::Signature & 0xFF;
    *ptr++ = (SwfShapeDecoder::Signature >> 8) & 0xFF;
    *ptr++ = (SwfShapeDecoder::Signature >> 16) & 0xFF;
    *ptr++ = (SwfShapeDecoder::Signature >> 24) & 0xFF;
#endif

    in->Align();
    in->ReadToBuffer(ptr, shapeLen);

    // Multiplier used for scaling.
    // In SWF 8, font shape resolution (tag 75) was increased by 20.
    float sfactor = 1.0f;
    if (tagType == Tag_DefineFont3)
    {
        sfactor = 1.0f / 20.0f;
        Shape.Flags |= ShapeDataBase::Flags_Sfactor20;
    }

    Stream* poriginalStream = in;
    Stream ss(ptr, shapeLen, p->GetLoadHeap(), in->GetLog(), in->GetParseControl());
    in = &ss;
    p->SetAltStream(in);

    // need to preprocess shape to parse and remove all additional fill & line styles.
    //
    // SHAPE
    //
    in->Align(); //!AB
    int numFillBits = in->ReadUInt(4);
    int numLineBits = in->ReadUInt(4);

    if (withStyle) // do not trace, if !withStyle (for example, for font glyphs)
        in->LogParse("  ShapeCharacter read: nfillbits = %d, nlinebits = %d\n", numFillBits, numLineBits);

    // These are state variables that keep the
    // current position & style of the shape
    // outline, and vary as we read the GFxEdge data.
    //
    // At the moment we just store each GFxEdge with
    // the full necessary info to render it, which
    // is simple but not optimally efficient.
    int fillBase = 0;
    int lineBase = 0;
    int moveX = 0, moveY = 0;
    int numMoveBits = 0;
    unsigned totalPathsCount = 0;
    unsigned totalShapesCount = 0;
    int curEdgesCount = 0;
    int curPathsCount = 0;
    bool shapeIsCorrupted = false;

#define SHAPE_LOG 0
#ifdef GFX_SHAPE_MEM_TRACKING
    int __startingFileOffset = in->Tell();
#endif //GFX_SHAPE_MEM_TRACKING
    // SHAPERECORDS
    while(!shapeIsCorrupted) 
    {
        int typeFlag = in->ReadUInt1();
        if (typeFlag == 0)
        {
            // Parse the record.
            int flags = in->ReadUInt(5);
            if (flags == 0) 
            {
                // End of shape records.
                if (curEdgesCount > 0)
                {
                    ++totalPathsCount;
                    curEdgesCount = 0;
                    ++curPathsCount;
                }
                break;
            }
            if (flags & 0x01)
            {
                // MoveTo = 1;
                if (curEdgesCount > 0)
                {
                    ++totalPathsCount;
                    curEdgesCount = 0;
                    ++curPathsCount;
                }

                numMoveBits = in->ReadUInt(5);
                moveX = in->ReadSInt(numMoveBits);
                moveY = in->ReadSInt(numMoveBits);

                if (in->IsVerboseParseShape())
                    in->LogParseShape("  ShapeCharacter read: moveto %4g %4g\n", (float) moveX * sfactor, (float) moveY * sfactor);
            }
            if ((flags & 0x02) && numFillBits > 0)
            {
                // FillStyle0_change = 1;
                if (curEdgesCount > 0)
                {
                    ++totalPathsCount;
                    curEdgesCount = 0;
                    ++curPathsCount;
                }

                int style = in->ReadUInt(numFillBits);
                if (in->IsVerboseParseShape())
                {
                    if (style > 0)
                    {
                        style += fillBase;
                    }
                    in->LogParseShape("  ShapeCharacter read: fill0 = %d\n", style);
                }
            }
            if ((flags & 0x04) && numFillBits > 0)
            {
                // FillStyle1_change = 1;
                if (curEdgesCount > 0)
                {
                    ++totalPathsCount;
                    curEdgesCount = 0;
                    ++curPathsCount;
                }

                int style = in->ReadUInt(numFillBits);
                if (in->IsVerboseParseShape())
                {
                    if (style > 0)
                    {
                        style += fillBase;
                    }
                    in->LogParseShape("  ShapeCharacter read: fill1 = %d\n", style);
                }
            }
            if ((flags & 0x08) && numLineBits > 0)
            {
                // StrokeStyleChange = 1;
                if (curEdgesCount > 0)
                {
                    ++totalPathsCount;
                    curEdgesCount = 0;
                    ++curPathsCount;
                }

                int style = in->ReadUInt(numLineBits);
                if (style > 0)
                {
                    style += lineBase;
                }

                if (in->IsVerboseParseShape())
                    in->LogParseShape("  ShapeCharacter read: line = %d\n", style);
            }
            if (flags & 0x10)
            {
                SF_ASSERT(tagType >= 22);

                in->LogParse("  ShapeCharacter read: more fill styles\n");

                if (curPathsCount > 0)
                {
                    ++totalShapesCount;
#ifdef GFX_SHAPE_MEM_TRACKING
                    printf("shr: curnp = %d, offset = %d\n", curPathsCount, (in->Tell() - __startingFileOffset));
#endif
                    curPathsCount = 0;
                }
                if (curEdgesCount > 0)
                {
                    ++totalPathsCount;
                    curEdgesCount = 0;
                    ++curPathsCount;
                }

#ifdef GFX_SHAPE_MEM_TRACKING
                int __offset = in->Tell();
#endif
                fillBase = (int)FillStyles.GetSize();
                lineBase = (int)StrokeStyles.GetSize();

                int fsoff = in->Tell();
                int foff = ReadFillStyles(p, tagType);
                int moff = in->Tell();
                int loff = ReadStrokeStyles(p, tagType);
                int eoff = in->Tell();

                if (moff != foff)
                {
                    if (foff > moff || unsigned(moff) > originalShapeLen)
                    {
                        shapeIsCorrupted = true;
                        break;
                    }
                    else
                    {
                        // check if we have additional fill styles
                        if ((int)FillStyles.GetSize() > fillBase)
                        {
                            // record fillBase at foff
                            // first byte should be non-zero
                            *(ptr + fsoff) = 0xFF;
                            // next three bytes represent new index in fillStyles array
                            *(ptr + fsoff + 1) = UByte((fillBase >> 16) & 0xFF);
                            *(ptr + fsoff + 2) = UByte((fillBase >> 8) & 0xFF);
                            *(ptr + fsoff + 3) = UByte((fillBase) & 0xFF);

                            // shift foff, so we will not erase the fillBase
                            foff += 4 - (foff - fsoff);
                        }

                        // collapse array of fill styles and correct all offsets
                        // leave 2 bytes to store fillstyles offset in FillStyles array
                        memmove(ptr + foff, ptr + moff, loff - moff);
                        loff -= moff - foff;
                        moff = foff;
                    }
                }
                if (loff != eoff)
                {
                    if (loff > eoff || unsigned(eoff) > originalShapeLen)
                    {
                        shapeIsCorrupted = true;
                        break;
                    }
                    else
                    {
                        // check if we have additional fill styles
                        if ((int)StrokeStyles.GetSize() > lineBase)
                        {
                            // record fillBase at foff
                            // first byte should be non-zero
                            *(ptr + moff) = 0xFF;
                            // next three bytes represent new index in fillStyles array
                            *(ptr + moff + 1) = UByte((lineBase >> 16) & 0xFF);
                            *(ptr + moff + 2) = UByte((lineBase >> 8) & 0xFF);
                            *(ptr + moff + 3) = UByte((lineBase) & 0xFF);

                            // shift foff, so we will not erase the fillBase
                            loff += 4 - (loff - moff);
                        }

                        // collapse array of lines
                        memmove(ptr + loff, ptr + eoff, shapeLen - eoff);
                        shapeLen -= eoff - loff;
                    }
                }
                in->SetPosition(loff); // do we need to set new DataSize in the stream? (!AB)

                numFillBits = in->ReadUInt(4);
                numLineBits = in->ReadUInt(4);
#ifdef GFX_SHAPE_MEM_TRACKING
                __startingFileOffset -= (in->Tell() - __offset);
#endif
            }
        }
        else
        {
            // EDGERECORD
            int edgeFlag = in->ReadUInt1();
            ++curEdgesCount;
            if (edgeFlag == 0)
            {
                // curved GFxEdge
                unsigned numbits = 2 + in->ReadUInt(4);
                int cx = in->ReadSInt(numbits);
                int cy = in->ReadSInt(numbits);
                int ax = in->ReadSInt(numbits);
                int ay = in->ReadSInt(numbits);

                if (in->IsVerboseParseShape())
                {
                    in->LogParseShape("  ShapeCharacter read: curved edge   = %4g %4g - %4g %4g - %4g %4g\n", 
                        (float) moveX * sfactor, (float) moveY * sfactor, 
                        (float) (moveX + cx) * sfactor, (float) (moveY + cy) * sfactor, 
                        (float) (moveX + cx + ax ) * sfactor, (float) (moveY + cy + ay ) * sfactor);
                }
                moveX += ax + cx;
                moveY += ay + cy;
            }
            else
            {
                // straight GFxEdge
                unsigned numbits = 2 + in->ReadUInt(4);
                int lineFlag = in->ReadUInt1();
                int dx = 0, dy = 0;
                if (lineFlag)
                {
                    // General line.
                    dx = in->ReadSInt(numbits);
                    dy = in->ReadSInt(numbits);
                }
                else
                {
                    int vertFlag = in->ReadUInt1();
                    if (vertFlag == 0) {
                        // Horizontal line.
                        dx = in->ReadSInt(numbits);
                    } else {
                        // Vertical line.
                        dy = in->ReadSInt(numbits);
                    }
                }

                if (in->IsVerboseParseShape())
                {
                    in->LogParseShape("  ShapeCharacter read: straight edge = %4g %4g - %4g %4g\n", 
                        (float)moveX * sfactor, (float)moveY * sfactor, 
                        (float)(moveX + dx) * sfactor, (float)(moveY + dy) * sfactor);
                }
                moveX += dx;
                moveY += dy;
            }
        }
        // detect if shape record is corrupted
        shapeIsCorrupted = ((unsigned)in->Tell() > originalShapeLen);
    }
    // make correction for new shape length (if built-in fill & line styles
    // were removed).
    unsigned newMemBlockSize = memBlockSize - (originalShapeLen - shapeLen);
    if (!shapeIsCorrupted && newMemBlockSize < (1L << 21))
    {
        SF_ASSERT(newMemBlockSize < (1L << 21)); // only 2Mb shape data is allowed

        if (memBlockSize > newMemBlockSize)
        {
            ppathAllocator->ReallocLastBlock(pmemBlock, memBlockSize, newMemBlockSize); 
        }
#ifdef SF_BUILD_DEBUG
        // erase tail of reallocated memory block to make sure data is not used
        if (memBlockSize > newMemBlockSize)
            memset(pmemBlock + newMemBlockSize, 0xFB, memBlockSize - newMemBlockSize);
#endif
    }
    else
    {
        shapeIsCorrupted = true;
        // corrupted shape; just make an empty shape and display warning message.
        in->LogWarning("Corrupted shape detected in file %s", 
                       poriginalStream->GetFileName().ToCStr());
        unsigned offset = 0;

        // pathFlags, memsz, numbits fill and line (1 b), end-of-shape record (1 b), shapes cnt, paths cnt
        unsigned innerNewMemBlockSize = 1 + 1 ; 
#ifdef SF_BUILD_DEBUG
        innerNewMemBlockSize += 4; // signature
        offset += 4;
#endif
        pmemBlock[offset++] = 0; // num bits in fill & line styles
        pmemBlock[offset++] = 0; // end-of-shape record
        if (memBlockSize > innerNewMemBlockSize)
        {
            ppathAllocator->ReallocLastBlock(pmemBlock, memBlockSize, innerNewMemBlockSize); 
        }

    }
    Shape.Paths = pmemBlock;
#ifdef SF_BUILD_DEBUG
    {
        // Make sure signature is still correct
        // Read a 4-bytes signature to ensure we use correct path format.
        UInt32 v = (UInt32(pmemBlock[0])      | (UInt32(pmemBlock[1])<<8) |
            (UInt32(pmemBlock[2])<<16) | (UInt32(pmemBlock[3])<<24));
        SF_UNUSED(v);
        SF_ASSERT(v == SwfShapeDecoder::Signature);
    }
#endif

    p->SetAltStream(NULL);

#ifdef GFX_SHAPE_MEM_TRACKING
    GFx_MT_SwfGeomMem += (in->Tell() - __startingFileOffset);
    printf("shr: curnp = %d, offset = %d\n", curPathsCount, (in->Tell() - __startingFileOffset));
    printf("shr: np = %d nsh = %d, finaloff = %d\n", totalPathsCount, totalShapesCount+1, (in->Tell() - __startingFileOffset));
//     if(withStyle)
//     {
//         printf("Shapes memory statistics:\n"
//             "   ShapesCount = %d, PathsCount = %d, SwfGeomMem = %d, AllocatedMem = %d\n", 
//             GFx_MT_ShapesCount, GFx_MT_PathsCount, GFx_MT_SwfGeomMem, GFx_MT_GeomMem);
//     }
#endif
    return !shapeIsCorrupted;
}

//////////////////////////////////////////////////////////////////////////
ShapeDataBase::ShapeDataBase(EmptyShape) : Flags(0)
{
    static const UByte emptyShp[] = 
    {
#ifdef SF_BUILD_DEBUG
        // put a 4-bytes signature at the beginning for debugging purposes
        // to catch improper iterator usage. Put it in little endian format
        // like all swf's integers.
        UByte(SwfShapeDecoder::Signature & 0xFF),
        UByte((SwfShapeDecoder::Signature >> 8) & 0xFF),
        UByte((SwfShapeDecoder::Signature >> 16) & 0xFF),
        UByte((SwfShapeDecoder::Signature >> 24) & 0xFF),
#endif
        0, 0, 0, 0, 0, 0
    };
    Paths = emptyShp;
}

void    ShapeDataBase::ComputeBound(RectF* r) const
{
    *r = Render::ComputeBoundsFill(*this, Matrix2F());
}

bool    ShapeDataBase::DefPointTestLocal(Render::ShapeMeshProvider* pshapeMeshProvider, const Render::PointF &pt, bool testShape, 
                                         const DisplayObjectBase *pinst) const
{
    SF_UNUSED3(pt, testShape, pinst);
    SF_ASSERT(pshapeMeshProvider);

    Ptr<Render::Scale9GridInfo> s9g;

#if defined (GFX_ENABLE_SCALE9_HITTEST)
    if (pinst && pinst->DoesScale9GridExist())
    {
        s9g = *pinst->CreateScale9Grid();
    }
#endif //GFX_ENABLE_SCALE9_HITTEST

    RectF b = pshapeMeshProvider->GetIdentityBounds();

    if (s9g.GetPtr())
        b = s9g->AdjustBounds(b);

    if (b.Contains(pt))
    {
        if (testShape)
        {
//// DBG
//bool ret = Render::HitTestFill<Matrix2F>(*this, Matrix2F::Identity, pt.x, pt.y);
//printf("%d", ret);
            TransformerBase* tr = 0;
            TransformerWrapper<Matrix2F> trAffine;
            TransformerWrapper<Scale9GridInfo> trScale9;

            if (s9g)
            {
                trScale9 = TransformerWrapper<Scale9GridInfo>(s9g);
                tr = &trScale9;
            }
            else
            {
                trAffine = TransformerWrapper<Matrix2F>(&Matrix2F::Identity);
                tr = &trAffine;
            }
            return Render::HitTestFill<TransformerBase>(*this, *tr, pt.x, pt.y);
        }
        else
            return true;
    }
    return false;
}

UInt32  ShapeDataBase::ComputeGeometryHash() const
{
    SF_ASSERT(0); //@TODO
    return 0;
}
bool    ShapeDataBase::IsEqualGeometry(const ShapeBaseCharacterDef& cmpWith) const
{
    SF_UNUSED1(cmpWith);
    SF_ASSERT(0); //@TODO
    return 0;
}

ShapePathType ShapeDataBase::ReadPathInfo(ShapePosInfo* pos, float* coord, unsigned* styles) const
{
    SwfShapeDecoder decoder(pos, Paths, (Flags & Flags_Sfactor20) ? 1.0f/20.f : 1.0f);

    ShapePathType retVal = Shape_NewPath;
    while (retVal != Shape_EndShape && !decoder.IsEdgeNext())
    {
        retVal = decoder.ReadNonEdgeRec(retVal);
    }
    if (retVal != Shape_EndShape)
    {
        styles[0] = pos->Fill0;
        styles[1] = pos->Fill1;
        styles[2] = pos->Stroke;

        coord[0] = float(pos->LastX * pos->Sfactor);
        coord[1] = float(pos->LastY * pos->Sfactor);
    }
    return retVal;
}

// Read next edge. Returns Edge_LineTo, Edge_QuadTo, or Edge_EndPath. 
// Coord must be float[Seg_MaxCoord].
PathEdgeType ShapeDataBase::ReadEdge(ShapePosInfo* pos, float* coord) const
{
    SwfShapeDecoder decoder(pos, Paths, (Flags & Flags_Sfactor20) ? 1.0f/20.f : 1.0f);

    PathEdgeType retVal = Edge_EndPath;
    if (decoder.IsEdgeNext())
    {
        SwfShapeDecoder::Edge edge;
        retVal = decoder.ReadEdge(&edge);
        if (retVal == Edge_QuadTo)
        {
            coord[0] = float(edge.Cx * pos->Sfactor);
            coord[1] = float(edge.Cy * pos->Sfactor);
            coord[2] = float(edge.Ax * pos->Sfactor);
            coord[3] = float(edge.Ay * pos->Sfactor);
        }
        else
        {
            coord[0] = float(edge.Ax * pos->Sfactor);
            coord[1] = float(edge.Ay * pos->Sfactor);
        }
    }
    return retVal;
}

//////////////////////////////////////////////////////////////////////////
bool    ConstShapeNoStyles::Read(LoadProcess* p, TagType tagType, unsigned lenInBytes, bool withStyle)
{
    ShapeSwfReader reader(*this, p->GetPathAllocator());
    return reader.Read(p, tagType, lenInBytes, withStyle);
}

ShapeDataBase* ConstShapeNoStyles::Clone() const
{
    return SF_HEAP_AUTO_NEW(this) ConstShapeNoStyles(*this);
}

//////////////////////////////////////////////////////////////////////////
ConstShapeWithStyles::ConstShapeWithStyles(const ConstShapeWithStyles& o) :
    ShapeDataBase(o), Styles(NULL)
{
    const FillStyleType*   fs = reinterpret_cast<const FillStyleType*>(o.Styles);
    const StrokeStyleType* ss = reinterpret_cast<const StrokeStyleType*>(fs + o.FillStylesNum);
    SetStyles(o.FillStylesNum, fs, o.StrokeStylesNum, ss);
    Bound = RectBound = RectF(0,0,0,0);
}

ConstShapeWithStyles::~ConstShapeWithStyles()
{
    // need to release all styles stored in Styles!
    // release fill styles first
    FillStyleType* fs = reinterpret_cast<FillStyleType*>(Styles);
    for (unsigned i = 0; i < FillStylesNum; ++i, ++fs)
        fs->~FillStyleType();
    // release stroke styles
    StrokeStyleType* ss = reinterpret_cast<StrokeStyleType*>(fs);
    for (unsigned i = 0; i < StrokeStylesNum; ++i, ++ss)
        ss->~StrokeStyleType();

    SF_FREE(Styles);
}

ShapeDataBase* ConstShapeWithStyles::Clone() const
{
    return SF_HEAP_AUTO_NEW(this) ConstShapeWithStyles(*this);
}

// idx is in range [1...GetFillStyleCount()] (inclusive)
void ConstShapeWithStyles::GetFillStyle(unsigned idx, FillStyleType* p) const
{  
    SF_ASSERT(idx <= FillStylesNum);
    *p = reinterpret_cast<const FillStyleType*>(Styles)[idx-1];
}

// idx is range [1...GetStrokeStyleCount()] (inclusive)
void ConstShapeWithStyles::GetStrokeStyle(unsigned idx, StrokeStyleType* p) const
{
    SF_ASSERT(idx <= StrokeStylesNum);
    *p = reinterpret_cast<const StrokeStyleType*>(reinterpret_cast<const FillStyleType*>(Styles)+FillStylesNum)[idx-1];
}

bool    ConstShapeWithStyles::Read(LoadProcess* p, TagType tagType, unsigned lenInBytes, bool withStyle)
{
    ShapeSwfReader reader(*this, p->GetPathAllocator());
    if (!reader.Read(p, tagType, lenInBytes, withStyle))
        return false;

    FillStylesNum   = reader.GetFillStyleCount();
    StrokeStylesNum = reader.GetStrokeStyleCount();
    MemoryHeap* pheap = p->GetLoadHeap();
    if (FillStylesNum || StrokeStylesNum)
    {
        Styles = (UByte*)SF_HEAP_ALLOC(pheap, sizeof(FillStyleType) * FillStylesNum  + 
            sizeof(StrokeStyleType) * StrokeStylesNum, StatMD_ShapeData_Mem);

        FillStyleType* destFillStyles = (FillStyleType*)Styles;
        for (unsigned i = 0; i < FillStylesNum; ++i)
        {
            Construct<FillStyleType>(destFillStyles, *reader.GetFillStyle(i));
            ++destFillStyles;
        }

        StrokeStyleType* destStrokeStyles = (StrokeStyleType*)destFillStyles;
        for (unsigned i = 0; i < StrokeStylesNum; ++i)
        {
            Construct<StrokeStyleType>(&destStrokeStyles[i], *reader.GetStrokeStyle(i));
        }
    }
    else
        Styles = NULL;
    return true;
}

void    ConstShapeWithStyles::SetStyles
    (unsigned fillStyleCount, const FillStyleType* fillStyles, unsigned strokeStyleCount, const StrokeStyleType* strokeStyles)
{
    if (Styles)
        SF_FREE(Styles);

    FillStylesNum = fillStyleCount;
    StrokeStylesNum = strokeStyleCount;
    if (FillStylesNum || StrokeStylesNum)
    {
        Styles = (UByte*)SF_HEAP_AUTO_ALLOC_ID(this, sizeof(FillStyleType) * FillStylesNum  + 
            sizeof(StrokeStyleType) * StrokeStylesNum, StatMD_ShapeData_Mem);

        FillStyleType* destFillStyles = (FillStyleType*)Styles;
        for (unsigned i = 0; i < FillStylesNum; ++i)
        {
            Construct<FillStyleType>(destFillStyles, fillStyles[i]);
            // make a clone of ComplexFill too!
            if (destFillStyles->pFill)
                destFillStyles->pFill = *SF_HEAP_AUTO_NEW(this) ComplexFill(*destFillStyles->pFill);
            ++destFillStyles;
        }

        StrokeStyleType* destStrokeStyles = (StrokeStyleType*)destFillStyles;
        for (UPInt i = 0; i < StrokeStylesNum; ++i)
        {
            Construct<StrokeStyleType>(&destStrokeStyles[i], strokeStyles[i]);
            // make a clone of ComplexFill too!
            if (destStrokeStyles->pFill)
                destStrokeStyles->pFill = *SF_HEAP_AUTO_NEW(this) ComplexFill(*destStrokeStyles->pFill);
        }
    }
    else
        Styles = NULL;
}

void    ConstShapeWithStyles::BindResourcesInStyles(const GFx::ResourceBinding& resourceBinding)
{
    FillStyleType* fillStyles = (FillStyleType*)Styles;
    ResourceBindData rdata;
    for (unsigned i = 0; i < FillStylesNum; ++i)
    {
        if (fillStyles[i].pFill && fillStyles[i].pFill->BindIndex != ~0u)
        {
            resourceBinding.GetResourceData(&rdata, fillStyles[i].pFill->BindIndex);
            if (rdata.pResource.GetPtr() && rdata.pResource->GetResourceType() == Resource::RT_Image)
            {
                ImageResource* imgRes       = static_cast<ImageResource*>(rdata.pResource.GetPtr());
                fillStyles[i].pFill->pImage = static_cast<Image*>(imgRes->GetImage());
                fillStyles[i].pFill->BindIndex = ~0u;
            }
            else // - type of resource is wrong, warning?
            {
                fillStyles[i].pFill = NULL;
                fillStyles[i].Color = 0xFFA7DD39; //???
            }
        }
    }

    StrokeStyleType* strokeStyles = (StrokeStyleType*)(fillStyles + FillStylesNum);
    for (UPInt i = 0; i < StrokeStylesNum; ++i)
    {
        if (strokeStyles[i].pFill && strokeStyles[i].pFill->BindIndex != ~0u)
        {
            resourceBinding.GetResourceData(&rdata, strokeStyles[i].pFill->BindIndex);
            if (rdata.pResource.GetPtr() && rdata.pResource->GetResourceType() == Resource::RT_Image)
            {
                ImageResource* imgRes           = static_cast<ImageResource*>(rdata.pResource.GetPtr());
                strokeStyles[i].pFill->pImage   = static_cast<Image*>(imgRes->GetImage());
                strokeStyles[i].pFill->BindIndex = ~0u;
            }
            else // - type of resource is wrong, warning?
            {
                strokeStyles[i].pFill = NULL;
                strokeStyles[i].Color = 0xFFA7DD39; //???
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
SwfShapeCharacterDef::SwfShapeCharacterDef(ShapeDataBase* shp) 
    : pShape(shp) 
{
    pShapeMeshProvider = *SF_HEAP_AUTO_NEW(this) ShapeMeshProvider(pShape);
}

RectF SwfShapeCharacterDef::GetBoundsLocal(float) const
{
    SF_ASSERT(pShapeMeshProvider); 
    RectF bnd = pShape->GetBoundsLocal();
    return (bnd.IsEmpty()) ? pShapeMeshProvider->GetIdentityBounds() : bnd;
}

RectF SwfShapeCharacterDef::GetRectBoundsLocal(float mr) const 
{
    RectF rbnd = pShape->GetRectBoundsLocal();
    return (rbnd.IsEmpty()) ? GetBoundsLocal(mr) : rbnd;
} 

Ptr<Render::ShapeMeshProvider> SwfShapeCharacterDef::BindResourcesInStyles(const GFx::ResourceBinding& resourceBinding) const
{
    Ptr<ShapeDataBase> newSh = *pShape->Clone();
    newSh->BindResourcesInStyles(resourceBinding);
    Ptr<Render::ShapeMeshProvider> shapeMeshProvider = *SF_HEAP_AUTO_NEW(this) ShapeMeshProvider(newSh);
    return shapeMeshProvider;
}

//////////////////////////////////////////////////////////////////////////
ImageShapeCharacterDef::ImageShapeCharacterDef(ImageResource* pimage, ImageCreator* imgCreator, bool bilinear)
{
    Ptr<Render::ShapeDataFloat> shapeData = *SF_HEAP_AUTO_NEW(this) Render::ShapeDataFloat();
    Render::FillStyleType fill;
    //fill.Color = 0xFF0394AA; //@DBG; comment all accesses to fill.pFill to enable solid color fill
    fill.pFill = *SF_HEAP_AUTO_NEW(this) Render::ComplexFill();
    if (pimage)
    {
        Ptr<Render::Image> img;
        if (pimage->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
            img = static_cast<Render::Image*>(pimage->GetImage());
        else
        {
            if (!imgCreator)
                LogDebugMessage(Log_Warning, "ImageCreator is null in ImageShapeCharacterDef ctor");
            else
            {
                ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, Memory::GetHeapByAddress(this));
                img = *imgCreator->CreateImage(cinfo, static_cast<Render::ImageSource*>(pimage->GetImage()));
            }
        }
        fill.pFill->pImage = img;
        if (!img)
        {
            LogDebugMessage(Log_Warning, "Image is not created.");
            return;
        }
        fill.pFill->ImageMatrix.SetIdentity();
        fill.pFill->ImageMatrix.AppendScaling(0.05f);
        fill.pFill->FillMode.Fill = Wrap_Clamp;
        if (bilinear)
            fill.pFill->FillMode.Fill |= Sample_Linear;
        else
            fill.pFill->FillMode.Fill |= Sample_Point;

        Render::ImageRect imgR = img->GetRect();
        Matrix2F m;
        img->GetMatrix(&m);
        RectF r;
        m.EncloseTransform(&r, RectF(float(imgR.x1), float(imgR.y1), float(imgR.x2), float(imgR.y2)));

        shapeData->AddFillStyle(fill);
        shapeData->StartPath(1, 0, 0);
        shapeData->MoveTo(0, 0);
        shapeData->LineTo(float(PixelsToTwips(r.Width())), 0);
        shapeData->LineTo(float(PixelsToTwips(r.Width())), float(PixelsToTwips(r.Height())));
        shapeData->LineTo(0, float(PixelsToTwips(r.Height())));
        shapeData->ClosePath();
        shapeData->EndPath();
        shapeData->EndShape();
        Ptr<ShapeMeshProvider> pr = *SF_HEAP_AUTO_NEW(this) Render::ShapeMeshProvider(shapeData);
        pShape = shapeData.GetPtr();
        pShapeMeshProvider = *SF_HEAP_AUTO_NEW(this) ShapeMeshProvider(pShape);
    }
    else
        LogDebugMessage(Log_Warning, "Image is null in ImageShapeCharacterDef ctor.");
}

bool ImageShapeCharacterDef::DefPointTestLocal(const Render::PointF &pt, bool testShape, 
                                               const DisplayObjectBase *pinst) const
{
    SF_UNUSED3(pt, testShape, pinst);
    SF_ASSERT(pShapeMeshProvider);
    RectF b = pShapeMeshProvider->GetIdentityBounds();
    if (b.Contains(pt))
    {
        if (testShape)
        {
            //// DBG
            //bool ret = Render::HitTestFill<Matrix2F>(*this, Matrix2F::Identity, pt.x, pt.y);
            //printf("%d", ret);
            SF_ASSERT(pShape);
            return Render::HitTestFill<Matrix2F>(*pShape.GetPtr(), Matrix2F::Identity, pt.x, pt.y);
        }
        else
            return true;
    }
    return false;
}

}} // SF::GFx
