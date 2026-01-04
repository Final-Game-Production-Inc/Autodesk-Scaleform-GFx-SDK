/**************************************************************************

PublicHeader:   Render
Filename    :   Render_ResizeImage.h
Content     :   Bitmap Image resizing with filtering
Created     :   2007
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

----------------------------------------------------------------------
The code of these classes was taken from the Anti-Grain Geometry
Project and modified for the use by Scaleform/Autodesk. 
Permission to use without restrictions is hereby granted to 
Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
See http://antigrain.com for details.
**************************************************************************/

#ifndef INC_SF_Render_ResizeImage_H
#define INC_SF_Render_ResizeImage_H

#include "Kernel/SF_ArrayUnsafe.h"
#include "Kernel/SF_Math.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
class LinearInterpolator
{
public:
    //--------------------------------------------------------------------
    LinearInterpolator(int y1, int y2, int count) :
    Cnt(count),
        Lft((y2 - y1) / count),
        Rem((y2 - y1) % count),
        m_y(y1)
    {
        Mod = Rem;
        if(Mod <= 0)
        {
            Mod += count;
            Rem += count;
            Lft--;
        }
        Mod -= count;
    }

    //--------------------------------------------------------------------
    void operator++()
    {
        Mod += Rem;
        m_y   += Lft;
        if(Mod > 0)
        {
            Mod -= Cnt;
            m_y++;
        }
    }

    //--------------------------------------------------------------------
    int y() const { return m_y; }

private:
    int Cnt;
    int Lft;
    int Rem;
    int Mod;
    int m_y;
};


//------------------------------------------------------------------------
enum ImageFilterConstants
{
    ImgSubpixelShift   = 8,
    ImgSubpixelShift2  = ImgSubpixelShift * 2,
    ImgSubpixelScale   = 1 << ImgSubpixelShift,
    ImgSubpixelMask    = ImgSubpixelScale - 1,
    ImgSubpixelOffset  = ImgSubpixelScale / 2,
    ImgSubpixelInitial = ImgSubpixelScale * ImgSubpixelScale / 2,

    ImgFilterShift = 14,
    ImgFilterScale = 1 << ImgFilterShift,
    ImgFilterMask  = ImgFilterScale - 1,

    ImgMaxFilterRadius = 8
};


//------------------------------------------------------------------------
class ImageFilterLut
{
public:
    template<class FilterF> void Calculate(const FilterF& filter,
                                           bool normalization=true)
    {
        float r = filter.GetRadius();
        reallocLut(r);
        unsigned i;
        unsigned pivot = GetDiameter() << (ImgSubpixelShift - 1);
        for (i = 0; i < pivot; i++)
        {
            float x = float(i) / float(ImgSubpixelScale);
            float y = filter.GetWeight(x);
            WeightArray[pivot + i] = 
            WeightArray[pivot - i] = SInt16(Alg::IRound(y * ImgFilterScale));
        }
        unsigned end = (GetDiameter() << ImgSubpixelShift) - 1;
        WeightArray[0] = WeightArray[end];
        if (normalization) 
            Normalize();
    }

    ImageFilterLut() : Radius(0), Diameter(0), Start(0) {}

    template<class FilterF> ImageFilterLut(const FilterF& filter, 
                                           bool normalization=true)
    {
        Calculate(filter, normalization);
    }

    float         GetRadius()      const { return Radius;   }
    unsigned      GetDiameter()    const { return Diameter; }
    int           GetStart()       const { return Start;    }
    const SInt16* GetWeightArray() const { return &WeightArray[0]; }
    void          Normalize();

private:
    void reallocLut(float radius);
    ImageFilterLut(const ImageFilterLut&);
    const ImageFilterLut& operator = (const ImageFilterLut&);

    float                           Radius;
    unsigned                        Diameter;
    int                             Start;
    Scaleform::ArrayUnsafe<SInt16>  WeightArray;
};



//------------------------------------------------------------------------
struct ImageFilterBilinear
{
    float GetRadius() const { return 1.0f; }
    float GetWeight(float x) const
    {
        return 1.0f - x;
    }
};


//------------------------------------------------------------------------
struct ImageFilterHanning
{
    float GetRadius() const { return 1.0f; }
    float GetWeight(float x) const
    {
        return 0.5f + 0.5f * cosf((float)SF_MATH_PI * x);
    }
};


//------------------------------------------------------------------------
struct ImageFilterHamming
{
    float GetRadius() const { return 1.0f; }
    float GetWeight(float x) const
    {
        return 0.54f + 0.46f * cosf((float)SF_MATH_PI * x);
    }
};

//------------------------------------------------------------------------
struct ImageFilterHermite
{
    float GetRadius() const { return 1.0f; }
    float GetWeight(float x) const
    {
        return (2.0f * x - 3.0f) * x * x + 1.0f;
    }
};

//------------------------------------------------------------------------
struct ImageFilterQuadric
{
    float GetRadius() const { return 1.5f; }
    float GetWeight(float x) const
    {
        float t;
        if(x <  0.5f) return 0.75f - x * x;
        if(x <  1.5f) {t = x - 1.5f; return 0.5f * t * t;}
        return 0.0f;
    }
};

//------------------------------------------------------------------------
// The piecewise cubic interpolation function can be controlled by two parameters
// (B and C) as shown in the function below.  The effects of changing these
// parameters is generalized in the graph of B and C on the following page.  This
// graph is the result of a subjective test in which nine expert observers were
// shown images reconstructed with random values of B and C. The observers were 
// asked to classify the appearance of the images into one of these categories: 
// blurring, ringing, anisotropy, and satisfactory. The graph shows the results of 
// the test over the range of 0.0 to 1.0.  Of course B and C may be outside this 
// range, the effects will be more noticeable.
//            1.0 +-------------+---------------
//                |             |               |
//                |             |               |
//         B  0.8 +           _~ \      II      | Regions:
//                |     I    /    \             | -------
//         P      |         <      ~-_         _ 
//         a  0.6 +          >        \     _-~ | I   - Blurring
//         r      |      __-~         |~-_-~    | II  - Anisotropy
//         a      +__--~~            /          | III - Ringing
//         m  0.4 + \               |           | IV  - Anisotropy
//         e      |  \       V     /            | V   - Satisfactory
//         t      |   \           /      III    |
//         e  0.2 +    ~\         |             |
//         r      |  IV  \        \             |
//                |       ~~~\     |            |
//            0.0 +-----+-----+----++-----+-----
//               0.0   0.2   0.4   0.6   0.8   1.0
//                        C Parameter 
// 
//
// The default values for (B, C) are (1/3, 1/3) which is recommended by 
// Mitchell and Netravali. Other interesting values are:
// 
// (1, 0)   - Equivalent to the Cubic B-Spline,
// (0, 0.5) - Equivalent to the Catmull-Rom Spline,
// (0, C)   - The family of Cardinal Cubic Splines,
// (B, 0)   - Duff's tensioned B-Splines ("Splines in Animation and Modeling").
//------------------------------------------------------------------------
struct ImageFilterBicubic
{
    float B, C; 
    ImageFilterBicubic(float b=0.3333f, float c=0.3333f) : B(b), C(c) {}
    float GetRadius() const { return 2.0f; }

    float GetWeight(float x) const
    {
        if (x < 1.0f)
        {
            return 
                ((12.0f -  9.0f*B - 6.0f*C)*x*x*x + 
                (-18.0f + 12.0f*B + 6.0f*C)*x*x   + 6.0f - 2.0f*B) / 6.0f;
        }
        else 
            if (x < 2.0f)
            {
                return   ((-B -  6.0f*C)*x*x*x + (6.0f*B + 30.0f*C)*x*x + 
                    (-12.0f*B - 48.0f*C)*x     + (8.0f*B + 24.0f*C)) / 6.0f;
            }
            return 0;
    }
};

//------------------------------------------------------------------------
class ImageFilterKaiser
{
    float a;
    float i0a;
    float epsilon;

public:
    ImageFilterKaiser(float b = 6.33f) :
      a(b), epsilon(1e-6f)
      {
          i0a = 1.0f / bessel_i0(b);
      }

      float GetRadius() const { return 1.0f; }
      float GetWeight(float x) const
      {
          return bessel_i0(a * sqrtf(1.0f - x * x)) * i0a;
      }

private:
    float bessel_i0(float x) const
    {
        int i;
        float sum, y, t;

        sum = 1.0f;
        y = x * x / 4.0f;
        t = y;

        for(i = 2; t > epsilon; i++)
        {
            sum += t;
            t *= (float)y / (i * i);
        }
        return sum;
    }
};

//------------------------------------------------------------------------
struct ImageFilterCatrom
{
    float GetRadius() const { return 2.0f; }
    float GetWeight(float x) const
    {
        if(x <  1.0f) return 0.5f * (2.0f + x * x * (-5.0f + x * 3.0f));
        if(x <  2.0f) return 0.5f * (4.0f + x * (-8.0f + x * (5.0f - x)));
        return 0.0f;
    }
};

//------------------------------------------------------------------------
class ImageFilterMitchell
{
    float p0, p2, p3;
    float q0, q1, q2, q3;

public:
    ImageFilterMitchell(float b = 1.0f/3.0f, float c = 1.0f/3.0f) :
        p0((6.0f - 2.0f * b) / 6.0f),
        p2((-18.0f + 12.0f * b + 6.0f * c) / 6.0f),
        p3((12.0f - 9.0f * b - 6.0f * c) / 6.0f),
        q0((8.0f * b + 24.0f * c) / 6.0f),
        q1((-12.0f * b - 48.0f * c) / 6.0f),
        q2((6.0f * b + 30.0f * c) / 6.0f),
        q3((-b - 6.0f * c) / 6.0f)
      {}

      float GetRadius() const { return 2.0f; }
      float GetWeight(float x) const
      {
          if(x < 1.0f) return p0 + x * x * (p2 + x * p3);
          if(x < 2.0f) return q0 + x * (q1 + x * (q2 + x * q3));
          return 0.0f;
      }
};


//------------------------------------------------------------------------
struct ImageFilterSpline16
{
    float GetRadius() const { return 2.0f; }
    float GetWeight(float x) const
    {
        if(x < 1.0f)
        {
            return ((x - 9.0f/5.0f ) * x - 1.0f/5.0f ) * x + 1.0f;
        }
        return ((-1.0f/3.0f * (x-1.0f) + 4.0f/5.0f) * (x-1.0f) - 7.0f/15.0f ) * (x-1.0f);
    }
};


//------------------------------------------------------------------------
struct ImageFilterSpline36
{
    float GetRadius() const { return 3.0f; }
    float GetWeight(float x) const
    {
        if(x < 1.0f)
        {
            return ((13.0f/11.0f * x - 453.0f/209.0f) * x - 3.0f/209.0f) * x + 1.0f;
        }
        if(x < 2.0f)
        {
            return ((-6.0f/11.0f * (x-1.0f) + 270.0f/209.0f) * (x-1.0f) - 156.0f / 209.0f) * (x-1.0f);
        }
        return ((1.0f/11.0f * (x-2.0f) - 45.0f/209.0f) * (x-2.0f) +  26.0f/209.0f) * (x-2.0f);
    }
};


//------------------------------------------------------------------------
struct ImageFilterGaussian
{
    float GetRadius() const { return 2.0f; }
    float GetWeight(float x) const
    {
        return expf(-2.0f * x * x) * sqrtf(2.0f / (float)SF_MATH_PI);
    }
};


//------------------------------------------------------------------------
// Function Bessel calculates Bessel function of first kind of order n
// Arguments:
//     n - an integer (>=0), the order
//     x - value at which the Bessel function is required
//--------------------
inline float Bessel(float x, int n)
{
    if(n < 0)
    {
        return 0;
    }
    float d = 1E-6f;
    float b = 0;
    if(fabsf(x) <= d) 
    {
        if(n != 0) return 0;
        return 1;
    }
    float b1 = 0; // b1 is the value from the previous iteration
    // Set up a starting order for recurrence
    int m1 = (int)fabsf(x) + 6;
    if(fabsf(x) > 5) 
    {
        m1 = (int)(fabsf(1.4f * x + 60 / x));
    }
    int m2 = (int)(n + 2 + fabsf(x) / 4);
    if (m1 > m2) 
    {
        m2 = m1;
    }

    // Apply recurrence down from current max order
    for(;;) 
    {
        float c3 = 0;
        float c2 = 1E-30f;
        float c4 = 0;
        int m8 = 1;
        if (m2 / 2 * 2 == m2) 
        {
            m8 = -1;
        }
        int imax = m2 - 2;
        for (int i = 1; i <= imax; i++) 
        {
            float c6 = 2 * (m2 - i) * c2 / x - c3;
            c3 = c2;
            c2 = c6;
            if(m2 - i - 1 == n)
                b = c6;
            m8 = -1 * m8;
            if (m8 > 0)
                c4 = c4 + 2 * c6;
        }
        float c6 = 2 * c2 / x - c3;
        if(n == 0)
            b = c6;
        c4 += c6;
        b /= c4;
        if(fabsf(b - b1) < d)
            break;
        b1 = b;
        m2 += 3;
    }
    return b;
}



//------------------------------------------------------------------------
struct ImageFilterBessel
{
    float GetRadius() const { return 3.2383f; } 
    float GetWeight(float x) const
    {
        return (x == 0.0f) ? 
            (float)SF_MATH_PI / 4.0f : 
            Bessel((float)SF_MATH_PI * x, 1) / (2.0f * x);
    }
};


//------------------------------------------------------------------------
class ImageFilterSinc
{
public:
    ImageFilterSinc(float r=4.0f) : 
      Radius(Alg::Clamp(r, 2.0f, float(ImgMaxFilterRadius))) {}

    float GetRadius() const { return Radius; }
    float GetWeight(float x) const
    {
        if(x == 0.0f) return 1.0f;
        x *= (float)SF_MATH_PI;
        return sinf(x) / x;
    }
private:
    float Radius;
};


//------------------------------------------------------------------------
class ImageFilterLanczos
{
public:
    ImageFilterLanczos(float r=4.0f) : 
        Radius(Alg::Clamp(r, 2.0f, float(ImgMaxFilterRadius))) {}

    float GetRadius() const { return Radius; }
    float GetWeight(float x) const
    {
        if(x == 0.0f) return 1.0f;
        if(x > Radius) return 0.0f;
        x *= (float)SF_MATH_PI;
        float xr = x / Radius;
        return (sinf(x) / x) * (sinf(xr) / xr);
    }
private:
    float Radius;
};


//------------------------------------------------------------------------
class ImageFilterBlackman
{
public:
    ImageFilterBlackman(float r=4.0f) : 
      Radius(Alg::Clamp(r, 2.0f, float(ImgMaxFilterRadius))) {}

    float GetRadius() const { return Radius; }
    float GetWeight(float x) const
    {
        if(x == 0.0f) return 1.0f;
        if(x > Radius) return 0.0f;
        x *= (float)SF_MATH_PI;
        float xr = x / Radius;
        return (sinf(x) / x) * (0.42f + 0.5f*cosf(xr) + 0.08f*cosf(2.0f*xr));
    }
private:
    float Radius;
};


// Operations for the image resizer
enum ResizeImageType
{
    ResizeNone,
    ResizeRgbToRgb,     // 24-bit RGB
    ResizeRgbaToRgba,   // 32-bit RGBA
    ResizeRgbToRgba,    // Add Alpha and form RGBA
    ResizeGray          // 8-bit gray scale
};

typedef ResizeImageType ImageRescaleType;


void SF_STDCALL ResizeImageBox(UByte* pDst, 
                               int dstWidth, int dstHeight, int dstPitch,
                               const UByte* pSrc, 
                               int srcWidth, int srcHeight, int srcPitch,
                               ResizeImageType type);

void SF_STDCALL ResizeImageBilinear(UByte* pDst, 
                                    int dstWidth, int dstHeight, int dstPitch,
                                    const UByte* pSrc, 
                                    int srcWidth, int srcHeight, int srcPitch,
                                    ResizeImageType type);

void SF_STDCALL ResizeImage(UByte* pDst, 
                            int dstWidth, int dstHeight, int dstPitch,
                            const UByte* pSrc, 
                            int srcWidth, int srcHeight, int srcPitch,
                            ResizeImageType type,
                            const ImageFilterLut& filter);


}}

#endif

