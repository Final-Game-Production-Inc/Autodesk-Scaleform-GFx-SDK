/**************************************************************************

Filename    :   GFx_Styles.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


//#include "Render/Render_Renderer.h"
#include "Render/Render_Image.h"

#include "GFx/GFx_Styles.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_FontResource.h"

#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_PlayerImpl.h"


#include "GFx/GFx_ImageResource.h"

#include "GFx/GFx_LoadProcess.h"

//#include "GFx/GFx_DisplayContext.h"


#ifdef SF_ASSERT_ON_GRADIENT_BITMAP_GEN
#define SF_ASSERT_ON_GRADIENT_BITMAP_GEN SF_ASSERT(0)
#else
#define SF_ASSERT_ON_GRADIENT_BITMAP_GEN ((void)0)
#endif //SF_ASSERT_ON_GRADIENT_BITMAP_GEN


namespace Scaleform { namespace GFx {

//
// *** GradientRecord
//

GradientRecord::GradientRecord()
    : Ratio(0)
{
}

void    GradientRecord::Read(LoadProcess* p, TagType tagType)
{
    Ratio = p->ReadU8();
    p->ReadRgbaTag(&ColorV, tagType);
}


//
// *** GradientData
//

GradientData::GradientData(FillType type, UInt16 recordCount, bool linearRgb)
{
    SF_ASSERT((type < 256) && (type & Fill_Gradient_TestBit));
    Type        = (UByte) type;

    pRecords    = 0;
    RecordCount = 0;
    SetRecordCount(recordCount);

    LinearRGB = linearRgb;
    FocalRatio = 0.0f;
}

GradientData::~GradientData()
{
    if (pRecords)
        SF_FREE(pRecords);
}

bool    GradientData::SetRecordCount(UInt16 count)
{
    if (count == RecordCount)
        return 1;

    GradientRecord* pnewRecords = 0;    
    if (count)
        if ((pnewRecords = (GradientRecord*)SF_ALLOC(sizeof(GradientRecord)*count, StatMD_CharDefs_Mem)) == 0)
            return 0;

    if (pRecords)
    {
        unsigned copyCount = Alg::Min(count, RecordCount);
        for (unsigned i=0; i<copyCount; i++)
            pnewRecords[i] = pRecords[i];
        SF_FREE(pRecords);
    }

    pRecords    = pnewRecords;
    RecordCount = count;
    return 1;
}

UPInt     GradientData::GetHashCode() const
{
    unsigned    i;
    UPInt   hashCode = Type;
    for (i = 0; i< RecordCount; i++)
        hashCode ^= pRecords[i].GetHashCode();
    return hashCode;
}

bool    GradientData::operator == (const GradientData& other) const
{
    if ((RecordCount != other.RecordCount) ||
        (Type != other.Type) ||
        (FocalRatio != other.FocalRatio) ||
        (LinearRGB != other.LinearRGB))
        return 0;
    
    unsigned i;
    for (i=0; i< RecordCount; i++)    
        if (pRecords[i] != other.pRecords[i])
            return 0;    
    return 1;
}

// Creates an image resource from gradient.
ImageResource*   GradientData::CreateImageResource(
                        ResourceWeakLib *plib,
                        GradientParams *pparams,
                        ImageCreator *pimageCreator,
                        RenderConfig* prconfig,
                        Log* plog,
                        bool threadedLoading)
{
    // Gradient data must live in a global heap since it is used as a key in ResourceLib.
    SF_ASSERT(Memory::GetHeapByAddress(this) == Memory::GetGlobalHeap());

    // Create a key and check it against library.
    ResourceKey      gradientKey = CreateGradientKey();
    ImageResource *  pres        = 0;

    ResourceLib::BindHandle bh;
    if (plib->BindResourceKey(&bh, gradientKey) == ResourceLib::RS_NeedsResolve)
    {
        // If resource does not exist, create it and resolve it in library.
        Ptr<Render::Image> pimage = *CreateGradientImage(pparams, plib->GetImageHeap());
        if (pimage)
        {
            //@IMG
            SF_ASSERT(0);
//             ImageCreateInfo   ico(pimage, Resource::Use_Gradient);
//             ico.SetStates(0, prconfig, plog, 0, 0);
//             ico.ThreadedLoading = threadedLoading;
//             ico.pHeap           = plib->GetImageHeap();
//             Ptr<ImageInfoBase> pimageInfo;
//             
//             if (pimageCreator)
//                 pimageInfo = *pimageCreator->CreateImage(ico);
//             else
//             {
//                 SF_DEBUG_WARNING(1, "GradientData::CreateImageResource failed - ImageCreator not installed");
//             }
// 
//             // Add bitmap resource, if image was created; otherwise, it will
//             // be added by loader.
//             pres = SF_HEAP_NEW(ico.pHeap) ImageResource(pimageInfo, gradientKey,
//                                                          Resource::Use_Gradient);
//             if (pres)
//                 bh.ResolveResource(pres);
        }

        if (!pres)
            bh.CancelResolve("Failed to create gradient");
    }
    else
    {
        // WaitForResolve AddRefs, so we are ok to return result.
        pres = (ImageResource*) bh.WaitForResolve();
    }

    return pres;
}



class GFxGradientImageResourceKey : public ResourceKey::KeyInterface
{
public:
    typedef ResourceKey::KeyHandle KeyHandle;

    virtual void    AddRef(KeyHandle hdata)
    {
        SF_ASSERT(hdata); ((GradientData*) hdata)->AddRef();
    }
    virtual void    Release(KeyHandle hdata)
    {        
        SF_ASSERT(hdata); ((GradientData*) hdata)->Release();
    }

    // Key/Hash code implementation.
    virtual ResourceKey::KeyType GetKeyType(KeyHandle hdata) const
    {
        SF_UNUSED(hdata);
        return ResourceKey::Key_Gradient;
    }

    virtual UPInt   GetHashCode(KeyHandle hdata) const
    {
        SF_ASSERT(hdata);        
        return ((GradientData*) hdata)->GetHashCode();
    }

    virtual bool    KeyEquals(KeyHandle hdata, const ResourceKey& other)
    {
        if (this != other.GetKeyInterface())
            return 0;        
        return *((GradientData*) hdata) == *((GradientData*) other.GetKeyData());
    }
};


class GFxGradientImageResourceCreator : public ResourceData::DataInterface
{
    // Creates/Loads resource based on data and loading process
    virtual bool    CreateResource(ResourceData::DataHandle hdata, ResourceBindData *pbindData,
                                   LoadStates *pls, MemoryHeap*) const
    {
        GradientData *pgd = (GradientData*) hdata;
        SF_ASSERT(pgd);        
        pbindData->pResource = *pgd->CreateImageResource(pls->GetLib(),
                                                         pls->GetBindStates()->pGradientParams,
                                                         pls->GetBindStates()->pImageCreator,
                                                         pls->GetRenderConfig(), pls->GetLog(),
                                                         pls->IsThreadedLoading());
        return pbindData->pResource ? 1 : 0;
    }
};



static GFxGradientImageResourceCreator static_inst_data;
static GFxGradientImageResourceKey     static_inst_key;

// Creates a ResourceData object from this GradientData;
// this object can be used to create a resource.
ResourceData GradientData::CreateGradientResourceData()
{
    return ResourceData(&static_inst_data, this);
}

ResourceKey  GradientData::CreateGradientKey()
{
    return ResourceKey(&static_inst_key, this);
}



//=========================================================================
// The code of this class was taken from the Anti-Grain Geometry
// Project and modified for the use by Scaleform. 
// Permission to use without restrictions is hereby granted to 
// Scaleform Corporation by the author of Anti-Grain Geometry Project.
//=========================================================================
// This class is called G..., not GFx... because it's independent and
// actually belongs to the renderer. 
//=========================================================================
class GFocalRadialGradient
{
public:
    //---------------------------------------------------------------------
    GFocalRadialGradient() : 
        Radius(100),
        FocusX(0), 
        FocusY(0)
    {
        updateValues();
    }

    //---------------------------------------------------------------------
    GFocalRadialGradient(float r, float fx, float fy) : 
        Radius(r), 
        FocusX(fx), 
        FocusY(fy)
    {
        updateValues();
    }

    //---------------------------------------------------------------------
    void Init(float r, float fx, float fy)
    {
        Radius = r;
        FocusX = fx;
        FocusY = fy;
        updateValues();
    }

    float Calculate(float x, float y) const;

private:
    void updateValues();

    float Radius;
    float FocusX;
    float FocusY;
    float Radius2;
    float Multiplier;
};



//---------------------------------------------------------------------
float GFocalRadialGradient::Calculate(float x, float y) const
{
    float dx = x - FocusX;
    float dy = y - FocusY;
    float d2 = dx * FocusY - dy * FocusX;
    float d3 = Radius2 * (dx * dx + dy * dy) - d2 * d2;
    return (dx * FocusX + dy * FocusY + sqrtf(fabsf(d3))) * Multiplier;
}

//---------------------------------------------------------------------
void GFocalRadialGradient::updateValues()
{
    // Calculate the invariant values. In case the focal center
    // lies exactly on the gradient circle the divisor degenerates
    // into zero. In this case we just move the focal center by
    // one subpixel unit possibly in the direction to the origin (0,0)
    // and calculate the values again.
    //-------------------------
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
class GFxGradientRamp
{
public:
    enum { RampSize = 256 };

    GFxGradientRamp() {}
    GFxGradientRamp(const GradientRecord* colorStops, 
                    unsigned numColors, 
                    float gamma=1)
    {
        Init(colorStops, numColors, gamma);
    }

    void Init(const GradientRecord* colorStops, 
              unsigned numColors, 
              float gamma=1);

    const Color& GetColor(unsigned i) const 
    { 
        return Ramp[(i < RampSize) ? i : RampSize-1]; 
    }

private:
    struct ColorType
    {
        UInt16 r,g,b,a;
        ColorType() {}
        ColorType(UInt16 r_, UInt16 g_, UInt16 b_, UInt16 a_) :
            r(r_), g(g_), b(b_), a(a_) {}
    };

    static ColorType applyGamma(const Color& c, const UInt16* gammaLut)
    {
        int a = c.GetAlpha();
        return ColorType(gammaLut[c.GetRed()  ], 
                         gammaLut[c.GetGreen()], 
                         gammaLut[c.GetBlue() ],
                         UInt16((a << 8) | a));
    }

    static UByte applyGamma(UInt16 v, float gamma)
    {
        return (UByte)floor(pow(v / 65535.0f, gamma) * 255.0f + 0.5f);
    }

    static Color blendColors(const ColorType& c1, const ColorType& c2, 
                              int ratio, int maxRatio,
                              float gamma)
    {
        int alphaDiv = (maxRatio << 8) | maxRatio;
        return Color(applyGamma(UInt16(c1.r + int(c2.r - c1.r) * ratio / maxRatio), gamma), 
                      applyGamma(UInt16(c1.g + int(c2.g - c1.g) * ratio / maxRatio), gamma), 
                      applyGamma(UInt16(c1.b + int(c2.b - c1.b) * ratio / maxRatio), gamma),
                           UByte(c1.a + int(c2.a - c1.a) * ratio / alphaDiv));
    }

    static Color blendColors(const ColorType& c1, const ColorType& c2, 
                              int ratio, int maxRatio)
    {
        maxRatio = (maxRatio << 8) | maxRatio;
        return Color(UByte(c1.r + int(c2.r - c1.r) * ratio / maxRatio), 
                      UByte(c1.g + int(c2.g - c1.g) * ratio / maxRatio), 
                      UByte(c1.b + int(c2.b - c1.b) * ratio / maxRatio),
                      UByte(c1.a + int(c2.a - c1.a) * ratio / maxRatio));
    }

    Color Ramp[RampSize];
};


//---------------------------------------------------------------------
void GFxGradientRamp::Init(const GradientRecord* colorStops, 
                           unsigned numColors, 
                           float gamma)
{
    unsigned i;
    if (numColors == 0 || colorStops == 0)
    {
        GradientRecord fakeColor;
        fakeColor.Ratio = 0;
        fakeColor.ColorV = 0xFF000000;
        numColors  = 1;
        colorStops = &fakeColor;
    }

    if(numColors > 1)
    {
        UInt16 gammaDir[256];
        float  gammaInv = 1;

        if(gamma == 1)
        {
            for(i = 0; i < 256; i++)
            {
                gammaDir[i] = UInt16( (i << 8) | i );
            }
        }
        else
        {
            // It's a simplified calculation of the linear color gradient LUT
            // Replace this LUT with sRGB if necessary
            for(i = 0; i < 256; i++)
            {
                gammaDir[i] = (UInt16)floor(pow(i / 255.0f, gamma) * 65535.0f + 0.5f);
            }
            gammaInv = 1/gamma;
        }

        unsigned start  = colorStops[0].Ratio;
        unsigned end    = 0;
        Color c = colorStops[0].ColorV;
        for(i = 0; i < start; i++) 
        {
            Ramp[i] = c;
        }
        for(i = 1; i < numColors; i++)
        {
            end = colorStops[i].Ratio;
            if(end < start) end = start;

            ColorType c1 = applyGamma(colorStops[i-1].ColorV, gammaDir);
            ColorType c2 = applyGamma(colorStops[i].ColorV,   gammaDir);

            unsigned x0 = start;
            unsigned dx = end - start;
            if(gamma == 1)
            {
                while(start < end)
                {
                    c = Ramp[start] = blendColors(c1, c2, start-x0+1, dx);
                    ++start;
                }
            }
            else
            {
                while(start < end)
                {
                    c = Ramp[start] = blendColors(c1, c2, start-x0+1, dx, gammaInv);
                    ++start;
                }
            }
        }

        for(; end < RampSize; end++)
        {
            Ramp[end] = c;
        }
    }
    else
    {
        for(i = 0; i < RampSize; i++)
        {
            Ramp[i] = colorStops[0].ColorV;
        }
    }
}

#ifdef GFX_ADAPTIVE_GRADIENT_SIZE
const unsigned GFxGradientData_RadialGradientSizeTable[] = 
{
    16, 16, 32, 32, 64, 64, 64, 128, 128, 128, 128, 128, 128, 256, 256, 256, 256, 256
};
#endif


unsigned GradientData::ComputeRadialGradientImageSize(GradientParams *pparams) const
{   
    unsigned radialGradientImageSize = 0;

    if (pparams && !pparams->IsAdaptive())
    {
        radialGradientImageSize = pparams->GetRadialImageSize() ?
                                    pparams->GetRadialImageSize() :
                                    GFX_GRADIENT_SIZE_DEFAULT;
    }

#ifdef GFX_ADAPTIVE_GRADIENT_SIZE
    if (radialGradientImageSize == 0)
    {
        // Adaptive gradient size. The function heuristically
        // computes the optimal gradient size depending on the 
        // maximal slope in the color ramp, focal point, and
        // linearRGB flag. See also RadialGradientSizeTable that may
        // be modified to improve the result. This table and all other 
        // coefficients are defined empirically, on the basis of 
        // the visual result in typical cases. It's a kind of magic...
        //----------------------------------
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
            return GFX_GRADIENT_SIZE_DEFAULT;

        if (LinearRGB) 
            maxSlope *= 1.5f;

        if (Type == Fill_FocalPointGradient)
        {
            float r = fabsf(FocalRatio);
            if (r > 0.5f)
                maxSlope /= 1.01f - r;
        }

        if (maxSlope < 0)
            maxSlope = 0;

        maxSlope = sqrtf((maxSlope + 0.18f) * 5);
        unsigned idx = (unsigned)maxSlope;
        if (idx >= sizeof(GFxGradientData_RadialGradientSizeTable) / sizeof(unsigned)) 
            idx  = sizeof(GFxGradientData_RadialGradientSizeTable) / sizeof(unsigned) - 1;
        return GFxGradientData_RadialGradientSizeTable[idx];
    }
    else
        return radialGradientImageSize;
#else
    return radialGradientImageSize ? radialGradientImageSize : GFX_GRADIENT_SIZE_DEFAULT;
#endif // GFX_ADAPTIVE_GRADIENT_SIZE
}


// Make a Render::Texture* corresponding to our gradient.
// We can use this to set the gradient fill style.
Render::Image* GradientData::CreateGradientImage(GradientParams *pparams,
                                             MemoryHeap* pimageHeap) const
{
    SF_UNUSED(pimageHeap); 
    SF_UNUSED(pparams);
#ifndef SF_NO_GRADIENT_GEN
    SF_ASSERT(Type == Fill_LinearGradient ||
            Type == Fill_RadialGradient ||
            Type == Fill_FocalPointGradient);
    SF_ASSERT_ON_GRADIENT_BITMAP_GEN;
    
    Render::Image *pim = 0;

    unsigned i, j, size;
    int ratio;
    float center, radius;
    Color sample;
    UByte r, g, b, a;
    float x, y;
    unsigned imgSize;

    // Gamma=2.17 is the closest approximation of the sRGB curve.
    //--------------------
    GFxGradientRamp ramp(pRecords, RecordCount, LinearRGB ? 2.17f : 1.0f);

    switch(Type)
    {
    case Fill_LinearGradient: // Linear gradient.
        if ((pim = Render::Image::CreateImage(Render::Image::Image_ARGB_8888, 256, 1, pimageHeap))!=0)
        {
            for (i = 0; i < pim->Width; i++)
            {
                sample = ramp.GetColor(i);
                pim->SetPixelRGBA(i, 0, 
                                  sample.GetRed(), 
                                  sample.GetGreen(), 
                                  sample.GetBlue(), 
                                  sample.GetAlpha());
            }
        }           
        break;

    case Fill_RadialGradient: // Radial gradient.
        imgSize = ComputeRadialGradientImageSize(pparams);
        if ((pim = Render::Image::CreateImage(Render::Image::Image_ARGB_8888, imgSize, imgSize, pimageHeap))!=0)
        {
            center = pim->Height / 2.0f;
            radius = center - 1;
            size   = imgSize - 1;

            for (j = 1; j < size; j++)
            {
                for (i = 1; i < size; i++)
                {                       
                    y = j - center + 0.5f;
                    x = i - center + 0.5f;
                    ratio = (int)floorf((float)sqrt(x * x + y * y) * 256 / radius + 0.5f);
                    if (ratio > 255)
                    {
                        ratio = 255;
                    }
                    sample = ramp.GetColor(ratio);
                    pim->SetPixelRGBA(i, j, 
                                      sample.GetRed(), 
                                      sample.GetGreen(), 
                                      sample.GetBlue(), 
                                      sample.GetAlpha());
                }
            }
            sample = ramp.GetColor(255);
            r = sample.GetRed();
            g = sample.GetGreen();
            b = sample.GetBlue();
            a = sample.GetAlpha();
            for(i = 1; i < imgSize; i++)
            {
                pim->SetPixelRGBA(i, 0,                          r, g, b, a);
                pim->SetPixelRGBA(pim->Width-1, i,               r, g, b, a);
                pim->SetPixelRGBA(pim->Width-i-1, pim->Height-1, r, g, b, a);
                pim->SetPixelRGBA(0, pim->Height-i-1,            r, g, b, a);
            }
        }
        break;

    case Fill_FocalPointGradient: // Radial gradient with focal point.
        imgSize = ComputeRadialGradientImageSize(pparams);
        if ((pim = Render::Image::CreateImage(Render::Image::Image_ARGB_8888, imgSize, imgSize, pimageHeap))!=0)
        {
            center = pim->Height / 2.0f;
            radius = center - 1;
            size   = imgSize - 1;

            GFocalRadialGradient gr(radius, FocalRatio * radius, 0);

            for (j = 1; j < size; j++)
            {
                for (i = 1; i < size; i++)
                {
                    y = j - center + 0.5f;
                    x = i - center + 0.5f;
                    ratio = (int)floorf(gr.Calculate(x, y) * 256 / radius + 0.5f);
                    if (ratio > 255)
                    {
                        ratio = 255;
                    }
                    sample = ramp.GetColor(ratio);
                    pim->SetPixelRGBA(i, j, 
                                      sample.GetRed(), 
                                      sample.GetGreen(), 
                                      sample.GetBlue(), 
                                      sample.GetAlpha());
                }
            }
            sample = ramp.GetColor(255);
            r = sample.GetRed();
            g = sample.GetGreen();
            b = sample.GetBlue();
            a = sample.GetAlpha();
            for(i = 1; i < imgSize; i++)
            {
                pim->SetPixelRGBA(i, 0,                          r, g, b, a);
                pim->SetPixelRGBA(pim->Width-1, i,               r, g, b, a);
                pim->SetPixelRGBA(pim->Width-i-1, pim->Height-1, r, g, b, a);
                pim->SetPixelRGBA(0, pim->Height-i-1,            r, g, b, a);
            }
        }
        break;
    }       
    return pim;
#else //SF_NO_GRADIENT_GEN
    return 0;
#endif //SF_NO_GRADIENT_GEN
}




//
// *** FillStyle
//


FillStyle::FillStyle()
{
    Type        = Fill_Solid;
    FillFlags   = FF_HasAlpha;
    pGradientData = 0;
}

FillStyle::FillStyle(const FillStyle& other)
{
    Type = Fill_Solid;
    // We need to take care of gradient assignment, etc;   
    // so just call operator ==.
    *this = other;
}

FillStyle::~FillStyle()
{
    if (IsGradientFill())
    {
        if (pGradientData)
            pGradientData->Release();
    }
}


void    FillStyle::Read(LoadProcess* p, TagType tagType)
{
    Type        = p->ReadU8();
    FillFlags   = 0;

    p->LogParse("  FillStyle read type = 0x%X\n", Type);

    if (Type == Fill_Solid)
    {
        // Read solid fill color.
        Color color;
        p->ReadRgbaTag(&color, tagType);
        
        p->LogParse("  color: ");
        p->GetStream()->LogParseClass(color);

        if (color.GetAlpha() < 255)
            FillFlags |= FF_HasAlpha;

        RawColor = color.Raw;
    }

    else if (IsGradientFill())
    {
        // 0x10: linear gradient fill
        // 0x12: radial gradient fill
        // 0x13: focal-point gradient fill for SWF 8
        
        p->GetStream()->ReadMatrix(&ImageMatrix);

        // GRADIENT
        UInt16 numGradients = p->ReadU8();

        // For DefineShape5 gradient records are interpreted differently.
        // Note that Flash 8 can include these extra bits even for old tag types.
        if (numGradients & 0x10)
            FillFlags |= FF_LinearRGB;
        // TBD: Other bits may be set here, such as 0x80 for focal point
        // Do they need custom processing?
        numGradients &= 0xF;
        
        // SWF 8 allows up to 15 points.
        SF_ASSERT(numGradients >= 1 && numGradients <= 15);
        // Allocate gradients.
        pGradientData = SF_NEW GradientData((FillType)Type, numGradients, IsLinearRGB());
        if (!pGradientData ||
            pGradientData->GetRecordCount() != numGradients)
        {
            // Memory error...
            if (pGradientData)
                pGradientData->Release();
            RawColor = 0;
            Type = Fill_Solid;
            return;
        }
                
        for (int i = 0; i < numGradients; i++)
        {
            (*pGradientData)[i].Read(p, tagType);
            // Flag if the gradient has alpha channel.
            if ((*pGradientData)[i].ColorV.GetAlpha() < 255)
                FillFlags |= FF_HasAlpha;
        }

        p->LogParse("  gradients: numGradients = %d\n", numGradients);

        // Record focal point.          
        if (Type == Fill_FocalPointGradient)           
            pGradientData->SetFocalRatio((float) (p->ReadS16()) / 256.0f);


        // The image gradient must be replaced by gradient data,
        // which controls how it will be generated during binding.

        // 1. Get gradient id.
        
        ResourceId charId = p->GetNextGradientId();


        // If the gradient corresponds to an image in a file,
        // cache it. Otherwise get the corresponding handle

        const ExporterInfo* pexporterInfo = p->GetExporterInfo();
        bool imageFromFile =  pexporterInfo && 
                              (pexporterInfo->ExportFlags & ExporterInfo::EXF_GradientTexturesExported);

        if (imageFromFile)
        {
            // We do not have to use bitmap lookup table any longer since
            // gradient file loading is matched through ResourceLib.                              
            ResourceHandle rh;
            if (p->GetResourceHandle(&rh, charId))            
                pImage.SetFromHandle(rh);            
        }
        else
        {
            // 2. Create an image handle for gradient
            ResourceData     resData = pGradientData->CreateGradientResourceData();
            ResourceHandle   rh      = p->AddDataResource(charId, resData);

            if (!rh.IsNull())
                pImage.SetFromHandle(rh);
        }

// for 'planeta.swf' loading
//        SF_ASSERT(!(pImage.IsIndex() && pImage.GetBindIndex() == 1));


        // We used to pre-compute parts of image matrices here, so that inversion 
        // did not need to happen in GetTexture. However, two reasons caused this
        // logic to be moved to rendering side:
        //   - the original matrix has to be lerped for correct gradient morph results
        //   - with ResourceLibn we don't know image sizes are they are computed later
        //     based on GradientParams, so those sizes can not be factored into the
        //     image matrix.
    }

    else if (IsImageFill())
    {
        // 0x40: tiled bitmap fill
        // 0x41: clipped bitmap fill
        // 0x42: non-smoothed tiled fill
        // 0x43: non-smoothed clipped bitmap fill

        ResourceId bitmapResourceId = ResourceId(p->ReadU16());
        p->LogParse("  BitmapChar = %d\n", bitmapResourceId.GetIdIndex());

        //      
        if (!p->GetResourceHandle(&pImage, bitmapResourceId))
        {
            // Should not happen..
            // New image ??
        }
        else
        {
            TempBindData* bd = p->GetTempBindData();

            if (bd && (Type == Fill_TiledImage || Type == Fill_TiledSmoothImage))
                bd->FillStyleImageWrap.Add(pImage.GetBindIndex());
        }


        // Look up the bitmap character.
        /*
        pBitmap = md->GetBitmapResource(bitmapResourceId);
        if (!pBitmap && bitmapResourceId != 0xFFFF)
        {
            // probably, this bitmap will be imported later.
            // we need to create an empty bitmap resource that will be
            // filled in the ImportResolve.
            pBitmap = *new GFxBitmapResource(0);
            md->AddBitmapResource(bitmapResourceId, pBitmap);
        }
        */
        
        p->GetStream()->ReadMatrix(&ImageMatrix);
        
        // Always 1 for now, should probably check formats in the future.
        FillFlags |= FF_HasAlpha;        
        p->GetStream()->LogParseClass(ImageMatrix);
    }
}



/*
// Push our style parameters into the renderer.
void    FillStyle::Apply(const DisplayContext& dc,
                            float scaleMultiplier,
                            const Matrix2D* s9gImgAdjust) const
{   
    Render::Renderer *prenderer = dc.GetRenderer();

    if (Type == Fill_Solid)
    {
        // 0x00: solid fill
        prenderer->FillStyleColor(Color(RawColor));
    }
    else
    {
        Render::Renderer::FillTexture ft;
        ft.pTexture = 0;
        
        // Get the texture for gradient/bitmap fill and use it.
        GetFillTexture(&ft, dc, scaleMultiplier, s9gImgAdjust);
        if (ft.pTexture)        
            prenderer->FillStyleBitmap(&ft);

    }
}

// Creates a FillTexture for a style
bool    FillStyle::GetFillTexture(Render::Renderer::FillTexture *pftexture,
                                     const DisplayContext& dc,
                                     float scaleMultiplier,
                                     const Matrix2D* s9gImgAdjust) const
{
    if (IsGradientFill())
    {
        return GetGradientFillTexture(pftexture, dc, scaleMultiplier, s9gImgAdjust);
    }
    else if (IsImageFill())
    {
        return GetImageFillTexture(pftexture, dc, scaleMultiplier, s9gImgAdjust);
    }
    return 0;
}


bool    FillStyle::GetGradientFillTexture(Render::Renderer::FillTexture *pftexture,
                                          const DisplayContext& dc,
                                          float scaleMultiplier,
                                          const Matrix2D* s9gImgAdjust) const
{
#ifdef SF_AMP_SERVER
    AMP::ScopeFunctionTimer fillTimer(dc.pStats, AMP::NativeCodeSwdHandle, AMP::Func_GFxFillStyle_GetGradientFillTexture);
#endif

    Render::Renderer *prenderer = dc.GetRenderer();
    Matrix2D imgMtx;
    SF_ASSERT(prenderer);
    SF_ASSERT(dc.pResourceBinding);

    // Type == Fill_LinearGradient ||
    // Type == Fill_RadialGradient ||
    // Type == Fill_FocalPointGradient (SWF 8)
    
    ImageResource* pimageRes = (*dc.pResourceBinding)[pImage];
    SF_ASSERT(!pimageRes || (pimageRes->GetResourceType() == Resource::RT_Image));
    
    if (!pimageRes)
    {
        // If our gradient image IS an index and is NOT valid it means that 
        // it is being rendered before being properly resolved at bind time.
        if (pImage.IsIndex())
        {
            SF_ASSERT(0);
            return 0;
        }

        // We can get here when morphing gradient styles.
        // TBD: May need to do locking here!

        pimageRes = pGradientData->CreateImageResource(
                        dc.pWeakLib, dc.pGradientParams, dc.pImageCreator,
                        dc.GetRenderConfig(),dc.pLog, false);

        if (pimageRes)
        {
            // TBD: Bad because we shouldn't we storing temporary
            //      instance - related data in fill style.
            // Threading - the fact that we can only display in one thread
            // may help us.
            FillStyle*   thisNonConst = const_cast<FillStyle*>(this);
            thisNonConst->pImage = pimageRes;

            // Release the extra gradient reference created above.
            pimageRes->Release();
        }            
    }

    if (pimageRes)
    {
        ImageInfoBase* pimageInfo =  pimageRes->GetImageInfo();

        if (pimageInfo)
        {   
            pftexture->pTexture         = pimageInfo->GetTexture(prenderer);
            pftexture->WrapMode         = Render::Renderer::Wrap_Clamp;
            pftexture->SampleMode       = Render::Renderer::Sample_Linear;

            pftexture->TextureMatrix.SetIdentity();

            if (Type == Fill_LinearGradient)
            {
                pftexture->TextureMatrix.AppendScaling(1.0f / 32768.F);
                pftexture->TextureMatrix.AppendTranslation(0.5F, 0.F);
            }
            else
            {
                // TBD: Some of these matrix computations could have been pre-computed;
                // however, that is hard to do since we don't know the image size at load time.
                // Furthermore, morphing requires original matrices to be used.
            
                pftexture->TextureMatrix.AppendScaling(1.0f / 32768.F);
                pftexture->TextureMatrix.AppendTranslation(0.5F, 0.5F);
                // gradient's square size is 32768x32768 (-16,384 - 16,384)
            }

            // When using scale9grid with gradients or images, 
            // the image matrix has to be adjusted according to the 
            // "logical difference" between the world matrix and the 
            // central part of the scale9grid. The s9gImgAdjust (if not null)
            // represents this adjustment matrix.
            imgMtx = ImageMatrix;
            if (s9gImgAdjust)
                imgMtx.Append(*s9gImgAdjust);

            pftexture->TextureMatrix.Prepend(imgMtx.GetInverse());
            pftexture->TextureMatrix.PrependScaling(scaleMultiplier);
            
            return 1;
        }
    }
    return 0;
}

bool    FillStyle::GetImageFillTexture(Render::Renderer::FillTexture *pftexture,
                                          const DisplayContext& dc,
                                          float scaleMultiplier,
                                          const Matrix2D* s9gImgAdjust) const
{
#ifdef SF_AMP_SERVER
    AMP::ScopeFunctionTimer fillTimer(dc.pStats, AMP::NativeCodeSwdHandle, AMP::Func_GFxFillStyle_GetImageFillTexture);
#endif

    Render::Renderer *prenderer = dc.GetRenderer();
    Render::Renderer::Matrix imgMtx;
    SF_ASSERT(prenderer);

    // Bitmap Fill (either tiled or clipped)
    ImageResource* pimageRes     = (*dc.pResourceBinding)[pImage];
    ImageInfoBase*   pimageInfo    = pimageRes ? pimageRes->GetImageInfo() : 0;

    if (pimageInfo)
    {
        Render::Texture* ptexture = pimageInfo->GetTexture(prenderer);
        if (ptexture)
        {
            Render::Renderer::BitmapWrapMode   wm = Render::Renderer::Wrap_Repeat;
            Render::Renderer::BitmapSampleMode sm = Render::Renderer::Sample_Linear;

            switch(Type)
            {
            case Fill_TiledSmoothImage:     wm = Render::Renderer::Wrap_Repeat;    sm = Render::Renderer::Sample_Linear; break;
            case Fill_ClippedSmoothImage:   wm = Render::Renderer::Wrap_Clamp;     sm = Render::Renderer::Sample_Linear; break;
            case Fill_TiledImage:           wm = Render::Renderer::Wrap_Repeat;    sm = Render::Renderer::Sample_Point; break;
            case Fill_ClippedImage:         wm = Render::Renderer::Wrap_Clamp;     sm = Render::Renderer::Sample_Point; break;
            default:
                SF_ASSERT(0);
            }

            // When using scale9grid with gradients or images, 
            // the image matrix has to be adjusted according to the 
            // "logical difference" between the world matrix and the 
            // central part of the scale9grid. The s9gImgAdjust (if not null)
            // represents this adjustment matrix.
            imgMtx = ImageMatrix;
            if (s9gImgAdjust)
                imgMtx.Append(*s9gImgAdjust);

            Render::Rect<int> rect = pimageInfo->GetRect();

            pftexture->pTexture  = ptexture;
            pftexture->TextureMatrix.SetInverse(imgMtx);
            pftexture->TextureMatrix.PrependScaling(scaleMultiplier);
            
            pftexture->TextureMatrix.AppendTranslation(float(rect.x1), float(rect.y1));
            pftexture->TextureMatrix.AppendScaling(1.0f/pimageInfo->GetWidth(), 1.0f/pimageInfo->GetHeight());

            pftexture->WrapMode  = wm;
            pftexture->SampleMode= sm;
            return 1;
        }
    }

    return 0;
}

ImageInfoBase*   FillStyle::GetImageInfo(const DisplayContext& dc) const
{
    if (IsImageFill() || IsGradientFill())
    {
        ImageResource* pimageRes     = (*dc.pResourceBinding)[pImage];
        if (pimageRes)
            return pimageRes->GetImageInfo();
    }
    return 0;
}
*/

// Sets this style to a blend of a and b.  t = [0,1]
void    FillStyle::SetLerp(const FillStyle& a, const FillStyle& b, float t)
{
    SF_ASSERT(t >= 0 && t <= 1);

    // fill style type
    Type = (UByte)a.GetType();
    SF_ASSERT(Type == b.GetType());


    if (Type == Fill_Solid)
    {
        // Fill style color
        RawColor = Color::Blend(a.GetColor(), b.GetColor(), t).Raw;
        return;
    }

    else if (IsGradientFill())
    {
        SF_ASSERT(a.pGradientData && b.pGradientData);
        SF_ASSERT(a.pGradientData->GetRecordCount() == b.pGradientData->GetRecordCount());

        GradientData &agd = *a.pGradientData,
                        &bgd = *b.pGradientData;

        // We have to create a new GradientData object here because
        // gradient data objects are used as keys when generating gradient 
        // image resources and thus can not be modified!!
        if (pGradientData)
            pGradientData->Release();
        pGradientData = SF_NEW GradientData(a.GetType(),
                                             agd.GetRecordCount(),
                                             a.IsLinearRGB());
        if (pGradientData)
        {
            for (unsigned j=0; j < pGradientData->GetRecordCount(); j++)
            {                
                (*pGradientData)[j].Ratio =
                    (UByte) Alg::IRound(Alg::Lerp((float)agd[j].Ratio, (float)bgd[j].Ratio, t));
                (*pGradientData)[j].ColorV = Color::Blend(agd[j].ColorV,
                                                          bgd[j].ColorV, t);
            }
        }

        pGradientData->SetFocalRatio(Alg::Lerp(agd.GetFocalRatio(),
                                            bgd.GetFocalRatio(), t));

        // Reset image handle ??? So that it is cached??
        // Need to test gradient morphing
        // NULL Handle -> uninitialized image pointer ??
        pImage = 0;

        // We will need to do this in thread safe manner
        // and coordinate with the library...
    }

    else if (IsImageFill())
    {
        pImage = a.pImage;
        SF_ASSERT(pImage == b.pImage);
    }

    // fill style bitmap Matrix2D
    ImageMatrix.SetLerp(a.ImageMatrix, b.ImageMatrix, t);
}


FillStyle& FillStyle::operator = (const FillStyle& src)
{
    // Implement correct ref-counting for pGradientData.
    if (src.IsGradientFill() && src.pGradientData)
        src.pGradientData->AddRef();
    if (IsGradientFill() && pGradientData)
        pGradientData->Release();

    Type            = src.Type;
    FillFlags       = src.FillFlags;

    pGradientData   = src.pGradientData;
    SF_COMPILER_ASSERT(sizeof(GradientData*) >= sizeof(RawColor));

    pImage          = src.pImage;
    ImageMatrix     = src.ImageMatrix;

    return *this;
}




void FillStyle::SetFillType(FillType type)
{
    if (IsGradientFill() && pGradientData)
        pGradientData->Release();    

    SF_ASSERT(type <= 255);
    Type = (UByte)type; 
}


// Sets fill style to an image, used to support externally loaded images.
void    FillStyle::SetImageFill(FillType fillType,
                                   ImageResource *pimage,
                                   const Matrix2D& mtx)
{   
    SetFillType(fillType);  
    SF_ASSERT(IsImageFill());

    /*
    if (bilinear)
    {
        if (tiled) Type = (UByte) Fill_TiledSmoothImage;
        else       Type = (UByte) Fill_ClippedSmoothImage;
    }
    else
    {
        if (tiled) Type = (UByte) Fill_TiledImage;
        else       Type = (UByte) Fill_ClippedImage;
    }
    */

    FillFlags   = FF_HasAlpha;
    ImageMatrix = mtx;
    ImageMatrix.AppendScaling(20);
    pImage      = pimage;
}

void    FillStyle::SetGradientFill(FillType fillType,
                                      GradientData *pgradientData,
                                      const Matrix2D& mtx)
{
    if (!(IsGradientFill() && pgradientData == pGradientData))
    {
        SetFillType(fillType);
        SF_ASSERT(pgradientData->GetFillType() == fillType);
        SF_ASSERT(IsGradientFill());

        // Gradient data must live in a global heap since it is used as a key in ResourceLib.
        SF_ASSERT(Memory::GetHeapByAddress(pgradientData) == Memory::GetGlobalHeap());

        pGradientData = pgradientData;
        if (pGradientData)
            pGradientData->AddRef();
    }
    
    ImageMatrix = mtx;
    ImageMatrix.M[0][2] = PixelsToTwips(ImageMatrix.M[0][2]);
    ImageMatrix.M[1][2] = PixelsToTwips(ImageMatrix.M[1][2]);
}




//
// LineStyle
//


LineStyle::LineStyle()        
{
    Width       = 0;
    pComplexFill= 0;
    StyleFlags  = 0;
#ifdef SF_RENDER_ENABLE_STROKER
    MiterSize   = 1.0f; // Must be 1.0 for GetStrokeExtent() to work correctly.
#endif
}

LineStyle::LineStyle(const LineStyle& other)
{
    pComplexFill = 0;
    *this = other;
}

LineStyle::~LineStyle()
{
    if (pComplexFill)
        delete pComplexFill;
}

// Line styles are stored in an array, so implement =
// for safety of data structures on assignment.
LineStyle&   LineStyle::operator = (const LineStyle& src)
{
    Width       = src.Width;
    ColorV      = src.ColorV;

#ifdef SF_RENDER_ENABLE_STROKER
    StyleFlags  = src.StyleFlags;
    MiterSize   = src.MiterSize;
#endif

    if (pComplexFill)
    {
        delete pComplexFill;
        pComplexFill = 0;
    }
    if (src.pComplexFill)
        pComplexFill = SF_NEW FillStyle(*src.pComplexFill);

    return *this;
}

void LineStyle::SetComplexFill(const FillStyle& f)
{
    if (pComplexFill)
        delete pComplexFill;
    pComplexFill = SF_NEW FillStyle(f);
}

FillStyle* LineStyle::CreateComplexFill()
{ 
    if (pComplexFill == 0)
        pComplexFill = SF_NEW FillStyle;
    return pComplexFill;
}

void    LineStyle::Read(LoadProcess* p, TagType tagType)
{
    Width = p->ReadU16();

    UInt16 styleFlags = 0;

    // Extra values come here for DefineShape5.
    if (tagType == Tag_DefineShape4)
    {
        styleFlags = p->ReadU16();
        StyleFlags = styleFlags;

        // Miter is followed by a special 16.16 value
        if (styleFlags & LineJoin_Miter)
        {
#ifdef SF_RENDER_ENABLE_STROKER     
            MiterSize = (float)(p->ReadU16()) / 256.0f;
#else 
            p->ReadU16();
#endif
        }
    }
    
    // If we have complex SWF 8 fill style, read it in.
    if (styleFlags & LineFlag_ComplexFill)
    {
        FillStyle    fs, *pfill;         

        if ((pComplexFill = SF_HEAP_NEW(p->GetLoadHeap()) FillStyle)!=0)
            pfill = pComplexFill;
        else
            pfill = &fs;

        pfill->Read(p, tagType);

        // Avoid ASSERT in GetColor() for gradient strokes.
        if (pfill->IsSolidFill())
            ColorV = pfill->GetColor();
        else if (pfill->IsGradientFill())
        {
            GradientData *pgdata = pfill->GetGradientData();
            if (pgdata && pgdata->GetRecordCount() > 0)
                ColorV = (*pgdata)[0].ColorV;
        }
    }
    else
    {
        // Usual case: solid color line.
        p->ReadRgbaTag(&ColorV, tagType);
    }
}


/*
void    LineStyle::Apply(Render::Renderer *prenderer) const
{   
    prenderer->LineStyleColor(ColorV);
    //prenderer->LineStyleWidth(Width);
}
*/

}} // namespace Scaleform::GFx
