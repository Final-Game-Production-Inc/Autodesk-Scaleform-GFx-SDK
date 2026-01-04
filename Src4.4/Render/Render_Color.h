/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Color.h
Content     :   RGBA Color class
Created     :   March 26, 1999
Authors     :   Michael Antonov, Brendan Iribe

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Color_H
#define INC_SF_Render_Color_H

#include "Kernel/SF_Alg.h"

namespace Scaleform { namespace Render {


// *** Default 32 bit color functions

#define SF_RGB32_RED_MASK              0x00FF0000
#define SF_RGB32_GREEN_MASK            0x0000FF00
#define SF_RGB32_BLUE_MASK             0x000000FF
#define SF_RGB32_ALPHA_MASK            0xFF000000
#define SF_RGB32_RGB_MASK              (SF_RGB32_RED_MASK|SF_RGB32_GREEN_MASK|SF_RGB32_BLUE_MASK)

#define SF_RGB32_RED(color)            (UByte)(((color)&SF_RGB32_RED_MASK)>>16)
#define SF_RGB32_GREEN(color)          (UByte)(((color)&SF_RGB32_GREEN_MASK)>>8)
#define SF_RGB32_BLUE(color)           (UByte)((color)&SF_RGB32_BLUE_MASK)
#define SF_RGB32_ALPHA(color)          (UByte)((color)>>24)

#define SF_RGB32_SET_RED(color, r)     (UInt32)(((color)&~SF_RGB32_RED_MASK)|((r)<<16))
#define SF_RGB32_SET_GREEN(color, g)   (UInt32)(((color)&~SF_RGB32_GREEN_MASK)|((g)<<8))
#define SF_RGB32_SET_BLUE(color, b)    (UInt32)(((color)&~SF_RGB32_BLUE_MASK)|(b))
#define SF_RGB32_SET_ALPHA(color, a)   (UInt32)(((color)&~SF_RGB32_ALPHA_MASK)|((a)<<24))

// *** Short 32bit color defines
#define SF_RGB(r,g,b)                  (UInt32(((UInt32)b)|(((UInt32)g)<<8)|(((UInt32)r)<<16)))
#define SF_RGBA(r,g,b,a)               (UInt32(SF_RGB(r,g,b)|(((UInt32)a)<<24)))


// GFx has no need for complex color models by default
#define SF_NO_COLOR_MODEL_CONVERSION
#define SF_NO_COLOR_COMPLEX_CONVERSION

// The luminance vector values for red, green, and blue
// - to convert a colors into black and white, the balance 
//   coefficient of 0.3086, 0.6094, and 0.0820 is used. 
// - the standard NTSC weights of 0.299, 0.587, and 0.114 are
//   not used, as they are only applicable to RGB colors in a 
//   gamma 2.2 color space. 
#define SF_COLOR_LUM_RED       0.3086
#define SF_COLOR_LUM_GREEN     0.6094
#define SF_COLOR_LUM_BLUE      0.0820
// Fixed point version must be divided by 32768
// - NTSC weights of 9798, 19235, 3735 are not used
#define SF_COLOR_ILUM_RED      10112L
#define SF_COLOR_ILUM_GREEN    19969L
#define SF_COLOR_ILUM_BLUE     2687L


// ***** Color class

class Color
{   
public:

    // Common RGB color values
    // * indicates the color name has a different value
    //   than the ColorValues version of the same color name
    enum StandardColors 
    {
        // Gray scale
        Black                   =SF_RGB(  0,  0,  0),     // 0x00000000
        White                   =SF_RGB(255,255,255),     // 0x00FFFFFF
        VeryLightGray           =SF_RGB(224,224,224),     // 0x00E0E0E0
        LightGray               =SF_RGB(192,192,192),     // 0x00C0C0C0   *
        Gray                    =SF_RGB(128,128,128),     // 0x00808080   *
        DarkGray                =SF_RGB( 64, 64, 64),     // 0x00404040   *
        VeryDarkGray            =SF_RGB( 32, 32, 32),     // 0x00202020
                                                            
        // RGB                                              
        Red                     =SF_RGB(255,  0,  0),     // 0x00FF0000
        LightRed                =SF_RGB(255,128,128),     // 0x00FF8080
        DarkRed                 =SF_RGB(128,  0,  0),     // 0x00800000   *
        VeryDarkRed             =SF_RGB( 64,  0,  0),     // 0x00400000
        Green                   =SF_RGB(  0,255,  0),     // 0x0000FF00
        LightGreen              =SF_RGB(128,255,128),     // 0x0080FF80   *
        DarkGreen               =SF_RGB(  0,128,  0),     // 0x00008000   *
        VeryDarkGreen           =SF_RGB(  0, 64,  0),     // 0x00004000
        Blue                    =SF_RGB(  0,  0,255),     // 0x000000FF
        LightBlue               =SF_RGB(128,128,255),     // 0x008080FF   *
        DarkBlue                =SF_RGB(  0,  0,128),     // 0x00000080   *
        VeryDarkBlue            =SF_RGB(  0,  0, 64),     // 0x00000040
        
        // CMY                  
        Cyan                    =SF_RGB(  0,255,255),     // 0x0000FFFF
        LightCyan               =SF_RGB(128,255,255),     // 0x0080FFFF   *
        DarkCyan                =SF_RGB(  0,128,128),     // 0x00008080   *
        Magenta                 =SF_RGB(255,  0,255),     // 0x00FF00FF
        LightMagenta            =SF_RGB(255,128,255),     // 0x00FF80FF
        DarkMagenta             =SF_RGB(128,  0,128),     // 0x00800080   *
        Yellow                  =SF_RGB(255,255,  0),     // 0x00FFFF00
        LightYellow             =SF_RGB(255,255,128),     // 0x00FFFF80   *
        DarkYellow              =SF_RGB(128,128,  0),     // 0x00808000
                                                            
        // Extended RGB color values                        
        Purple                  =SF_RGB(255,  0,255),     // 0x00FF00FF   *
        DarkPurple              =SF_RGB(128,  0,128),     // 0x00800080
        Pink                    =SF_RGB(255,192,192),     // 0x00FFC0C0   *
        DarkPink                =SF_RGB(192,128,128),     // 0x00C08080
        Beige                   =SF_RGB(255,192,128),     // 0x00FFC080   *
        LightBeige              =SF_RGB(255,224,192),     // 0x00FFE0C0
        DarkBeige               =SF_RGB(192,128, 64),     // 0x00C08040
        Orange                  =SF_RGB(255,128,  0),     // 0x0080FF00   *
        Brown                   =SF_RGB(128, 64,  0),     // 0x00804000   *
        LightBrown              =SF_RGB(192, 96,  0),     // 0x00C06000
        DarkBrown               =SF_RGB( 64, 32,  0)      // 0x00402000
    };
    
    // Common Alpha color values
    enum StandardAlphas 
    {
        Alpha0                  =SF_RGBA(0,0,0,  0),
        Alpha25                 =SF_RGBA(0,0,0, 64),
        Alpha50                 =SF_RGBA(0,0,0,127),
        Alpha75                 =SF_RGBA(0,0,0,191),
        Alpha100                =SF_RGBA(0,0,0,255)
    };

    // 32bit Color Structure
    struct Rgb32
    {
    #if SF_BYTE_ORDER==SF_LITTLE_ENDIAN
        UByte   Blue;
        UByte   Green;
        UByte   Red;
        UByte   Alpha;
    #else
        UByte   Alpha;
        UByte   Red;
        UByte   Green;
        UByte   Blue;
    #endif
    };

    // 32bit Color Data 
    union
    {
        Rgb32   Channels;
        UInt32  Raw;
    };
    
    // Constructors
    inline Color()                                                            { }
    inline Color(const Color &c)                                              { SetColor(c); }
    inline Color(const Color &c, UByte a8)                                    { SetColor(c, a8); }
    inline Color(const Color &c, Color ac)                                    { SetColor(c, ac); }
    inline Color(UInt32 raw32)                                                { SetColor(raw32); }
    inline Color(UInt32 raw32, UByte a8)                                      { SetColor(raw32, a8); }
    inline Color(UByte red, UByte green, UByte blue, UByte a8=0)              { SetColor(red, green, blue, a8); } 

    
    // Add formatting to support GFx debugging output
    // Buffer must be at least 32 characters in size
    //SF_EXPORT void Format (char *pbuffer) const;

    
#ifndef SF_NO_COLOR_MODEL_CONVERSION

    // Color model
    // - Xyz, Lab, Luv and Yiq all require floating point conversion
    enum Model  { Rgb, Hsv, Hsi
#ifndef SF_NO_COLOR_COMPLEX_CONVERSION
        , Cmy, Xyz, Lab, Luv, Yiq, Yuv
#endif
        };
    
    // When initializing the color with integer values and 
    // the following models: Xyz, Lab, Luv or Yiq the RGB range is (0-100)
    // - the floating point methods should be used with those models
    // - the alpha value for integer methods is always (0-255)
    SF_EXPORT Color(int x, int y, int z, Model model);
    SF_EXPORT Color(int x, int y, int z, int a8, Model model);
    // All floating point values must be between (0.0-1.0)
    SF_EXPORT Color(Double x, Double y, Double z, Model model);
    SF_EXPORT Color(Double x, Double y, Double z, Double aD, Model model);

#endif // SF_NO_COLOR_MODEL_CONVERSION

    
    // Initialize the color values

    inline void            SetColor(const Color &c)                            { Raw = c.Raw; }
    inline void            SetColor(const Color &c, UByte a8)                  { SetColor(c.Raw,a8); }
    inline void            SetColor(const Color &c, Color ac)                  { SetColor(c.Raw,ac.GetAlpha()); }
    inline void            SetColor(UInt32 raw)                                { Raw = raw; }
    inline void            SetColor(UInt32 raw, UByte a8)                      { Raw = raw; SetAlpha(a8); }
    inline void            SetColor(UByte r, UByte g, UByte b, UByte a=0)      { SetRGBA(r, g, b, a); }

    inline void            SetColorRGB  (const Color &c)                       { Raw = UInt32((Raw&SF_RGB32_ALPHA_MASK)|(c.Raw&SF_RGB32_RGB_MASK)); }
    inline void            SetColorAlpha(const Color &c)                       { SetAlpha(c.GetAlpha()); }

    // Return colors
    inline Color           GetColorRGB     () const                 { return Color(Raw&SF_RGB32_RGB_MASK);   }
    inline Color           GetColorAlpha   () const                 { return Color(Raw&SF_RGB32_ALPHA_MASK); }
    inline Color           GetColorGray    () const;
    
    // *** Conversion
    inline UInt32          ToColor32       () const                 { return Raw; }    
    inline static Color    FromColor32     (const UInt32 c)         { return Color(c); }

    inline static Color    FromRed         (UByte r)                { return Color(r,0,0,0);   }
    inline static Color    FromGreen       (UByte g)                { return Color(0,g,0,0);   }
    inline static Color    FromBlue        (UByte b)                { return Color(0,0,b,0);   }
    inline static Color    FromAlpha       (UByte a)                { return Color(0,0,0,a);   }

    // RGB Color
    inline void            SetRGB (UByte r, UByte g, UByte b);
    inline void            GetRGB (UByte *pr, UByte *pg, UByte *pb) const;
    inline void            SetRGBA(UByte r, UByte g, UByte b, UByte a);                                
    inline void            GetRGBA(UByte *pr, UByte *pg, UByte *pb, UByte *pa) const;

    // RGB Color - floating point
    inline void            SetRGBFloat  (float r, float g, float b);
    inline void            GetRGBFloat  (float *pr, float *pg, float *pb) const;
    inline void            GetRGBFloat  (float *prgb) const;
    inline void            SetRGBAFloat (float r, float g, float b, float a);  
    inline void            GetRGBAFloat (float *pr, float *pg, float *pb, float *pa) const;
    inline void            GetRGBAFloat (float *prgba) const;
    inline void            SetAlphaFloat(float a);
    inline void            GetAlphaFloat(float *pa) const;

    // Access RGB channels
    inline UByte           GetAlpha() const        { return Channels.Alpha; }
    inline UByte           GetRed  () const        { return Channels.Red;  }
    inline UByte           GetGreen() const        { return Channels.Green; }
    inline UByte           GetBlue () const        { return Channels.Blue; }
    // Initialize RGB channels
    inline void            SetAlpha(UByte a)       { Channels.Alpha = a; }
    inline void            SetRed  (UByte r)       { Channels.Red   = r; }
    inline void            SetGreen(UByte g)       { Channels.Green = g; }
    inline void            SetBlue (UByte b)       { Channels.Blue  = b; }

    // Grayscale 
    inline UByte           GetGray () const;
    inline void            SetGray (UByte g)       { SetRGB(g,g,g); }


    // *** Color operations
    // - Standard functions operate on both RGB and Alpha
    // - RGB functions only deal with the RGB channels and return null alpha value    
    
    // ** Binary ops
    
    // XOr - (c1 ^ c2)
    inline static UByte    XOrCh               (unsigned c1, unsigned c2);
    inline static Color    XOr                 (Color c1, Color c2);
    inline static Color    XOrRGB              (Color c1, Color c2);    
    inline Color&          XOr                 (Color c)                      { SetColor(XOr(*this, c)); return *this; }
    inline Color&          XOrRGB              (Color c)                      { SetColor(XOrRGB(*this, c)); return *this; }       
                                                                                
    // Or - (c1 | c2)                                                           
    inline static UByte    OrCh                (unsigned c1, unsigned c2);
    inline static Color    Or                  (Color c1, Color c2);         
    inline static Color    OrRGB               (Color c1, Color c2);         
    inline Color&          Or                  (Color c)                      { SetColor(Or(*this, c)); return *this; }
    inline Color&          OrRGB               (Color c)                      { SetColor(OrRGB(*this, c)); return *this; }    
                                                                                
    // And - (c1 & c2)                          
    inline static UByte    AndCh               (unsigned c1, unsigned c2);
    inline static Color    And                 (Color c1, Color c2);         
    inline static Color    AndRGB              (Color c1, Color c2);         
    inline Color&          And                 (Color c)                      { SetColor(And(*this, c)); return *this; }
    inline Color&          AndRGB              (Color c)                      { SetColor(AndRGB(*this, c)); return *this; }   
                                                                       
                                                                                
    // ** Color blending ops                                                    
                                                                                
    // Modulate - (c1 * c2)
    // - equation: ((c1+1)*c2)>>8
    inline static UByte    ModulateCh          (unsigned c1, unsigned c2);
    inline static Color    Modulate            (Color c1, Color c2);         
    inline static Color    ModulateRGB         (Color c1, Color c2);         
    inline Color&          Modulate            (Color c)                      { SetColor(Modulate(*this, c)); return *this; }
    inline Color&          ModulateRGB         (Color c)                      { SetColor(ModulateRGB(*this, c)); return *this; }  
                                                                            
    // ModulateFactor - (c1 * factor)
    // - equation: min(c1*(f+1), 255)
    inline static UByte    ModulateFactorCh    (unsigned c1,  UByte factor);
    inline static Color    ModulateFactor      (Color c1, UByte factor);
    inline static Color    ModulateFactorRGB   (Color c1, UByte factor);
    inline Color&          ModulateFactor      (UByte factor)                  { SetColor(ModulateFactor(*this, factor)); return *this; }
    inline Color&          ModulateFactorRGB   (UByte factor)                  { SetColor(ModulateFactorRGB(*this, factor)); return *this; }   
                                                                            
    // Add - (c1 + c2)
    // - equation: min(c1+c2, 255)
    inline static UByte    AddCh               (unsigned c1, unsigned c2);
    inline static Color    Add                 (Color c1, Color c2);         
    inline static Color    AddRGB              (Color c1, Color c2);         
    inline Color&          Add                 (Color c)                      { SetColor(Add(*this, c)); return *this; }
    inline Color&          AddRGB              (Color c)                      { SetColor(AddRGB(*this, c)); return *this; }   
                                                             
    // Subtract - (c1 - c2)
    // - equation: max(c1-c2, 0)
    inline static UByte    SubtractCh          (unsigned c1, unsigned c2);
    inline static Color    Subtract            (Color c1, Color c2);         
    inline static Color    SubtractRGB         (Color c1, Color c2);         
    inline Color&          Subtract            (Color c)                      { SetColor(Subtract(*this, c)); return *this; }
    inline Color&          SubtractRGB         (Color c)                      { SetColor(SubtractRGB(*this, c)); return *this; }  
                                                       
    // Blend - (c2 * (1-f) + c1 * f)
    // - equation: (((c1+1)*f)>>8)+(((c2+1)*(255-f))>>8)
    // - treats the second parameter as dest
    inline static UByte    BlendCh             (unsigned c1, unsigned c2, UByte factor);
    inline static Color    Blend               (Color c1, Color c2, UByte factor);
    inline static Color    BlendRGB            (Color c1, Color c2, UByte factor);
    inline Color&          Blend               (Color c, UByte factor)        { SetColor(Blend(*this, c, factor)); return *this; }
    inline Color&          BlendRGB            (Color c, UByte factor)        { SetColor(BlendRGB(*this, c, factor)); return *this; } 
    // Special blend version with floating-point factor
    SF_EXPORT static Color SF_STDCALL Blend    (Color c1, Color c2, float factor);
    
    // Inverse (1-c)
    // - equation: (255-c)
    inline static UByte    InverseCh           ( unsigned c );
    inline static Color    Inverse             ( Color c );
    inline static Color    InverseRGB          ( Color c );
    inline Color&          Inverse             (  )                            { SetColor(Inverse(*this)); return *this; }
    inline Color&          InverseRGB          (  )                            { SetColor(InverseRGB(*this)); return *this; }
    

    // *** Operator overrides

    // Equality
    inline bool            operator == (Color c) const        { return Raw == c.Raw; }
    inline bool            operator != (Color c) const        { return Raw != c.Raw; }


    inline Color          operator ~  () const                { return Color(~Raw); }

    // Assignment
    inline const Color&   operator =  (const Color &c)        { Raw = c.Raw; return *this; }

    // Binary operations
    inline const Color&   operator ^= (Color c)               { return XOr(c); }
    inline const Color&   operator |= (Color c)               { return Or (c); }
    inline const Color&   operator &= (Color c)               { return And(c); }

    // Advanced operations                          
    inline const Color&   operator += (Color c)               { return Add(c); }
    inline const Color&   operator -= (Color c)               { return Subtract(c); }
    inline const Color&   operator *= (Color c)               { return Modulate(c); }
    inline const Color&   operator *= (UByte factor)          { return Modulate(factor); }
    
    // Operator overrides
    // - call color operations
    inline friend const Color operator ^  (Color c1, Color c2);
    inline friend const Color operator |  (Color c1, Color c2);
    inline friend const Color operator &  (Color c1, Color c2);

    inline friend const Color operator +  (Color c1, Color c2);
    inline friend const Color operator -  (Color c1, Color c2);
    inline friend const Color operator *  (Color c1, Color c2);
    inline friend const Color operator *  (Color c1, UByte factor);


    // ** Additional color information

    // Calculate the distance between the local color and the passed color
    inline int             DistanceSquared(Color c) const;
    
    // Calculate the intensity of the color
    inline unsigned        IIntensity() const;
    inline float           FIntensity() const;

    
    // ** HSV color conversions

#ifndef SF_NO_COLOR_HSV_CONVERSION

    // HSV - Hue, Saturation and Value
    // int parameter ranges are h (0-360) s(0-255) v(0-255)
    // float parameter ranges are all (0.0-1.0)
    SF_EXPORT void            SetHSV(int h, int s, int v);
    SF_EXPORT void            GetHSV(int* ph, int* ps, int* pv) const;
    SF_EXPORT void            SetHSV(float h, float s, float v);
    SF_EXPORT void            GetHSV(float *ph, float *ps, float *pv) const;
    // HSI - Hue, Saturation and Intensity
    // int parameter ranges are h (0-360) s(0-255) i(0-255)
    // float parameter ranges are all (0.0-1.0)
    SF_EXPORT void            SetHSI(int h, int s, int i);
    SF_EXPORT void            GetHSI(int* ph, int* ps, int* pi) const;
    SF_EXPORT void            SetHSI(float h, float s, float i);
    SF_EXPORT void            GetHSI(float *ph, float *ps, float *pi) const;

    // Precise HSI - RGB conversion using trig functions
    SF_EXPORT static void SF_STDCALL    ConvertHSIToRGB(Double h, Double s, Double i,
                                                        Double *pr, Double *pg, Double *pb);
    SF_EXPORT static void SF_STDCALL    ConvertRGBToHSI(Double r, Double g, Double b,
                                                        Double *ph, Double *ps, Double *pi);

#endif // SF_NO_COLOR_HSV_CONVERSION
    

    // ** Additional color conversions

#ifndef SF_NO_COLOR_COMPLEX_CONVERSION

    // CMYK - Cyan, Magenta, Yellow and Black
    SF_EXPORT void            SetCMYK(int c, int m, int y, int k);
    SF_EXPORT void            GetCMYK(int* pc, int* pm, int* py, int* pk) const;
    // CMY - Cyan, Magenta, Yellow
    SF_EXPORT void            SetCMY(int c, int m, int y);
    SF_EXPORT void            GetCMY(int* pc, int* pm, int* py) const;

    // Reduces the C, M and Y components and boosts the K component 
    // to achieve the same color but with a higher black component
    SF_EXPORT static void SF_STDCALL CorrectCMYK(int* pc, int* pm, int* py, int* pk);

    // YIQ
    SF_EXPORT void            SetYIQ(float y, float i, float q);
    SF_EXPORT void            GetYIQ(float *py, float *pi, float *pq) const;
    // YUV
    SF_EXPORT void            SetYUV(float y, float u, float v);
    SF_EXPORT void            GetYUV(float *py, float *pu, float *pv) const;

    // CIE XYZ
    SF_EXPORT void            SetXYZ(float x, float y, float z);
    SF_EXPORT void            GetXYZ(float *px, float *py, float *pz) const;
    // CIE Lab
    SF_EXPORT void            SetLab(float l, float a, float b);
    SF_EXPORT void            GetLab(float *pl, float *pa, float *pb) const;
    // CIE Luv
    SF_EXPORT void            SetLuv(float l, float u, float v);
    SF_EXPORT void            GetLuv(float *pl, float *pu, float *pv) const;

    // Lab - XYZ conversion
    SF_EXPORT static void SF_STDCALL ConvertLabToXYZ(Double l, Double a, Double b,
                                                     Double *px, Double *py, Double *pz);
    SF_EXPORT static void SF_STDCALL ConvertXYZToLab(Double x, Double y, Double z,
                                                     Double *pl, Double *pa, Double *pb);
    // Luv - XYZ conversion
    SF_EXPORT static void SF_STDCALL ConvertLuvToXYZ(Double l, Double u, Double v,
                                                     Double *px, Double *py, Double *pz);
    SF_EXPORT static void SF_STDCALL ConvertXYZToLuv(Double x, Double y, Double z,
                                                     Double *pl, Double *pu, Double *pv);

#endif  // SF_NO_COLOR_COMPLEX_CONVERSION


};

// ** Inline Implementation

// RGB Color                
inline void        Color::SetRGB(UByte r, UByte g, UByte b)
{
    SetRed(r);
    SetGreen(g);
    SetBlue(b);
}

inline void        Color::SetRGBA(UByte r, UByte g, UByte b, UByte a)
{
    SetRGB(r, g, b);
    SetAlpha(a);
}
                        

inline void        Color::GetRGB(UByte *pr, UByte *pg, UByte *pb) const
{ 
    *pr = GetRed(); 
    *pg = GetGreen(); 
    *pb = GetBlue(); 
}

inline void        Color::GetRGBA(UByte *pr, UByte *pg, UByte *pb, UByte *pa) const
{ 
    GetRGB(pr, pg, pb);
    *pa = GetAlpha();
}

// RGB Color - floating point
inline void        Color::SetRGBAFloat(float r, float g, float b, float a)
{
    SetRGBFloat(r,g,b);
    SetAlphaFloat(a);
}

inline void        Color::SetRGBFloat(float r, float g, float b)
{
    SetRed  ( UByte(r*255) );
    SetGreen( UByte(g*255) );
    SetBlue ( UByte(b*255) );
}

inline void        Color::SetAlphaFloat(float a)
{
    SetAlpha( UByte(a*255) );
}

inline void        Color::GetRGBAFloat(float *pr, float *pg, float *pb, float *pa) const
{
    GetRGBFloat(pr, pg, pb);
    GetAlphaFloat(pa);
}

inline void        Color::GetRGBAFloat(float *prgba) const
{
    GetRGBAFloat(&prgba[0], &prgba[1], &prgba[2], &prgba[3]);
}

inline void        Color::GetRGBFloat(float *pr, float *pg, float *pb) const
{
    *pr = GetRed()==0   ? 0 : GetRed()   / float(255);
    *pg = GetGreen()==0 ? 0 : GetGreen() / float(255);
    *pb = GetBlue()==0  ? 0 : GetBlue()  / float(255);
}

inline void        Color::GetRGBFloat  (float *prgb) const
{
    GetRGBFloat(&prgb[0], &prgb[1], &prgb[2] );
}

inline void        Color::GetAlphaFloat(float *pa) const
{
    *pa = GetAlpha()==0 ? 0 : GetAlpha() / float(255);
}

// Grayscale 
inline UByte       Color::GetGray() const
{ 
    return UByte( (unsigned(GetRed())*11 + unsigned(GetGreen())*16 + unsigned(GetBlue())*5)/32 ); 
}

inline Color      Color::GetColorGray() const
{
    UByte gray = GetGray();
    return Color(gray, gray, gray);
}

inline unsigned    Color::IIntensity() const
{
    return (unsigned)((SF_COLOR_ILUM_RED*GetRed()+SF_COLOR_ILUM_GREEN*GetGreen()+SF_COLOR_ILUM_BLUE*GetBlue())/32768L);
}

inline float       Color::FIntensity() const
{
    return (float)(SF_COLOR_LUM_RED*GetRed() + SF_COLOR_LUM_GREEN*GetGreen() + SF_COLOR_LUM_BLUE*GetBlue());
}

inline int         Color::DistanceSquared(Color c) const
{
    return ( ((c.GetRed()   - int (GetRed()))   * (c.GetRed()   - int (GetRed())))   + 
             ((c.GetGreen() - int (GetGreen())) * (c.GetGreen() - int (GetGreen()))) + 
             ((c.GetBlue()  - int (GetBlue()))  * (c.GetBlue()  - int (GetBlue()))) );
}

// *** Binary ops

// XOr - (c1 ^ c2)
inline UByte      Color::XOrCh(unsigned c1, unsigned c2)
{ return UByte( c1 ^ c2 ); }
inline Color      Color::XOr(Color c1, Color c2)
{ return Color( c1.Raw ^ c2.Raw ); }
inline Color      Color::XOrRGB(Color c1, Color c2)
{ return Color( c1.Raw ^ c2.Raw, c1.GetAlpha() ); }

// Or - (c1 | c2)
inline UByte      Color::OrCh(unsigned c1, unsigned c2)
{ return UByte( c1 | c2 ); }
inline Color      Color::Or(Color c1, Color c2)
{ return Color( c1.Raw | c2.Raw ); }
inline Color      Color::OrRGB(Color c1, Color c2)
{ return Color( c1.Raw | c2.Raw, c1.GetAlpha() ); }

// And - (c1 & c2)
inline UByte       Color::AndCh(unsigned c1, unsigned c2)
{ return UByte( c1 & c2 ); }
inline Color      Color::And(Color c1, Color c2)
{ return Color( c1.Raw & c2.Raw ); }
inline Color      Color::AndRGB(Color c1, Color c2)
{ return Color( c1.Raw & c2.Raw, c1.GetAlpha() ); }

// *** Color blending ops
 
// Modulate - (c1 * c2)
inline UByte       Color::ModulateCh(unsigned c1, unsigned c2)
{ return UByte( ((unsigned(c1)+1)*c2)>>8 ); }
inline Color      Color::Modulate(Color c1, Color c2)
{ 
    return Color( ModulateCh(c1.GetRed(),      c2.GetRed()),
                  ModulateCh(c1.GetGreen(),    c2.GetGreen()),
                  ModulateCh(c1.GetBlue(),     c2.GetBlue()),
                  ModulateCh(c1.GetAlpha(),    c2.GetAlpha()) );
}

inline Color      Color::ModulateRGB(Color c1, Color c2)
{
    return Color( ModulateCh(c1.GetRed(),      c2.GetRed()),
                  ModulateCh(c1.GetGreen(),    c2.GetGreen()),
                  ModulateCh(c1.GetBlue(),     c2.GetBlue()),
                  c1.GetAlpha() );
}

// Modulate - (c1 * factor)
inline UByte       Color::ModulateFactorCh(unsigned c1, UByte f)
{ 
    return UByte( Alg::Min<unsigned>(unsigned(c1*(unsigned(f)+1)), 255) ); 
}
inline Color      Color::ModulateFactor(Color c1, UByte f)
{
    return Color( ModulateFactorCh(c1.GetRed(),    f),
                  ModulateFactorCh(c1.GetGreen(),  f),
                  ModulateFactorCh(c1.GetBlue(),   f),
                  ModulateFactorCh(c1.GetAlpha(),  f)  );
}

inline Color      Color::ModulateFactorRGB(Color c1, UByte f)
{
    return Color( ModulateFactorCh(c1.GetRed(),    f),
                  ModulateFactorCh(c1.GetGreen(),  f),
                  ModulateFactorCh(c1.GetBlue(),   f),
                  c1.GetAlpha() );
}


// Add - (c1 + c2)
inline UByte       Color::AddCh(unsigned c1, unsigned c2)
{ 
    return UByte( Alg::Min<unsigned>(unsigned(c1)+c2, 255) ); 
}
inline Color      Color::Add(Color c1, Color c2)
{
    return Color( AddCh(c1.GetRed(),   c2.GetRed()),
                  AddCh(c1.GetGreen(), c2.GetGreen()),
                  AddCh(c1.GetBlue(),  c2.GetBlue()),
                  AddCh(c1.GetAlpha(), c2.GetAlpha()) );
}

inline Color      Color::AddRGB(Color c1, Color c2)
{
    return Color( AddCh(c1.GetRed(),   c2.GetRed()),
                  AddCh(c1.GetGreen(), c2.GetGreen()),
                  AddCh(c1.GetBlue(),  c2.GetBlue()),
                  c1.GetAlpha() );
}

// Subtract - (c1 - c2)
inline UByte       Color::SubtractCh(unsigned c1, unsigned c2)
{ 
    return UByte( (c1 < c2) ? 0 : c1 - c2 ); 
}
inline Color      Color::Subtract(Color c1, Color c2)
{
    return Color( SubtractCh(c1.GetRed(),      c2.GetRed()),
                  SubtractCh(c1.GetGreen(),    c2.GetGreen()),
                  SubtractCh(c1.GetBlue(),     c2.GetBlue()),
                  SubtractCh(c1.GetAlpha(),    c2.GetAlpha()) );
}

inline Color      Color::SubtractRGB(Color c1, Color c2)
{
    return Color( SubtractCh(c1.GetRed(),      c2.GetRed()),
                  SubtractCh(c1.GetGreen(),    c2.GetGreen()),
                  SubtractCh(c1.GetBlue(),     c2.GetBlue()),
                  c1.GetAlpha() );
}

// Blend 
inline UByte       Color::BlendCh(unsigned c1, unsigned c2, UByte f)
{ 
    return UByte( (((unsigned(c1)+1)*f)>>8) + (((unsigned(c2)+1)*(255-f))>>8) ); 
}

inline Color      Color::Blend(Color c1, Color c2, UByte f)
{
    return Color(  BlendCh(c1.GetRed(),    c2.GetRed(), f),
                   BlendCh(c1.GetGreen(),  c2.GetGreen(), f),
                   BlendCh(c1.GetBlue(),   c2.GetBlue(), f),
                   BlendCh(c1.GetAlpha(),  c2.GetAlpha(), f) );
}

inline Color      Color::BlendRGB(Color c1, Color c2, UByte f)
{
    return Color(  BlendCh(c1.GetRed(),    c2.GetRed(), f),
                   BlendCh(c1.GetGreen(),  c2.GetGreen(), f),
                   BlendCh(c1.GetBlue(),   c2.GetBlue(), f),
                   c1.GetAlpha() );
}


inline UByte       Color::InverseCh( unsigned c )
{ 
    return UByte(255-c); 
}

inline Color      Color::Inverse( Color c )
{
    return Color(  InverseCh(c.GetRed()),
                   InverseCh(c.GetGreen()),
                   InverseCh(c.GetBlue()),
                   InverseCh(c.GetAlpha()) );
}

inline Color      Color::InverseRGB( Color c )
{
    return Color(  InverseCh(c.GetRed()),
                   InverseCh(c.GetGreen()),
                   InverseCh(c.GetBlue()),
                   c.GetAlpha() );
}


inline const Color    operator ^  (Color c1, Color c2)
{ return Color::XOr(c1, c2); }
inline const Color    operator |  (Color c1, Color c2)
{ return Color::Or(c1, c2); }
inline const Color    operator &  (Color c1, Color c2)
{ return Color::And(c1, c2); }

inline const Color    operator -  (Color c1, Color c2)
{ return Color::Subtract(c1, c2); }
inline const Color    operator +  (Color c1, Color c2)
{ return Color::Add(c1, c2); }
inline const Color    operator *  (Color c1, Color c2)
{ return Color::Modulate(c1, c2); }
inline const Color    operator *  (Color c1, UByte factor)
{ return Color::Modulate(c1, factor); }


}} // Scaleform::Render

#endif
