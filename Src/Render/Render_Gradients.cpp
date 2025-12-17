/**************************************************************************

Filename    :   Render_Gradients.cpp
Content     :   Gradient Generator
Created     :   February 2010
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Gradients.h"
#include "Render_Primitive.h"
#include "Kernel/SF_Math.h"

namespace Scaleform { namespace Render {


//-----------------------------------------------------------------------
unsigned const GradientData::ImageSizeTable[] = 
{
    16, 32, 32, 64, 64, 64, 64, 128, 128, 128, 128, 128, 128, 256, 256, 256, 256, 256
};


//-----------------------------------------------------------------------
GradientData::GradientData(GradientType type, UInt16 recordCount, bool linearRgb) :
    LinearRGB(linearRgb),
    Type(UByte(type)),          // Use UByte instead of GradientType for
                                // potentially better data alignment
    RecordCount(0),
    pRecords(0),
    FocalRatio(0),
    pMorphTo(0)
{
    //SF_ASSERT((type < 256) && (type & Fill_Gradient_TestBit)); // TO DO: Restore
    SetRecordCount(recordCount);
}

//-----------------------------------------------------------------------
GradientData::~GradientData()
{
    if (pRecords)
        SF_FREE(pRecords);
}

//-----------------------------------------------------------------------
bool GradientData::SetRecordCount(UInt16 count, bool tmpHeap)
{
    if (count == RecordCount)
        return 1;

    GradientRecord* newRecs = tmpHeap ? (GradientRecord*)SF_ALLOC(sizeof(GradientRecord)*count, StatRender_TreeCache_Mem) :
                                        (GradientRecord*)SF_HEAP_AUTO_ALLOC(this, sizeof(GradientRecord)*count);
    if (newRecs == 0)
        return 0;

    if (pRecords)
    {
        unsigned copyCount = Alg::Min(count, RecordCount);
        for (unsigned i=0; i<copyCount; i++)
        {
            newRecs[i] = pRecords[i];
        }
        SF_FREE(pRecords);
    }

    pRecords    = newRecs;
    RecordCount = count;
    return 1;
}


//-----------------------------------------------------------------------
void GradientData::SetLerp(const GradientData* data1, const GradientData* data2, float morphRatio)
{
    SF_ASSERT(data1->GetGradientType() == data2->GetGradientType());
    SF_ASSERT(data1->GetRecordCount() == data2->GetRecordCount());

    Type = (UByte)data1->GetGradientType();
    SetRecordCount(data1->GetRecordCount(), true);
    for(UInt16 i = 0; i < RecordCount; ++i)
        pRecords[i] = data1->At(i).LerpTo(data2->At(i), morphRatio);

    FocalRatio = Alg::Lerp(data1->GetFocalRatio(), data2->GetFocalRatio(), morphRatio);
}

//-----------------------------------------------------------------------
UPInt GradientData::GetHashValue(float morphRatio) const
{
    unsigned    i;
    UPInt   hashCode = Type;
    for (i = 0; i< RecordCount; i++)
    {
        hashCode ^= pRecords[i].GetHashValue();
    }
    if (pMorphTo)
    {
        UInt32 t;
        memcpy(&t, &morphRatio, 4);
        hashCode ^= (t & 0xFFFF) ^ (t >> 16);
    }
    return hashCode;
}

//-----------------------------------------------------------------------
bool GradientData::operator == (const GradientData& other) const
{
    if ((RecordCount != other.RecordCount) ||
        (Type != other.Type) ||
        (FocalRatio != other.FocalRatio) ||
        (LinearRGB != other.LinearRGB))
    {
        return 0;
    }
    
    unsigned i;
    for (i=0; i< RecordCount; i++)
    {
        if (pRecords[i] != other.pRecords[i])
        {
            return 0;
        }
    }
    return 1;
}


// Adaptive gradient size. The function empirically computes the optimal gradient 
// size depending on the maximal slope in the color ramp, focal point, and linearRGB flag. 
// See also RadialGradientSizeTable that may be modified to improve the result. This 
// table and all other coefficients are defined empirically, on the basis of the visual 
// result in typical cases. It's a kind of magic...
//-----------------------------------------------------------------------
unsigned GradientData::CalcImageSize() const
{
    unsigned i;
    float maxSlope = 0;
    for (i = 1; i < GetRecordCount(); ++i)
    {
        float slope;
        const GradientRecord& g1 = pRecords[i - 1];
        const GradientRecord& g2 = pRecords[i];
        float dx = (float)g2.Ratio - (float)g1.Ratio;
        if (dx > 0)
        {
            slope = abs((int)g1.ColorV.GetRed()   - (int)g2.ColorV.GetRed())   / dx;
            if (slope > maxSlope) maxSlope = slope;
            slope = abs((int)g1.ColorV.GetGreen() - (int)g2.ColorV.GetGreen()) / dx;
            if (slope > maxSlope) maxSlope = slope;
            slope = abs((int)g1.ColorV.GetBlue()  - (int)g2.ColorV.GetBlue())  / dx;
            if (slope > maxSlope) maxSlope = slope;
            slope = abs((int)g1.ColorV.GetAlpha() - (int)g2.ColorV.GetAlpha()) / dx;
            if (slope > maxSlope) maxSlope = slope;
        }
    }

    if (maxSlope == 0)
        return DefaultImageSize;

    if (LinearRGB) 
        maxSlope *= 1.5f;

    if (Type == GradientFocalPoint)
    {
        float r = fabsf(FocalRatio);
        if (r > 0.5f)
            maxSlope /= 1.01f - r;
    }

    if (maxSlope < 0)
        maxSlope = 0;

    maxSlope = sqrtf((maxSlope + 0.18f) * 5);
    unsigned idx = (unsigned)maxSlope;

    if (idx >= sizeof(ImageSizeTable) / sizeof(unsigned)) 
        idx  = sizeof(ImageSizeTable) / sizeof(unsigned) - 1;

    return ImageSizeTable[idx];
}





//---------------------------------------------------------------------
class GradientRamp
{
public:
    enum 
    { 
        RampSize = 256,
        R=0, G=1, B=2, A=3
    };

    GradientRamp() {}
    GradientRamp(const GradientRecord* records, unsigned recordCount, float gamma=1);

    void WriteRGBA(unsigned i, UByte* rgba) const 
    { 
        memcpy(rgba, Ramp + i*4, 4);
    }

    const UByte* GetRampBytes() const { return Ramp; }

private:
    struct ColorType
    {
        UInt16 r,g,b,a;
        ColorType() {}
        ColorType(UInt16 r_, UInt16 g_, UInt16 b_, UInt16 a_) :
            r(r_), g(g_), b(b_), a(a_) {}
    };

    static ColorType applyGammaDir(const Color& c, float gamma)
    {
        int a = c.GetAlpha();
        return ColorType(applyGammaDir(c.GetRed(),   gamma),
                         applyGammaDir(c.GetGreen(), gamma), 
                         applyGammaDir(c.GetBlue(),  gamma),
                         UInt16((a << 8) | a));
    }

    static UInt16 applyGammaDir(UByte v, float gamma)
    {
        return (UInt16)floor(pow(v / 255.0f, gamma) * 65535.0f + 0.5f);
    }

    static UByte applyGammaInv(UInt16 v, float gamma)
    {
        return (UByte)floor(pow(v / 65535.0f, gamma) * 255.0f + 0.5f);
    }

    static void blendColors(UByte* c, const ColorType& c1, const ColorType& c2, 
                            int ratio, int maxRatio, float gamma)
    {
        int alphaDiv = (maxRatio << 8) | maxRatio;
        c[R] = applyGammaInv(UInt16(c1.r + int(c2.r - c1.r) * ratio / maxRatio), gamma);
        c[G] = applyGammaInv(UInt16(c1.g + int(c2.g - c1.g) * ratio / maxRatio), gamma); 
        c[B] = applyGammaInv(UInt16(c1.b + int(c2.b - c1.b) * ratio / maxRatio), gamma);
        c[A] =                UByte(c1.a + int(c2.a - c1.a) * ratio / alphaDiv);
    }

    static void blendColors(UByte* c, const ColorType& c1, const ColorType& c2, int ratio, int maxRatio)
    {
        maxRatio = Alg::Max(1, (maxRatio << 8) | maxRatio);
        c[R] = UByte(c1.r + int(c2.r - c1.r) * ratio / maxRatio);
        c[G] = UByte(c1.g + int(c2.g - c1.g) * ratio / maxRatio);
        c[B] = UByte(c1.b + int(c2.b - c1.b) * ratio / maxRatio);
        c[A] = UByte(c1.a + int(c2.a - c1.a) * ratio / maxRatio);
    }

    UByte Ramp[RampSize*4];
};



GradientRamp::GradientRamp(const GradientRecord* records, unsigned recordCount, float gamma)
{
    unsigned i;
    if (recordCount == 0 || records == 0)
    {
        GradientRecord fakeColor;
        fakeColor.Ratio = 0;
        fakeColor.ColorV = 0xFF000000;
        recordCount  = 1;
        records = &fakeColor;
    }

    UByte c[4];
    c[R] = records[0].ColorV.GetRed();
    c[G] = records[0].ColorV.GetGreen();
    c[B] = records[0].ColorV.GetBlue();
    c[A] = records[0].ColorV.GetAlpha();

    if(recordCount > 1)
    {
        unsigned start = records[0].Ratio;
        unsigned end   = 0;
        float    gammaInv = 1/gamma;
        for(i = 0; i < start; i++) 
        {
            memcpy(Ramp + i*4, c, 4);
        }
        for(i = 1; i < recordCount; i++)
        {
            end = records[i].Ratio;
            if(end < start) end = start;

            ColorType c1 = applyGammaDir(records[i-1].ColorV, gamma);
            ColorType c2 = applyGammaDir(records[i].ColorV,   gamma);

            unsigned x0 = start;
            unsigned dx = end - start;
            if(gamma == 1)
            {
                while(start < end)
                {
                    blendColors(c, c1, c2, start-x0, dx-1);
                    memcpy(Ramp + start*4, c, 4); 
                    ++start;
                }
            }
            else
            {
                while(start < end)
                {
                    blendColors(c, c1, c2, start-x0, dx-1, gammaInv);
                    memcpy(Ramp + start*4, c, 4); 
                    ++start;
                }
            }
            c[R] = UByte(c2.r >> 8);
            c[G] = UByte(c2.g >> 8);
            c[B] = UByte(c2.b >> 8);
            c[A] = UByte(c2.a >> 8);
        }

        for(; end < RampSize; end++)
        {
            memcpy(Ramp + end*4, c, 4); 
        }
    }
    else
    {
        for(i = 0; i < RampSize; i++)
        {
            memcpy(Ramp + i*4, c, 4); 
        }
    }
}




//=========================================================================
// The code of these classes was taken from the Anti-Grain Geometry
// Project and modified for the use by Scaleform/Autodesk. 
// Permission to use without restrictions is hereby granted to 
// Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
// See http://antigrain.com for details.
//=========================================================================
class FocalRadialGradient
{
public:
    //---------------------------------------------------------------------
    void  Init(float r, float fx, float fy);
    float Calculate(float x, float y) const;

private:
    float Radius;
    float FocusX;
    float FocusY;
    float Radius2;
    float Multiplier;
};

//---------------------------------------------------------------------
void FocalRadialGradient::Init(float r, float fx, float fy)
{
    // Calculate the invariant values. In case the focal center
    // lies exactly on the gradient circle the divisor degenerates
    // into zero. In this case we just move the focal center by
    // one subpixel unit possibly in the direction to the origin (0,0)
    // and calculate the values again.
    //-------------------------
    Radius = r; 
    FocusX = fx;
    FocusY = fy;
    float fx2 = FocusX * FocusX;
    float fy2 = FocusY * FocusY;
    Radius2   = Radius * Radius;
    float d   = Radius2 - (fx2 + fy2);
    if(d == 0)
    {
        if(FocusX != 0) { if(FocusX < 0) FocusX += 1; else FocusX -= 1; }
        if(FocusY != 0) { if(FocusY < 0) FocusY += 1; else FocusY -= 1; }
        fx2 = FocusX * FocusX;
        fy2 = FocusY * FocusY;
        d   = Radius2 - (fx2 + fy2);
    }
    Multiplier = Radius / d;
}


//---------------------------------------------------------------------
float FocalRadialGradient::Calculate(float x, float y) const
{
    float dx = x - FocusX;
    float dy = y - FocusY;
    float d2 = dx * FocusY - dy * FocusX;
    float d3 = Radius2 * (dx * dx + dy * dy) - d2 * d2;
    return (dx * FocusX + dy * FocusY + sqrtf(fabsf(d3))) * Multiplier;
}






//-----------------------------------------------------------------------
GradientImage::GradientImage(PrimitiveFillManager* mng, GradientData* data, float morphRatio) :
    pManager(mng),
    pData(data),
    Size(1, 1),
    MorphRatio(morphRatio)
{
    if (data)
    {
        if (data->GetGradientType() == GradientLinear)
        {
            Size.Width  = MaxImageSize;
            Size.Height = 1;
        }
        else
        {
            Size.Width = Size.Height = data->CalcImageSize();
        }
    }
    SF_AMP_CODE(ImageId = ImageBase::GetNextImageId();)
}

//-----------------------------------------------------------------------
GradientImage::~GradientImage()
{
    if (pManager)
        pManager->removeGradient(this);
}
 
//-----------------------------------------------------------------------
bool GradientImage::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    ImagePlane  dplane;
    pdest->GetPlane(0, &dplane);
    UByte* dst = dplane.pData;
    UByte  src[4*MaxImageSize];

    // TO DO: Revise, maybe it's better just to return false or use an assertion.
    if (pData.GetPtr() == 0)
    {
        // Fake image, 1x1
        src[0] = src[1] = src[2] = src[3] = 0;
        copyScanline(dst, src, 4, 0, arg);
        return true;
    }

    const GradientData* data = pData;
    const GradientData* morph = pData->GetMorphTo();
    GradientData tmpData(GradientLinear);
    if (morph)
    {
        tmpData.SetLerp(data, morph, MorphRatio);
        data = &tmpData;
    }

    // Gamma=2.17 is the closest approximation of the sRGB curve.
    //--------------------
    GradientRamp ramp(data->GetRecords(), data->GetRecordCount(), 
                      data->IsLinearRGB() ? 2.17f : 1.0f);

    float center, radius, x, y;
    int   ratio;
    UByte last[4];
    UPInt toLast = dplane.Pitch * (Size.Height-1);

    unsigned i, j, size;
    if (data->GetGradientType() == GradientLinear)
    {
        SF_ASSERT(Size.Width == 256);
        for (i = 0; i < Size.Height; ++i)
        {
            copyScanline(dst, ramp.GetRampBytes(), 4 * Size.Width, 0, arg);
        }
    }
    else
    {
        ramp.WriteRGBA(255, last);
        for(i = 0; i < Size.Width; ++i)
        {
            memcpy(src + 4*i, last, 4);
        }

        copyScanline(dst,        src, 4 * Size.Width, 0, arg);
        copyScanline(dst+toLast, src, 4 * Size.Width, 0, arg);
        SF_ASSERT(Size.Width == Size.Height);
        center = Size.Width / 2.0f;
        radius = center - 1;
        size   = Size.Width - 1;
        memcpy(src,                last, 4);
        memcpy(src+Size.Width*4-4, last, 4);

        FocalRadialGradient gr;
        if (data->GetGradientType() == GradientFocalPoint)
        {
            gr.Init(radius, data->GetFocalRatio() * radius, 0);
        }

        dst += dplane.Pitch;
        for (j = 1; j < size; j++)
        {
            if (data->GetGradientType() == GradientRadial)
            {
                for (i = 1; i < size; i++)
                {                       
                    y = j - center + 0.5f;
                    x = i - center + 0.5f;
                    ratio = (int)floorf((float)sqrt(x * x + y * y) * 256 / radius + 0.5f);
                    if (ratio > 255) ratio = 255;
                    ramp.WriteRGBA(ratio, src + i*4);
                }
            }
            else
            {
                SF_ASSERT(data->GetGradientType() == GradientFocalPoint);
                for (i = 1; i < size; i++)
                {
                    y = j - center + 0.5f;
                    x = i - center + 0.5f;
                    ratio = (int)floorf(gr.Calculate(x, y) * 256 / radius + 0.5f);
                    if (ratio > 255) ratio = 255;
                    ramp.WriteRGBA(ratio, src + i*4);
                }
            }
            copyScanline(dst, src, 4 * Size.Width, 0, arg);
            dst += dplane.Pitch;
        }
    }
    return true;
}

//-----------------------------------------------------------------------
Texture* GradientImage::GetTexture(TextureManager* pmanager)
{
    if (pTexture && pTexture->GetTextureManager() == pmanager)
    {
        return pTexture;
    }
    if (!pmanager)
        return 0;

    pTexture = 0;
    Texture* ptexture = 
        pmanager->CreateTexture(GetFormat(), 
                                1,
                                Size,
                                ImageUse_Wrap, // TO DO: Revise for linear Gradients                                
                                this);
    initTexture_NoAddRef(ptexture);
    return ptexture;
}

// Initially the gradient is mapped to coordinates (0,0)-(1,1).
// The function maps the initial (0,0)-(1,1) square to line segment (x1,y1)-(x2,y2).
// Most suitable for linear gradients.
//-----------------------------------------------------------------------
Matrix2F LineToMatrix2D(float x1, float y1, float x2, float y2)
{
    Matrix2F m;
    float dx = (x2-x1)/2;
    float dy = (y2-y1)/2;
    float parl[6] = { x1-dy, y1+dx, x1+dy, y1-dx, x2+dy, y2-dx };
    m.SetParlToRect(parl, 0, 0, 1, 1);
    return m;
}

// Initially the gradient is mapped to coordinates (0,0)-(1,1).
// The function maps the initial (0,0)-(1,1) square to parallelogram 
// (x1,y1)-(x2,y2)-(x3,y3). Sutable for linear and radial gradients.
//-----------------------------------------------------------------------
Matrix2F ParlToMatrix2D(float x1, float y1, float x2, float y2, float x3, float y3)
{
    Matrix2F m;
    float parl[6] = { x1, y1, x2, y2, x3, y3 };
    m.SetParlToRect(parl, 0, 0, 1, 1);
    return m;
}


}} // Scaleform::Render
