/**************************************************************************

Filename    :   Render_Color.cpp
Content     :   Color types & definitions
Created     :   March 26, 1999
Authors     :   Michael Antonov & Brendan Iribe

Notes       :   Several of the complex color model conversion
                functions were adopted from freely distributed
                open source libraries with unrestrictive licenses

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Math.h"
#include "Kernel/SF_Std.h"
#include "Render_Color.h"
#ifdef SF_MATH_H
#include SF_MATH_H
#else
#include <math.h>
#endif

#include <stdio.h>

namespace Scaleform { namespace Render {

// *** Color

//#define SF_STREAM_BUFFER_SIZE  512
//
//// For debugging.
//void Color::Format(char *pbuffer) const
//{
//    Format(StringDataPtr(pbuffer, SF_STREAM_BUFFER_SIZE),
//        "RGBA: {0} {1} {2} {3}\n",
//        (int)Channels.Red, (int)Channels.Green, (int)Channels.Blue, (int)Channels.Alpha
//        );
//}

// Special blend version with floating-point factor
Color  Color::Blend               (Color c1, Color c2, float f)
{
    Color c;
    c.Channels.Red      = (UByte) Alg::URound(Alg::Lerp((float)c1.Channels.Red,   (float)c2.Channels.Red, f));
    c.Channels.Green    = (UByte) Alg::URound(Alg::Lerp((float)c1.Channels.Green, (float)c2.Channels.Green, f));
    c.Channels.Blue     = (UByte) Alg::URound(Alg::Lerp((float)c1.Channels.Blue,  (float)c2.Channels.Blue, f));
    c.Channels.Alpha    = (UByte) Alg::URound(Alg::Lerp((float)c1.Channels.Alpha, (float)c2.Channels.Alpha, f));
    return c;
}


#ifndef SF_NO_COLOR_MODEL_CONVERSION

// Constructors
Color::Color(int x, int y, int z, Model model)
{
    switch (model)
    {
        case Rgb:   SetRGB(x,y,z); break;
        
#ifndef SF_NO_COLOR_HSV_CONVERSION
        case Hsv:   SetHSV(x,y,z); break;
        case Hsi:   SetHSI(x,y,z); break;
#endif // SF_NO_COLOR_HSV_CONVERSION

#ifndef SF_NO_COLOR_COMPLEX_CONVERSION
        case Cmy:   SetCMY(x,y,z); break;
        // Scales value by 100
        case Xyz:   SetXYZ(float(x)/100,float(y)/100,float(z)/100); break;
        case Lab:   SetLab(float(x)/100,float(y)/100,float(z)/100); break;
        case Luv:   SetLuv(float(x)/100,float(y)/100,float(z)/100); break;
        case Yiq:   SetYIQ(float(x)/100,float(y)/100,float(z)/100); break;
        case Yuv:   SetYUV(float(x)/100,float(y)/100,float(z)/100); break;
#endif // SF_NO_COLOR_COMPLEX_CONVERSION
    }
}
Color::Color(int x, int y, int z, int a, Model model)
{
    switch (model)
    {
        case Rgb:   SetRGBA(x,y,z,a); break;
        
#ifndef SF_NO_COLOR_HSV_CONVERSION
        case Hsv:   SetHSV(x,y,z); SetAlpha(a); break;
        case Hsi:   SetHSI(x,y,z); SetAlpha(a); break;
#endif // SF_NO_COLOR_HSV_CONVERSION
        
#ifndef SF_NO_COLOR_COMPLEX_CONVERSION
        case Cmy:   SetCMY(x,y,z); SetAlpha(a); break;
        // Scales value by 100
        case Xyz:   SetXYZ(float(x)/100,float(y)/100,float(z)/100); SetAlpha(a); break;
        case Lab:   SetLab(float(x)/100,float(y)/100,float(z)/100); SetAlpha(a); break;
        case Luv:   SetLuv(float(x)/100,float(y)/100,float(z)/100); SetAlpha(a); break;
        case Yiq:   SetYIQ(float(x)/100,float(y)/100,float(z)/100); SetAlpha(a); break;
        case Yuv:   SetYUV(float(x)/100,float(y)/100,float(z)/100); SetAlpha(a); break;
#endif // SF_NO_COLOR_COMPLEX_CONVERSION
    }
}

Color::Color(Double x, Double y, Double z, Model model)
{
    switch (model)
    {
        case Rgb:   SetRGBFloat(float(x),float(y),float(z)); break;
        
#ifndef SF_NO_COLOR_HSV_CONVERSION
        case Hsv:   SetHSV(float(x),float(y),float(z)); break;
        case Hsi:   SetHSI(float(x),float(y),float(z)); break;
#endif // SF_NO_COLOR_HSV_CONVERSION
        
#ifndef SF_NO_COLOR_COMPLEX_CONVERSION
        case Cmy:   SetRGBFloat(float(1.0-x),float(1.0-y),float(1.0-z)); break;
        case Xyz:   SetXYZ(float(x),float(y),float(z)); break;
        case Lab:   SetLab(float(x),float(y),float(z)); break;
        case Luv:   SetLuv(float(x),float(y),float(z)); break;
        case Yiq:   SetYIQ(float(x),float(y),float(z)); break;
        case Yuv:   SetYUV(float(x),float(y),float(z)); break;
#endif // SF_NO_COLOR_COMPLEX_CONVERSION
    }
}
Color::Color(Double x, Double y, Double z, Double a, Model model)
{
    switch (model)
    {
        case Rgb:   SetRGBAFloat(float(x),float(y),float(z),float(a)); break;
        
#ifndef SF_NO_COLOR_HSV_CONVERSION
        case Hsv:   SetHSV(float(x),float(y),float(z)); SetAlphaFloat(float(a)); break;
        case Hsi:   SetHSI(float(x),float(y),float(z)); SetAlphaFloat(float(a)); break;
#endif // SF_NO_COLOR_HSV_CONVERSION

#ifndef SF_NO_COLOR_COMPLEX_CONVERSION
        case Cmy:   SetRGBAFloat(float(1.0-x),float(1.0-y),float(1.0-z),float(a)); break;
        case Xyz:   SetXYZ(float(x),float(y),float(z)); SetAlphaFloat(float(a)); break;
        case Lab:   SetLab(float(x),float(y),float(z)); SetAlphaFloat(float(a)); break;
        case Luv:   SetLuv(float(x),float(y),float(z)); SetAlphaFloat(float(a)); break;
        case Yiq:   SetYIQ(float(x),float(y),float(z)); SetAlphaFloat(float(a)); break;
        case Yuv:   SetYUV(float(x),float(y),float(z)); SetAlphaFloat(float(a)); break;
#endif // SF_NO_COLOR_COMPLEX_CONVERSION
    }
}

#endif // SF_NO_COLOR_MODEL_CONVERSION



// *** HSV Color Conversion

#ifndef SF_NO_COLOR_HSV_CONVERSION

#define SF_MATH_TWO_PI_3   2.0943951023931954923
#define SF_MATH_FOUR_PI_3  4.1887902047863909846
#define SF_MATH_FIVE_PI_3  5.23598775598298873075



/*
// HSL and RGB max values
#define     SF_COLOR_H_MAX             360     // Hue varies over 0-H_MAX
#define     SF_COLOR_S_MAX             255     // Saturation varies over 0-S_MAX
#define     SF_COLOR_L_MAX             255     // Lightness varies over 0-L_MAX
#define     SF_COLOR_RGB_MAX           255     // R,G, and B vary over 0-SF_COLOR_RGB_MAX

// Hue is undefined if Saturation is 0 (grey-scale)
#define     SF_COLOR_H_UNDEFINED       (SF_COLOR_H_MAX*2/3)

float Color_HueToRGB(float n1, float n2, float hue)
{
    if (hue > 360)  hue = hue - 360;
    if (hue < 0)    hue = hue + 360;

    if (hue < 60)   return n1 + (n2-n1) * hue / 60.0f;
    if (hue < 180)  return n2;
    if (hue < 240)  return n1 + (n2-n1) * (240-hue) / 60;
    return n1;
}

float Color_HueToRGBFloat(float n1, float n2, float hue)
{
    if ( hue < 0 ) hue += 1;
    if ( hue > 1 ) hue -= 1;

    if ( ( 6 * hue ) < 1 ) return float(n1 + ( n2 - n1 ) * 6 * hue);
    if ( ( 2 * hue ) < 1 ) return n2;
    if ( ( 3 * hue ) < 2 ) return float(n1 + ( n2 - n1 ) * ((2.0/3.0) - hue) * 6);
    return n1;
}
*/

// Hue, saturation and value
void            Color::SetHSV(int h, int s, int v)
{
    int r = v;
    int g = v;
    int b = v;

    // If the saturation is zero or the hue is negative then
    // do not process (achromatic case)
    if ( s != 0 && h > -1 )     
    {                   
        // Otherwise, process (chromatic case)
        
        if ( h >= 360 )
            h %= 360;
        unsigned f = h%60;
        h /= 60;

        unsigned p = (unsigned)(2*v*(255-s)+255)/510;
        unsigned q, t;
        if ( h&1 ) 
        {
            q = (unsigned) (2*v*(15300-s*f)+15300)/30600;
            switch ( h ) 
            {
                case 1: r=(int)q; g=(int)v, b=(int)p; break;
                case 3: r=(int)p; g=(int)q, b=(int)v; break;
                case 5: r=(int)v; g=(int)p, b=(int)q; break;
            }
        } 
        else 
        {
            t = (unsigned) (2*v*(15300-(s*(60-f)))+15300)/30600;
            switch ( h ) 
            {
                case 0: r=(int)v; g=(int)t, b=(int)p; break;
                case 2: r=(int)p; g=(int)v, b=(int)t; break;
                case 4: r=(int)t; g=(int)p, b=(int)v; break;
            }
        }       
    }       

    /*
    SetHSV Version #2
    float m1, m2;
    float hf, sf, vf;
    UByte r, g, b;

    hf = float(h) * 360.0f/SF_COLOR_H_MAX;
    sf = float(s) / SF_COLOR_S_MAX;
    vf = float(v) / SF_COLOR_L_MAX;

    if (vf <= 0.5)
        m2 = vf * (1+sf);
    else
        m2 = vf + sf - vf*sf;

    m1 = 2 * vf - m2;

    if (sf == 0) 
    {
        r = 
        g = 
        b = (UByte)(vf*255.0f);
    } 
    else 
    {
        r = (UByte)(Color_HueToRGB(m1,m2,hf+120) * 255.0f);
        g = (UByte)(Color_HueToRGB(m1,m2,hf)    * 255.0f);
        b = (UByte)(Color_HueToRGB(m1,m2,hf-120) * 255.0f);
    }
    */

    SetRGB( UByte(r), UByte(g), UByte(b) );
}

void            Color::SetHSV(float h, float s, float v)
{
    //int   i;
    float   r, g, b;
    float   f, p, q, t;
    
    if (s == 0) 
    {
        // achromatic (gray)
        r = g = b = v;
        goto convert_rgb;
    }
    // sector 0 to 5
    if (h == 1.0)
        h = 0.0;
    else
        h *= 6.0;
    //h *= 6.0;
    //i = (int)floor( h );
    // factorial part of hue
    f = h - (int)h;
    p = v * ( 1.0f - s );
    q = v * ( 1.0f - s * f );
    t = v * ( 1.0f - s * ( 1.0f - f ) );
    switch ( (int)h ) 
    {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        // case 5:
        default:        
            r = v;
            g = p;
            b = q;
            break;
    }
convert_rgb:
    SetRGBFloat(r, g, b);
}
                        
void            Color::GetHSV(int* ph, int* ps, int* pv) const
{
    // Get the current rgb values
    UByte br, bg, bb;
    GetRGB(&br, &bg, &bb);
    int  r=br, g=bg, b=bb;

    // Determine the maximum color
    enum ColorType { CT_Red, CT_Green, CT_Blue  };
    unsigned    maxColor = r;       
    ColorType   maxType  = CT_Red;
    if ( (unsigned)g > maxColor ) 
    {
        maxColor = g;
        maxType  = CT_Green;
    }
    if ( (unsigned)b > maxColor ) 
    {
        maxColor = b;
        maxType  = CT_Blue;
    }

    // Find the minimum color
    unsigned minColor = r;
    if ( (unsigned)g < minColor ) minColor = g;
    if ( (unsigned)b < minColor ) minColor = b;

    int  delta = maxColor - minColor;
    
    // calc value, saturation and hue
    *pv = maxColor;
    *ps = maxColor ? (510*delta+maxColor)/(2*maxColor) : 0;
    
    if ( *ps == 0 ) 
    {
        // undefined hue
        *ph = 0;                
    } 
    else 
    {
        switch ( maxType ) 
        {
            // red is max
            case CT_Red:
                if ( g >= b )
                    *ph = (120*(g-b)+delta)/(2*delta);
                else
                    *ph = (120*(g-b+delta)+delta)/(2*delta) + 300;
                break;
            // green is max
            case CT_Green:              
                if ( b > r )
                    *ph = 120 + (120*(b-r)+delta)/(2*delta);
                else
                    *ph =  60 + (120*(b-r+delta)+delta)/(2*delta);
                break;
            // blue is max
            case CT_Blue:               
                if ( r > g )
                    *ph = 240 + (120*(r-g)+delta)/(2*delta);
                else
                    *ph = 180 + (120*(r-g+delta)+delta)/(2*delta);
                break;
        }
    }
    
    /*
    // GetHSV Version #2
    UByte    r, g, b;                    
    unsigned h, s, v;
    UByte    cmax, cmin;             
    UInt16   rdelta, gdelta, bdelta;

    GetRGB(&r, &g, &b);

    // Calculate lightness
    cmax = max( max(r,g), b);   
    cmin = min( min(r,g), b);
    v = (UByte)((((cmax+cmin)*SF_COLOR_L_MAX)+SF_COLOR_RGB_MAX)/(2*SF_COLOR_RGB_MAX));

    // Achromatic case (r==g==b)
    if (cmax==cmin)
    {           
        s = 0;      
        h = SF_COLOR_H_UNDEFINED;
    } 
    // Chromatic case
    else 
    {
        // Calculate saturation 
        if (v <= (SF_COLOR_L_MAX/2))   
            s = (UByte)((((cmax-cmin)*SF_COLOR_S_MAX)+((cmax+cmin)/2))/(cmax+cmin));
        else
            s = (UByte)((((cmax-cmin)*SF_COLOR_S_MAX)+((2*SF_COLOR_RGB_MAX-cmax-cmin)/2))/(2*SF_COLOR_RGB_MAX-cmax-cmin));

        // Calculate hue
        rdelta = (UInt16)((((cmax-r)*(SF_COLOR_H_MAX/6)) + ((cmax-cmin)/2) ) / (cmax-cmin));
        gdelta = (UInt16)((((cmax-g)*(SF_COLOR_H_MAX/6)) + ((cmax-cmin)/2) ) / (cmax-cmin));
        bdelta = (UInt16)((((cmax-b)*(SF_COLOR_H_MAX/6)) + ((cmax-cmin)/2) ) / (cmax-cmin));

        if (r == cmax)
            h = (UByte)(bdelta - gdelta);
        else if (g == cmax)
            h = (UByte)((SF_COLOR_H_MAX/3) + rdelta - bdelta);
        else //if (b == cmax)
            h = (UByte)(((2*SF_COLOR_H_MAX)/3) + gdelta - rdelta);

        if (h > SF_COLOR_H_MAX) h -= SF_COLOR_H_MAX;
    }

    *ph = h;
    *ps = s;
    *pv = v;
    */

}

void            Color::GetHSV(float *ph, float *ps, float *pv) const
{   
    float r, g, b;
    GetRGBFloat(&r, &g, &b);

    // get the min/max and delta values
    float minColor  = Alg::Min( r, Alg::Min(g, b) );
    float maxColor  = Alg::Max( r, Alg::Max(g, b) );
    float delta     = maxColor - minColor;

    // value
    *pv = maxColor;

    // saturation
    if ( maxColor != 0 )
        *ps = delta / maxColor;     
    else 
    {
        // if r, g and b equal 0 then
        // s = 0 and v is undefined
        *ps = 0.0;
        *ph = 0.0;
        return;
    }

    // hue
    if (*ps == 0.0)
        *ph = 0.0;
    else
    {
        if ( r == maxColor )
            *ph = ( g - b ) / delta;        // between yellow & magenta
        else if( g == maxColor )
            *ph = 2 + ( b - r ) / delta;    // between cyan & yellow
        else
            *ph = 4 + ( r - g ) / delta;    // between magenta & cyan

        // degrees
        *ph /= 6.0;

        if ( *ph < 0.0 ) *ph += 1.0;
        if ( *ph > 1.0 ) *ph -= 1.0;
    }
}


// HSI - Hue, Saturation and Intensity
void    Color::SetHSI(int h, int s, int i)
{
    float nh = float(h) / 360;
    float ns = float(s) / 255;
    float ni = float(i) / 255;
    SetHSI(nh, ns, ni);
}

void    Color::GetHSI(int* ph, int* ps, int* pi) const
{
    float h, s, i;
    GetHSI(&h,&s,&i);
    *ph = int (h * 360);
    *ps = int (s * 255);
    *pi = int (i * 255);
}

void    Color::SetHSI(float h, float s, float i)
{
    Double tR, tG, tB;
    ConvertHSIToRGB(h, s, i, &tR, &tG, &tB);
    SetRGBFloat((float)tR, (float)tG, (float)tB);

    /*
    UByte tR, tG, tB;
    
    // HSL values = From 0 to 1
    if ( s == 0 )                       
    {
        // RGB results = From 0 to 255
        tR = UByte(i * 255);
        tG = UByte(i * 255);
        tB = UByte(i * 255);
    }
    else
    {
        float n1, n2;

        if ( i < 0.5 ) n2 = i * ( 1 + s );
        else           n2 = ( i + s ) - ( s * i );

        n1 = 2 * i - n2;

        tR = UByte(255 * Color_HueToRGBFloat( n1, n2, float(h + (1.0/3.0)) ));
        tG = UByte(255 * Color_HueToRGBFloat( n1, n2, h ));
        tB = UByte(255 * Color_HueToRGBFloat( n1, n2, float(h - (1.0/3.0)) ));
    }
    SetRGB(tR, tG, tB);
    */  
}

void    Color::GetHSI(float *ph, float *ps, float *pi) const
{
    float tR, tG, tB;
    GetRGBFloat(&tR, &tG, &tB);

    Double nh, ns, ni;
    ConvertRGBToHSI(tR, tG, tB, &nh, &ns, &ni);
    *ph = (float)nh;
    *ps = (float)ns;
    *pi = (float)ni;

    /*
    // Min/Max/Delta value of RGB
    float minColor  = min( tR, min(tG, tB) );
    float maxColor  = max( tR, max(tG, tB) );
    float maxDelta  = maxColor - minColor;

    *pi = ( maxColor + minColor ) / 2;

    // This is a gray, no chromatic data
    if ( maxDelta == 0 )                     
    {
        // HSI results = From 0 to 1
        *ph = 0;                                
        *ps = 0;
    }
    else                                    // Chromatic data...
    {
        if ( *pi < 0.5 ) *ps = maxDelta / ( maxColor + minColor );
        else             *ps = maxDelta / ( 2 - maxDelta );
        
        float dr = ( ( ( maxColor - tR ) / 6 ) + ( maxDelta / 2 ) ) / maxDelta;
        float dg = ( ( ( maxColor - tG ) / 6 ) + ( maxDelta / 2 ) ) / maxDelta;
        float db = ( ( ( maxColor - tB ) / 6 ) + ( maxDelta / 2 ) ) / maxDelta;
        
        if      ( tR == maxColor ) *ph = db - dg;
        else if ( tG == maxColor ) *ph = ( 1 / 3 ) + dr - db;
        else if ( tB == maxColor ) *ph = ( 2 / 3 ) + dg - dr;
        
        if ( *ph < 0 ) *ph += 1.0;
        if ( *ph > 1 ) *ph -= 1.0;
    }
    */
}


// Precise HSI/RGB conversion using trig functions
void    Color::ConvertRGBToHSI(Double r, Double g, Double b,
                               Double *ph, Double *ps, Double *pi)
 {
    // NOTE: Use Double typecasts to avoid warnings with SF_NO_DOUBLE.
    Double i = (r+g+b) / (Double)3.0;
    Double s = (Double) ((i == 0.0) ? 1.0 : 1.0 - (Alg::Min(Alg::Min(r,g),b) / i));
    Double h;

    if (r == g && g == b)
        h = 0.0;
    else 
    {
        Double t = (Double)acos((0.5*(r-g+r-b)) / (Double)sqrt((r-g)*(r-g)+(r-b)*(g-b)));
        h = (g > b) ? t : SF_MATH_2_PI - t;
    }

    *ph = h;
    *ps = s;
    *pi = i;
}


void    Color::ConvertHSIToRGB(Double h, Double s, Double i,
                               Double *pr, Double *pg, Double *pb)
{
    Double ht;
    Double r,g,b;
    
    if (h == 0.0)
        r = g = b = i;
    else if (h > 0.0 && h < SF_MATH_TWO_PI_3 ) 
    {
        // NOTE: Use Double typecasts to avoid warnings with SF_NO_DOUBLE.
        ht =(Double) (1 / sqrt(3.0) * tan(h - SF_MATH_PI_3));
        b = (Double) (1.0 - s) * i;
        g = (Double) ((1.5 + 1.5*ht ) * i - ((0.5 + 1.5*ht) * b));
        r = (Double) (3.0 * i - g - b);
    }
    else if (h >= SF_MATH_TWO_PI_3 && h < SF_MATH_FOUR_PI_3) 
    {
        ht= (Double) (1 / sqrt(3.0) * tan(h - SF_MATH_PI));
        r = (Double) ((1.0 - s) * i);
        b = (Double) ((1.5 + 1.5*ht ) * i - ((0.5 + 1.5*ht) * r));
        g = (Double) (3.0 * i - b - r);
    }
    else 
    { 
        ht= (Double) (1 / sqrt(3.0) * tan(h - SF_MATH_FIVE_PI_3));
        g = (Double) ((1.0 - s) * i);
        r = (Double) ((1.5 + 1.5*ht ) * i - ((0.5 + 1.5*ht) * g));
        b = (Double) (3.0 * i - r - g);
    }

    *pr = r;
    *pg = g;
    *pb = b;
}

#endif // SF_NO_COLOR_HSV_CONVERSION
 


// *** Additional color conversions

#ifndef SF_NO_COLOR_COMPLEX_CONVERSION

/*
 There is a direct relationship between RGB colors and CMY colors. 
 In a CMY color, black tones are achieved by printing equal amounts 
 of Cyan, Magenta, and Yellow ink. The black component in a CMY color 
 is achieved by reducing the CMY components by the minimum of (C, M, and Y) 
 and substituting pure black in its place producing a sharper print and 
 using less ink. Since it is possible for a user to boost the C,M and Y 
 components where boosting the black component would have been preferable, 
 a ColorCorrectCMYK() function is provided to achieve the same color by
 reducing the C, M and Y components, and boosting the K component.
 
 The description above is an excerpt from a Borland Example.
*/

// CMYK - Cyan, Magenta, Yellow and Black
void            Color::SetCMYK(int c, int m, int y, int k)
{
    UByte r,g,b;
    
    if ((c + k) < 255)
         r = 255 - (c + k);
    else
         r = 0;

    if ((m + k) < 255)
         g = 255 - (m + k);
    else
         g = 0;
    
    if ((y + k) < 255)
         b = 255 - (y + k);
    else
         b = 0;

    SetRGB(r,g,b);
}

void            Color::GetCMYK(int* pc, int* pm, int* py, int* pk) const
{
    UByte r,g,b;
    GetRGB(&r,&g,&b);
    
    *pc = 255 - r;
    *pm = 255 - g;
    *py = 255 - b;
    
    if (*pc < *pm)
        *pk = *pc; 
    else
        *pk = *pm;
    
    if (*py < *pk)
        *pk = *py;
    
    if (*pk > 0)
    {
        *pc = *pc - *pk;
        *pm = *pm - *pk;
        *py = *py - *pk;
    }
}

void    Color::CorrectCMYK(int* pc, int* pm, int* py, int* pk)
{
    UByte minColor;
    
    if (*pc < *pm)
        minColor = *pc;
    else
        minColor = *pm;
    if (*py < minColor)
        minColor = *py;

    if (minColor + *pk > 255)
        minColor = 255 - *pk;

    *pc = *pc - minColor;
    *pm = *pm - minColor;
    *py = *py - minColor;
    *pk = *pk + minColor;
}


// CMY - Cyan, Magenta, Yellow
void            Color::SetCMY(int c, int m, int y)
{
    UByte r = 255 - c;
    UByte g = 255 - m;
    UByte b = 255 - y;
    SetRGB(r,g,b);
}

void            Color::GetCMY(int* pc, int* pm, int* py) const
{
    UByte r,g,b;
    GetRGB(&r,&g,&b);
    *pc = 255 - r;
    *pm = 255 - g;
    *py = 255 - b;
}


/*
The YIQ system is the color primary system adopted by National Television System Committee (NTSC) 
for color TV broadcasting. The YIQ color solid is made by a linear transformation of the RGB cube. 
Its purpose is to exploit certain characteristics of the human eye to maximize the utilization of 
a fixed bandwidth. The human visual system is more sensitive to changes in luminance than to changes 
in hue or saturation, and thus a wider bandwidth should be dedicated to luminance than to color 
information. Y is similar to perceived luminance, I and Q carry color information and some luminance
information. The Y signal usually has 4.2 MHz bandwidth in a 525 line system. Originally, the I and Q 
had different bandwidths (1.5 and 0.6 MHz), but now they commonly have the same bandwidth of 1 MHz. 

The RGB to YIQ conversion is as follows:

    [ Y ]     [ 0.299   0.587   0.114 ] [ R ]
    [ I ]  =  [ 0.596  -0.275  -0.321 ] [ G ]
    [ Q ]     [ 0.212  -0.523   0.311 ] [ B ]

The YIQ to RGB conversion is as follows:

    [ R ]     [ 1.000   0.956   0.621 ] [ Y ]
    [ G ]  =  [ 1.000  -0.272  -0.647 ] [ I ]
    [ B ]     [ 1.000  -1.105   1.702 ] [ Q ]

The description above is an excerpt from "A Guided Tour of Color Space" by Charles Poynton; 
available online at http://www.poynton.com/PDFs/Guided_tour.pdf and "Introduction to Color" 
by Eugene Vishnevsky; http://www.cs.rit.edu/~ncs/color.
*/

// YIQ
SF_EXPORT void          Color::SetYIQ(float y, float i, float q)
{
    float r, g, b;
    r   = y +  0.956F * i +  0.621F * q;
    g   = y + -0.272F * i + -0.647F * q;
    b   = y + -1.105F * i +  1.702F * q;
    SetRGBFloat(r, g, b);
}

SF_EXPORT void          Color::GetYIQ(float *py, float *pi, float *pq) const
{
    float r, g, b;
    GetRGBFloat(&r, &g, &b);
    *py = 0.299F * r +  0.587F * g +  0.114F * b;
    *pi = 0.596F * r + -0.275F * g + -0.321F * b;
    *pq = 0.212F * r + -0.523F * g +  0.311F * b;
}



/*
YUV is used in European TVs and YIQ in North American TVs (NTSC).
Y is linked to the component of luminancy, and U,V and I,Q are linked
to the components of chrominancy. Y came from the standard CIE 1931 XYZ.

YUV uses D65 white point which coordinates are (xn; yn) = (0.312713; 0.329016).
The RGB chromacity coordinates are:

Red:   xr=0.64 yr=0.33
Green: xg=0.29 yg=0.60
Blue:  xb=0.15 yb=0.06

The RGB to YUV conversion is as follows:

    [ Y ]     [  0.299   0.587   0.114 ] [ R ]
    [ U ]  =  [ -0.147  -0.289   0.436 ] [ G ]
    [ V ]     [  0.615  -0.515  -0.100 ] [ B ]

The YUV to RGB conversion is as follows:

    [ R ]     [  1.000   0.000   1.140 ] [ Y ]
    [ G ]  =  [  1.000  -0.396  -0.581 ] [ U ]
    [ B ]     [  1.000   2.029   0.000 ] [ V ]

The description above is an excerpt from "A Guided Tour of Color Space" by Charles Poynton; 
available online at http://www.poynton.com/PDFs/Guided_tour.pdf and "Introduction to Color" 
by Eugene Vishnevsky; http://www.cs.rit.edu/~ncs/color.
*/

SF_EXPORT void          Color::SetYUV(float y, float u, float v)
{
    float r, g, b;
    r   = y +  0.000F * u +  1.140F * v;
    g   = y + -0.396F * u + -0.581F * v;
    b   = y +  2.029F * u +  0.000F * v;
    SetRGBFloat(r, g, b);
}

SF_EXPORT void          Color::GetYUV(float *py, float *pu, float *pv) const
{
    float r, g, b;
    GetRGBFloat(&r, &g, &b);
    *py =  0.299F * r +  0.587F * g +  0.114F * b;
    *pu = -0.147F * r + -0.289F * g +  0.436F * b;
    *pv =  0.615F * r + -0.515F * g + -0.100F * b;
}



// CIE XYZ
/*
RGB values in a particular set of primaries can be transformed to and from CIE XYZ via 
a 3x3 matrix transform. These transforms involve tristimulus values, that is a set of 
three linear-light components that conform to the CIE color-matching functions. CIE XYZ is 
a special set of tristimulus values. In XYZ, any color is represented as a set of positive values.

To transform from XYZ to RGB (with D65 white point), the matrix transform used is [3]:

   [ R ]   [  3.240479 -1.537150 -0.498535 ]   [ X ]
   [ G ] = [ -0.969256  1.875992  0.041556 ] * [ Y ]
   [ B ]   [  0.055648 -0.204043  1.057311 ]   [ Z ]

The range for valid R, G, B values is [0,1]. Note, this matrix has negative coefficients. Some 
XYZ color may be transformed to RGB values that are negative or greater than one. This means 
that not all visible colors can be produced using the RGB system.

The inverse transformation matrix is as follows:

   [ X ]   [  0.412453  0.357580  0.180423 ]   [ R ]
   [ Y ] = [  0.212671  0.715160  0.072169 ] * [ G ]
   [ Z ]   [  0.019334  0.119193  0.950227 ]   [ B ]

*/

SF_EXPORT void          Color::SetXYZ(float x, float y, float z)
{
    float   r, g, b;
    float   k = 1.088751F;

    r   =  3.240479F * x + -1.537150F * y + -0.498535F * z * k;
    g   = -0.969256F * x +  1.875992F * y +  0.041556F * z * k;
    b   =  0.055648F * x + -0.204043F * y +  1.057311F * z * k;
    SetRGBFloat(r, g, b);
}

SF_EXPORT void          Color::GetXYZ(float *px, float *py, float *pz) const
{
    float r, g, b;
    GetRGBFloat(&r, &g, &b);
    *px =  (0.412453F * r +  0.357580F * g +  0.180423F * b);
    *py =  (0.212671F * r +  0.715160F * g +  0.072169F * b);
    *pz =  (0.019334F * r +  0.119193F * g +  0.950227F * b) * 0.918483657F; //
}

// CIE Lab
/*
CIE 1976 L*a*b* is based directly on CIE XYZ and is an attempt to linearize the perceptibility 
of color differences. The non-linear relations for L*, a*, and b* are intended to mimic the 
logarithmic response of the eye. Coloring information is referred to the color of the white 
point of the system, subscript n.

    L* = 116 * (Y/Yn)1/3 - 16    for Y/Yn > 0.008856
    L* = 903.3 * Y/Yn             otherwise

    a* = 500 * ( f(X/Xn) - f(Y/Yn) )
    b* = 200 * ( f(Y/Yn) - f(Z/Zn) )
        where f(t) = t1/3      for t > 0.008856
              f(t) = 7.787 * t + 16/116    otherwise

Here Xn, Yn and Zn are the tristimulus values of the reference white.
The reverse transformation (for Y/Yn > 0.008856) is

    X = Xn * ( P + a* / 500 ) 3
    Y = Yn * P 3
    Z = Zn * ( P - b* / 200 ) 3
        where P = (L* + 16) / 116
*/

void            Color::SetLab(float l, float a, float b)
{
    Double x,y,z;
    ConvertLabToXYZ(Double(l),Double(a),Double(b),&x,&y,&z);
    SetXYZ(float(x),float(y),float(z));
}

void            Color::GetLab(float *pl, float *pa, float *pb) const
{
    float   x,y,z;
    Double  l,a,b;
    GetXYZ(&x,&y,&z);
    ConvertXYZToLab(x,y,z,&l,&a,&b);
    *pl = float(l);
    *pa = float(a);
    *pb = float(b);
}


// ** CIE Luv
/*
CIE 1976 L*u*v* (CIELUV) is based directly on CIE XYZ and is another attempt to linearize
the perceptibility of color differences. 

  The non-linear relations for L*, u*, and v* are given below:
  
    L* =  116 * (Y/Yn)1/3 - 16
    u* =  13L* * ( u' - un' )
    v* =  13L* * ( v' - vn' )
    
The quantities un' and vn'  refer to the reference white or the light source; for 
the 2� observer and illuminant C,  un' = 0.2009, vn' = 0.4610 [ 1 ]. 
  
Equations for u' and v' are given below:

    u' = 4X / (X + 15Y + 3Z) = 4x / ( -2x + 12y + 3 )
    v' = 9Y / (X + 15Y + 3Z) = 9y / ( -2x + 12y + 3 ).

The transformation from (u',v') to (x,y) is:

    x = 27u' / ( 18u' - 48v' + 36 )
    y = 12v' / ( 18u' - 48v' + 36 ).

The transformation from CIELUV to XYZ is performed as following:

    u' = u / ( 13L*) + un
    v' = v / ( 13L* ) + vn
    Y = (( L* + 16 ) / 116 )3
    X = - 9Yu' / (( u' - 4 ) v' - u'v' )
    Z = ( 9Y - 15v'Y - v'X ) / 3v'
*/

void            Color::SetLuv(float l, float u, float v)
{
    Double x,y,z;
    ConvertLuvToXYZ(l,u,v,&x,&y,&z);
    SetXYZ(float(x),float(y),float(z));
}

void            Color::GetLuv(float *pl, float *pu, float *pv) const
{
    float   x,y,z;
    Double  l,u,v;
    GetXYZ(&x,&y,&z);
    ConvertXYZToLuv(x,y,z,&l,&u,&v);
    *pl = float(l);
    *pu = float(u);
    *pv = float(v);
}


Double  Color_Cube(Double v)
{
    if (v >= (6 / 29))
        return v * v * v;
    else
        return (108 / 841) * (v - (4 / 29));
}

// Lab - XYZ conversion
void        Color::ConvertLabToXYZ(Double l, Double a, Double b,
                                   Double *px, Double *py, Double *pz)
{
    Double whitePointX = 0.95;
    Double whitePointY = 1.00;
    Double whitePointZ = 1.09;

    Double ll = (l + 16) / 116;
    *px = whitePointX * Color_Cube(ll + a / 500);
    *py = whitePointY * Color_Cube(ll);
    *pz = whitePointZ * Color_Cube(ll - b / 200);

    /*
    Double tY = ( l + 16 ) / 116.0;
    Double tX = a / 500.0 + tY;
    Double tZ = tY - b / 200.0;

    Double y3 = tY*tY*tY;
    Double x3 = tX*tX*tX;
    Double z3 = tZ*tZ*tZ;

    if ( y3 > 0.008856 ) tY = y3;
    else                 tY = ( tY - 16 / 116.0 ) / 7.787;
    if ( x3 > 0.008856 ) tX = x3;
    else                 tX = ( tX - 16 / 116.0 ) / 7.787;
    if ( z3 > 0.008856 ) tZ = z3;
    else                 tZ = ( tZ - 16 / 116.0 ) / 7.787;

    // Observer= 2�, Illuminant= D65
    *px =  95.047 * tX;
    *py = 100.000 * tY;
    *pz = 108.883 * tZ;
    */
}

Double Color_CubeRoot(Double v)
{
    if (v >= (216.0/24389.0))
        return pow(v, (1.0/3.0));
    else
        return (841.0/108.0) * v + (4.0/29.0);
}

void        Color::ConvertXYZToLab(Double x, Double y, Double z,
                                   Double *pl, Double *pa, Double *pb)
{
    Double whitePointX = 0.95;
    Double whitePointY = 1.00;
    Double whitePointZ = 1.09;
    
    Double  yy = Color_CubeRoot(y / whitePointY);
    *pl = 116.0 * yy - 16;
    *pa = 500.0 * (Color_CubeRoot(x / whitePointX) - yy);
    *pb = 200.0 * (yy - Color_CubeRoot(z / whitePointZ));

    /*
    // Observer = 2�, Illuminant = D65
    Double tX = x /  95.047;
    Double tY = y / 100.000;
    Double tZ = z / 108.883;

    if ( tX > 0.008856 ) tX = pow(tX, 1.0/3.0);
    else                 tX = ( 7.787 * tX ) + ( 16 / 116 );
    if ( tY > 0.008856 ) tY = pow(tY, 1.0/3.0);
    else                 tY = ( 7.787 * tY ) + ( 16 / 116 );
    if ( tZ > 0.008856 ) tZ = pow(tZ, 1.0/3.0);
    else                 tZ = ( 7.787 * tZ ) + ( 16 / 116 );

    *pl = ( 116 * tY ) - 16;
    *pa = 500 * ( tX - tY );
    *pb = 200 * ( tY - tZ );

    // ** Version #2
    Double  x0, y0, z0;
    Double  x1, y1, z1;
  
    x0 = 0.98072;
    y0 = 1.00000;
    z0 = 1.18225;
    
    if ((x / x0) > 0.008856)  x1 = pow ((x / x0), (1.0 / 3.0));
    else                      x1 = (7.787 * (x / x0)) + (16.0 / 116.0);
    if ((y / y0) > 0.008856)  y1 = pow ((y / y0), (1.0 / 3.0));
    else                      y1 = (7.787 * (y / y0)) + (16.0 / 116.0);
    if ((z / z0) > 0.008856)  z1 = pow ((z / z0), (1.0 / 3.0));
    else                      z1 = (7.787 * (z / z0)) + (16.0 / 116.0);
    
    *pl = (116.0 * y1) - 16.0;
    *pa = 504.3 * (x1 - y1);
    *pb = 201.7 * (y1 - z1);
    */
}

// Luv - XYZ conversion
void        Color::ConvertLuvToXYZ(Double l, Double u, Double v,
                                   Double *px, Double *py, Double *pz)
{
    Double tY = ( l + 16 ) / 116.0;
    Double y3 = tY*tY*tY;
    if ( y3 > 0.008856 ) tY = y3;
    else                 tY = ( tY - 16 / 116.0 ) / 7.787;

    // Observer= 2�, Illuminant= D65
    Double rX =  95.047;     
    Double rY = 100.000;
    Double rZ = 108.883;

    Double rU = ( 4 * rX ) / ( rX + ( 15 * rY ) + ( 3 * rZ ) );
    Double rV = ( 9 * rY ) / ( rX + ( 15 * rY ) + ( 3 * rZ ) );

    Double tU = u / ( 13.0 * l ) + rU;
    Double tV = v / ( 13.0 * l ) + rV;

    *py = tY * 100;
    *px =  - ( 9 * (*py) * tU ) / ( ( tU - 4 ) * tV  - tU * tV );
    *pz = ( 9 * (*py) - ( 15 * tV * (*py) ) - ( tV * (*px) ) ) / ( 3 * tV );

    
    /*
    Adopted from the Horus library
    Double Y = L2Y(v.x());

    Double tmp = Xn + 15 * Yn + 3 * Zn;
    Double unp = 4 * Xn / tmp;
    Double vnp = 9 * Yn / tmp;
    Double Q = v.y() / (13 * v.x()) + unp;
    Double R = v.z() / (13 * v.x()) + vnp;
    Double A = 3 * Y * (5 * R - 3);
    Double Z = ((Q - 4) * A - 15 * Q * R * Y) / (12 * R);
    Double X = -(A / R + 3 * Z);

    *px = X;
    *py = Y;
    *pz = Z;
    */
}

void        Color::ConvertXYZToLuv(Double x, Double y, Double z,
                                   Double *pl, Double *pu, Double *pv)
{
    Double tU = ( 4 * x ) / ( x + ( 15 * y ) + ( 3 * z ) );
    Double tV = ( 9 * y ) / ( x + ( 15 * y ) + ( 3 * z ) );

    Double tY = y / 100;
    if ( tY > 0.008856 ) tY = pow(tY, (1.0/3.0));
    else                 tY = ( 7.787 * tY ) + ( 16.0 / 116.0 );

    // Observer= 2�, Illuminant= D65
    Double rX =  95.047;
    Double rY = 100.000;
    Double rZ = 108.883;

    Double rU = ( 4 * rX ) / ( rX + ( 15 * rY ) + ( 3 * rZ ) );
    Double rV = ( 9 * rY ) / ( rX + ( 15 * rY ) + ( 3 * rZ ) );

    *pl = ( 116 * tY ) - 16;
    *pu = 13 * (*pl) * ( tU - rU );
    *pv = 13 * (*pl) * ( tV - rV );


    /*
    Double  x0, y0, z0;
    Double  u0, v0;
    Double  y1, u1, v1;
  
    x0 = 0.98072;
    y0 = 1.00000;
    z0 = 1.18225;
  
    if ((y / y0) > 0.008856)  
        y1 = pow((y/y0), (1.0/3.0));
    else
        y1 = (7.787 * (y / y0)) + (16.0 / 116.0);
  
    (*pl) = (116.0 * y1) - 16.0;
  
    if ((x + (15.0 * y) + (3.0 * z)) == 0.0) 
    {
        (*pu) = 0;
        (*pv) = 0;
    } 
    else 
    {
        u0 = (4.0 * x0) / (x0 + (15.0 * y0) + (3.0 * z0));
        v0 = (9.0 * y0) / (x0 + (15.0 * y0) + (3.0 * z0));
        u1 = (4.0 * x)  / (x  + (15.0 * y)  + (3.0 * z));
        v1 = (9.0 * y)  / (x  + (15.0 * y)  + (3.0 * z));
        (*pu) = 13.0 * (*pl) * (u1 - u0);
        (*pv) = 13.0 * (*pl) * (v1 - v0);
    }
    */

    /*
    Adopted from the Horus library
    Double L = Y2L(v.y() / Yn);

    Double tmp = Xn + 15 * Yn + 3 * Zn;
    Double unp = 4 * Xn / tmp;
    Double vnp = 9 * Yn / tmp;
    tmp = v.x() + 15 * v.y() + 3 * v.z();
    Double up = 4 * v.x() / tmp;
    Double vp = 9 * v.y() / tmp;
    Double us = 13 * L * (up - unp);
    Double vs = 13 * L * (vp - vnp);

    *pl = L;
    *pu = us;
    *pv = vs;
    */
}

/*
// OOO (Geusebroek Thesis) - RGB conversion
void        Color::ConvertRGBToOOO(Double r, Double g, Double b,
                                   Double *po, Double *po1, Double *po11)
{
    *po   = r * -0.004362 + g *  0.010954 + b *  0.003408;
    *po1  = r *  0.004055 + g *  0.001220 + b * -0.004120;
    *po11 = r *  0.011328 + g * -0.011755 + b * -0.000664;
}

void        Color::ConvertOOOToRGB(Double o, Double o1, Double o11,
                                   Double *pr, Double *pg, Double *pb)
{
    *pr = o * 328.084 + o1 *  469.182 + o11 *  687.853;
    *pg = o * 199.794 + o1 *  172.242 + o11 * -202.904;
    *pb = o * 314.533 + o1 * -441.212 + o11 *  291.574;
}
*/

#endif // SF_NO_COLOR_COMPLEX_CONVERSION


}} // Scaleform::Render
