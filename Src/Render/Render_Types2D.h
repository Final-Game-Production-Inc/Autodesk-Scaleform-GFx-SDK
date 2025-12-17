/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Types2D.h
Content     :   Geometric 2D floating point types
Created     :   January 14, 1999
Authors     :   Michael Antonov, Brendan Iribe, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Types2D_H
#define INC_SF_Render_Types2D_H

#ifdef SF_MATH_H
#include SF_MATH_H
#else
#include <math.h>
#endif
#include <string.h>

#include "Kernel/SF_Alg.h"
#include "Kernel/SF_SIMD.h"
#include "Kernel/SF_RefCount.h"

namespace Scaleform { namespace Render {

// ****************************************************************************
// Size template class
// 
template <class T>
class Size
{
public:
    // *** Size data members
    T   Width;
    T   Height;

    enum BoundsType { Min, Max };

    // Constructors
    inline Size()                      { }
    inline explicit Size(T val)        { SetSize(val); }
    inline Size(T width, T height)     { SetSize(width, height); }
    inline Size(const Size<T> &sz)     { SetSize(sz.Width, sz.Height); }
    inline Size(BoundsType bt)         { SetSize(bt); }            
    
    // Initialization
    inline void SetSize(T val)                 { Width = val; Height = val; }
    inline void SetSize(T w, T h)              { Width = w;   Height = h; }
    inline void SetSize(const Size<T> &sz)     { SetSize(sz.Width, sz.Height); }
    //inline void SetSize(BoundsType bt);      // this function uses GFC_MAX_T which is not defined
    inline void SetWidth(T w)                  { Width = w; }
    inline void SetHeight(T h)                 { Height= h; }                                                
    inline void Clear()                        { Width=0; Height=0; }
    
    // Casting
    template<class S>
    inline operator Size<S>() const            { return Size<S>((S)Width, (S)Height); }

    // Swaps the two sizes
    inline void Swap(Size<T> *psz);
    inline void Transpose()            { Alg::Swap(Width, Height); }

    // Set to a + (b - a) * t
    inline Size<T>& SetLerp(const Size<T>& a, const Size<T>& b, T t);
                                                                         
    // State                                            
    inline bool IsEmpty() const    { return (Width<=1)||(Height<=1); }
    inline bool IsNull() const     { return (Width==0)&&(Height==0); }
    inline bool IsValid() const    { return (Width>=0)&&(Height>=0); }
                                                
    // Center values                        
    inline Size<T> Center() const { return Size<T>(HCenter(), VCenter()); }
    inline T HCenter() const       { return Width/2; }
    inline T VCenter() const       { return Height/2; }
                                                       
    // Area
    inline T Area() const  { return Width*Height; }
                                                
    // Expand the size
    inline Size<T>& Expand(T w, T h)              { Width+=w; Height+=h; return *this; }
    inline Size<T>& Expand(const Size<T> &sz)     { return Expand(sz.Width, sz.Height); }
    inline Size<T>& Expand(T value)               { return Expand(value,value); }
                
    // Contract the size
    inline Size<T>& Contract(T value)             { Width-=value; Height-=value; return *this; }
    inline Size<T>& Contract(const Size<T> &sz)   { return Contract(sz.Width, sz.Height); }
    inline Size<T>& Contract(T w, T h)            { Width-=w; Height-=h; return *this; }

    // Mul (scale) the size
    inline Size<T>& Mul(const Size<T> &sz)    { return Mul(sz.Width, sz.Height); }
    inline Size<T>& Mul(T value)              { Width*=value; Height*=value; return *this; }  
    inline Size<T>& Mul(T w, T h)             { Width*=w; Height*=h; return *this; }

    // Divide (scale) the size
    inline Size<T>& Div(const Size<T> &sz)    { return Div(sz.Width, sz.Height); }
    inline Size<T>& Div(T value)              { Width/=value; Height/=value; return *this; }
    inline Size<T>& Div(T w, T h)             { Width/=w; Height/=h; return *this; }
    
    // Clamp the size
    inline Size<T>& Clamp(const Size<T> &sz)  { return Clamp(sz.Width, sz.Height); }
    inline Size<T>& Clamp(T w, T h)           { if (Width>w) Width=w; if (Height>h) Height=h; return *this; }
    inline Size<T>& ClampWidth(T w)           { if (Width>w) Width=w; return *this; }
    inline Size<T>& ClampHeight(T h)          { if (Height>h) Height=h; return *this; }

    // Return new adjusted sizes
    inline Size<T> ExpandedTo(const Size<T> &sz) const    { return Size<T>( Alg::Max(Width, sz.Width), Alg::Max(Height, sz.Height) ); }
    inline Size<T> ExpandedTo(T w, T h) const             { return Size<T>( Alg::Max<T>(Width, w), Alg::Max<T>(Height, h) ); }
    inline Size<T> ExpandedToWidth(T w) const             { return Size<T>( Alg::Max<T>(Width, w), Height ); }
    inline Size<T> ExpandedToHeight(T h) const            { return Size<T>( Width, Alg::Max<T>(Height, h) ); }
                                                        
    inline Size<T> ClampedTo(const Size<T> &sz) const     { return Size<T>( Alg::Min(Width, sz.Width), Alg::Min(Height, sz.Height) ); }
    inline Size<T> ClampedTo(T w, T h) const              { return Size<T>( Alg::Min<T>(Width, w), Alg::Min<T>(Height, h) ); }
    inline Size<T> ClampedToWidth(T w) const              { return Size<T>( Alg::Min<T>(Width, w), Height); }
    inline Size<T> ClampedToHeight(T h) const             { return Size<T>( Width, Alg::Min<T>(Height, h) ); }

    // Operator overrides
    inline const Size<T>& operator =  ( const Size<T> &sz )   { SetSize(sz); return *this; }
    inline const Size<T>& operator =  ( T value )             { SetSize(value, value); return *this; }
    inline const Size<T>& operator =  ( BoundsType bt )       { SetSize(bt); return *this; }

    inline const Size<T>& operator += ( const Size<T> &sz )   { return Expand(sz); }
    inline const Size<T>& operator += ( T value )             { return Expand(value); }
                                                            
    inline const Size<T>& operator -= ( const Size<T> &sz )   { return Contract(sz); }
    inline const Size<T>& operator -= ( T value )             { return Contract(value); }
                                                            
    inline const Size<T>& operator *= ( const Size<T> &sz )   { return Mul(sz); }
    inline const Size<T>& operator *= ( T value )             { return Mul(value); }
                                                            
    inline const Size<T>& operator /= ( const Size<T> &sz )   { return Div(sz); }
    inline const Size<T>& operator /= ( T value )             { return Div(value); }

    inline const Size<T>& operator |= ( const Size<T> &sz )  { return (*this = ExpandedTo(sz)); }
    inline const Size<T>& operator &= ( const Size<T> &sz )  { return (*this = ClampedTo(sz)); }

    inline bool           operator == ( const Size<T> &sz ) const  { return (Width == sz.Width) && (Height == sz.Height); }
    inline bool           operator != ( const Size<T> &sz ) const  { return (Width != sz.Width) || (Height != sz.Height); }

    inline const Size<T>  operator -  ()                         const { return Size<T>(-Width, -Height); }
    inline const Size<T>  operator +  ( const Size<T> &sz)       const { return Size<T>(Width+sz.Width, Height+sz.Height); }
    inline const Size<T>  operator +  ( T value )                const { return Size<T>(Width+value, Height+value); }
    inline const Size<T>  operator -  ( const Size<T> &sz)       const { return Size<T>(Width-sz.Width, Height-sz.Height); }
    inline const Size<T>  operator -  ( T value )                const { return Size<T>(Width-value, Height-value); }
    inline const Size<T>  operator *  ( const Size<T> &sz)       const { return Size<T>(Width*sz.Width, Height*sz.Height); }
    inline const Size<T>  operator *  ( int value )              const { return Size<T>(Width*value, Height*value); }
    //inline const Size<T>    operator *  ( T value, const Size<T> &sz )                 { return Size<T>(sz.Width*value, sz.Height*value); }
    inline const Size<T>  operator /  ( const Size<T> &sz)       const { return Size<T>(Width/sz.Width, Height/sz.Height); }
    inline const Size<T>  operator /  ( T value )                const { return Size<T>(Width/value, Height/value); }
    inline const Size<T>  operator |  ( const Size<T> &sz)       const { return Size<T>(Alg::Max(Width,sz.Width), Alg::Max(Height,sz.Height)); }
    inline const Size<T>  operator &  ( const Size<T> &sz)       const { return Size<T>(Alg::Min(Width,sz.Width), Alg::Min(Height,sz.Height)); }
};


// Set to a + (b - a) * t
template <class T>
inline Size<T>& Size<T>::SetLerp(const Size<T>& a, const Size<T>& b, T t)
{
    Width   = a.Width + (b.Width - a.Width) * t;
    Height  = a.Height + (b.Height - a.Height) * t;
    return *this;
}


// Swaps the two sizes
template <class T>
inline void Size<T>::Swap(Size<T> *psz)
{
    Alg::Swap(Width, psz->Width);
    Alg::Swap(Height, psz->Height);
}

// ** End Inline Implementation


// ****************************************************************************
// Size template class
// 
template <class T>
class Point
{
public:
    // *** Point data members
    T   x;
    T   y;

    enum BoundsType { Min, Max };

    inline Point()                     { }
    inline explicit Point(T value)     { SetPoint(value); }
    inline Point(T x2, T y2)           { SetPoint(x2,y2); }
    inline Point(const Point<T> &pt)   { SetPoint(pt); }
    //inline Point<T>(BoundsType bt)      { SetPoint(bt); }
                                                            
    // *** Initialization                                                                                               
    inline void SetPoint(T val)                { x=y=val; }
    inline void SetPoint(T x2, T y2)           { x=x2; y=y2; }
    inline void SetPoint(const Point<T> &pt)   { SetPoint(pt.x, pt.y); }
    inline void SetPoint(BoundsType bt);                                                                
    inline void Clear()                        { x=y=0; }
    inline void Swap(Point<T> *ppt);

    // Casting
    template<class S>
    inline operator Point<S>() const            { return Point<S>((S)x, (S)x); }

    // Set to a + (b - a) * t
    inline Point<T>& SetLerp(const Point<T>& a, const Point<T>& b, T t);
                                                            
    // *** State information                                                                                            
    inline bool IsNull() const { return (x==0 && y==0); }
  
    // Offset the position
    inline Point<T>& Offset(const Point<T> &pt)  { return Offset(pt.x, pt.y); }
    inline Point<T>& Offset(const Size<T> &sz)   { return Offset(sz.Width, sz.Height); }
    inline Point<T>& Offset(T val)               { return Offset(val,val); }
    inline Point<T>& Offset(T x2, T y2)          { x+=x2; y+=y2; return *this; }

    // Multiply the position
    inline Point<T>& Mul(const Point<T> &pt)  { return Mul(pt.x, pt.y); }
    inline Point<T>& Mul(T val)               { return Mul(val,val); }
    inline Point<T>& Mul(T x2, T y2)          { x*=x2; y*=y2; return *this; }
    
    // Divide the position                                                      
    inline Point<T>& Div(const Point<T> &pt)  { return Div(pt.x, pt.y); }
    inline Point<T>& Div(T val)               { return Div(val,val); }    
    inline Point<T>& Div(T x2, T y2)          { x/=x2; y/=y2; return *this; }

    // Dot product
    inline T Dot(T x2,T y2) const             { return (x*x2 + y*y2); }
    inline T Dot(const Point<T> &pt) const    { return Dot(pt.x, pt.y); }
    
    // Angle in radians (between this and the passed point)
    inline T Angle(T x2,T y2) const           { return (T)atan2(y2-y, x2-x); }
    inline T Angle(const Point<T> &pt) const  { return Angle(pt.x,pt.y); }

    // Distance squared (between this and the passed point)
    inline T DistanceSquared(T x2,T y2) const           { return ((x2-x)*(x2-x)+(y2-y)*(y2-y)); }
    inline T DistanceSquared(const Point<T> &pt) const  { return DistanceSquared(pt.x, pt.y); }
    inline T DistanceSquared() const                    { return Dot(*this); }

    // Distance
    inline T Distance(T x2,T y2) const             { return T(sqrt(DistanceSquared(x2,y2))); }
    inline T Distance(const Point<T> &pt) const    { return Distance(pt.x, pt.y); }
    inline T Distance() const                      { return T(sqrt(DistanceSquared())); }

    // L1 Distance (Manhattan style)
    // - the equation is: Alg::Abs(x1-x2) + Alg::Abs(y1-y2)
    inline T DistanceL1(T x2,T y2) const           { return T(Alg::Abs(x-x2)+Alg::Abs(y-y2)); }
    inline T DistanceL1(const Point<T> &pt) const  { return DistanceL1(pt.x, pt.y); }
    // L Infinite Distance 
    // - the equation is: max(Alg::Abs(x1-x2),Alg::Abs(y1-y2))
    inline T DistanceLn(T x2,T y2) const           { return T(Alg::Max(Alg::Abs(x-x2), Alg::Abs(y-y2))); }
    inline T DistanceLn(const Point<T> &pt) const  { return DistanceLn(pt.x, pt.y); }  

    // Clamping                                                                     
    inline void Clamp(T _min, T _max)  { if (x<_min) x=_min; else if (x>_max) x=_max; 
                                         if (y<_min) y=_min; else if (y>_max) y=_max; }
    inline void ClampMin(T _min)       { if (x<_min) x=_min; if (y<_min) y=_min; }
    inline void ClampMax(T _max)       { if (x>_max) x=_max; if (y>_max) y=_max; }                                                 
        
    // Stores the absolute version of the point                         
    inline Point<T>& Absolute()   { x=(T)Alg::Abs(x); y=(T)Alg::Abs(y); return *this; }

    // Bitwise comparison; return true if *this is bitwise identical to p.  
    inline bool BitwiseEqual(const Point<T>& p) const { return memcmp(this, &p, sizeof(p)) == 0; }

    // *** Operators
    inline const Point<T>& operator = (const Point<T> &pt) { SetPoint(pt); return *this; }
    inline const Point<T>& operator = (T val)              { SetPoint(val); return *this; }
    inline const Point<T>& operator = (BoundsType bt)      { SetPoint(bt); return *this; }

    inline const Point<T>   operator - () const { return Point<T>(-x, -y); }

    inline const Point<T>& operator += (const Point<T> &pt)  { return Offset(pt); }
    inline const Point<T>& operator += (const Size<T> &sz)   { return Offset(sz); }
    inline const Point<T>& operator += (T val)               { return Offset(val); }
     
    inline const Point<T>& operator -= (const Point<T> &pt)  { return Offset(-pt.x, -pt.y); }
    inline const Point<T>& operator -= (const Size<T> &sz)   { return Offset(-sz.Width, -sz.Height); }
    inline const Point<T>& operator -= (T val)               { return Offset(-val); }
     
    inline const Point<T>& operator *= (const Point<T> &pt)  { return Mul(pt); }
    inline const Point<T>& operator *= (T val)               { return Mul(val); }
     
    inline const Point<T>& operator /= (const Point<T> &pt)  { return Div(pt); }
    inline const Point<T>& operator /= (T val)               { return Div(val); }

    inline bool operator == (const Point<T> &pt) const    { return ( (x==pt.x)&&(y==pt.y) ); }
    inline bool operator != (const Point<T> &pt) const    { return ( (x!=pt.x)||(y!=pt.y) ); }

    inline const Point<T>   operator + (const Point<T> &pt) const { return Point<T>(x+pt.x, y+pt.y); }
    inline const Point<T>   operator + (const Size<T> &sz)  const { return Point<T>(x+sz.Width, y+sz.Height); }
    inline const Point<T>   operator + (T val)              const { return Point<T>(x+val, y+val); }
    inline const Point<T>   operator - (const Point<T> &pt) const { return Point<T>(x-pt.x, y-pt.y); }
    inline const Point<T>   operator - (const Size<T> &sz)  const { return Point<T>(x-sz.Width, y-sz.Height); }
    inline const Point<T>   operator - (T val)              const { return Point<T>(x-val, y-val); }
    inline const Point<T>   operator * (const Point<T> &pt) const { return Point<T>(x*pt.x, y*pt.y); }
    inline const Point<T>   operator * (T val)              const { return Point<T>(x*val, y*val); }
    //inline const Point<T>   operator * (T val, const Point<T> &pt)               { return Point<T>(pt.x*val, pt.y*val); }
    inline const Point<T>   operator / (const Point<T> &pt) const { return Point<T>(x/pt.x, y/pt.y); }
    inline const Point<T>   operator / (T val)              const { return Point<T>(x/val, y/val); }
};


template <class T>
inline Point<T>& Point<T>::SetLerp(const Point<T>& a, const Point<T>& b, T t)
{
    x = a.x + (b.x - a.x) * t;
    y = a.y + (b.y - a.y) * t;
    return *this;
}

// Swaps the two points
template <class T>
inline void Point<T>::Swap(Point<T> *ppt)
{
    Alg::Swap(x, ppt->x);
    Alg::Swap(y, ppt->y);
}

// ** End Inline Implementation

// ****************************************************************************
// Rect template class
// 

// Extract data into a base class template, so it can be specialized when needed for alignment.
template <class T>
class RectData
{
public:
    // *** Rect data members
    T x1, y1, x2, y2;    
};

template <class T>
class Rect : public RectData<T>
{
public:
    using RectData<T>::x1;  // GCC 3.4 compatibility.
    using RectData<T>::y1;  // GCC 3.4 compatibility.
    using RectData<T>::x2;  // GCC 3.4 compatibility.
    using RectData<T>::y2;  // GCC 3.4 compatibility.

    enum NoInitType { NoInit };
    enum BoundsType { Min, Max };

    // Constructors
    inline Rect( )                                         { SetRect(0, 0, 0, 0); }
    inline Rect(NoInitType)                                { }
    inline explicit Rect(T val)                            { SetRect(val, val); }
    inline Rect(const Rect<T> &rc)                         { SetRect(rc); }
    inline Rect(const Size<T> &sz)                         { SetRect(sz); }
    inline Rect(T w, T h)                                  { SetRect(w, h); }
    inline Rect(const Point<T> &tl, const Point<T> &br)    { SetRect(tl, br); }    
    inline Rect(const Point<T> &tl, const Size<T> &sz)     { SetRect(tl, sz); }
    inline Rect(T x, T y, const Size<T> &sz)               { SetRect(x, y, sz); }  
    inline Rect(T left, T top, T right, T bottom)          { SetRect(left, top, right, bottom); }
    inline Rect(BoundsType bt)                             { SetRect(bt); }

    // *** Utility functions

    // Initialization
    inline void SetRect(T l, T t, T r, T b)                      { x1=l; y1=t; x2=r; y2=b; }   
    inline void SetRect(const Size<T> &sz)                       { SetRect(0, 0, sz.Width, sz.Height); }
    inline void SetRect(T w, T h)                                { SetRect(0, 0, w, h); }
    inline void SetRect(const Rect<T> &rc)                       { SetRect(rc.x1, rc.y1, rc.x2, rc.y2); }
    inline void SetRect(const Point<T> &tl, const Point<T> &br)  { SetRect(tl.x, tl.y, br.x, br.y); }
    inline void SetRect(T x, T y, const Size<T> &sz)             { SetRect(x, y, x+sz.Width, y+sz.Height); }
    inline void SetRect(const Point<T> &tl, const Size<T> &sz)   { SetRect(tl.x, tl.y, sz); }    
    inline void SetRect(BoundsType bt);     

    // Casting.
    template<class S>
    inline operator Rect<S>() const                              { return Rect<S>((S)x1, (S)y1, (S)x2, (S)y2); }

    // Set to a + (b - a) * t
    inline Rect<T>& SetLerp(const Rect<T>& a, const Rect<T>& b, T t); 

    inline void Clear()    { SetRect(Point<T>(0,0),Size<T>(0,0)); }

    inline void Swap(Rect<T> *pr);
    

    // *** Different rect data conversion
    // Point/Size rect (values are stored as a point and a size)
    inline Rect<T>&   FromRectPS(const Rect<T> *prc) { SetRect(prc->x1,prc->y1,Size<T>(prc->x2,prc->y2)); return *this; }
    inline void        ToRectPS(Rect<T> *prc) const   { prc->SetRect(x1,y1,Width(),Height()); }


    //  *** State functions
    // - these functions are for Normal rectangles only
    inline bool IsNull() const     { return (x2 == x1) && (y2 == y1); }
    inline bool IsEmpty() const    { return (x1 >= x2) || (y1 >= y2); }
    inline bool IsNormal() const   { return (x2 >= x1) && (y2 >= y1); }

    // Point properties
    inline T X1() const    { return x1; }
    inline T Y1() const    { return y1; }
    inline T X2() const    { return x2; }
    inline T Y2() const    { return y2; }

    // *** Corner position
    inline Point<T> TopLeft() const       { return Point<T>(x1, y1); }
    inline Point<T> TopRight() const      { return Point<T>(x2, y1); }
    inline Point<T> BottomLeft() const    { return Point<T>(x1, y2); }
    inline Point<T> BottomRight() const   { return Point<T>(x2, y2); }
                                                            
    inline void SetTopLeft(const Point<T> &pt)        { x1 = pt.x; y1 = pt.y; }
    inline void SetTopRight(const Point<T> &pt)       { x2 = pt.x; y1 = pt.y; }
    inline void SetBottomLeft(const Point<T> &pt)     { x1 = pt.x; y2 = pt.y; }
    inline void SetBottomRight(const Point<T> &pt)    { x2 = pt.x; y2 = pt.y; }
    
    // *** Side access
    // Returns a new rectangle that 
    inline Rect<T> TopSide(T height = 0.0) const      { return Rect<T>(x1, y1, x2, (y1+height)); }
    inline Rect<T> BottomSide(T height = 0.0) const   { return Rect<T>(x1, (y2-height), x2, y2); }
    inline Rect<T> LeftSide(T width = 0.0) const      { return Rect<T>(x1, y1, (x1+width), y2); }
    inline Rect<T> RightSide(T width = 0.0) const     { return Rect<T>((x2-width), y1, x2, y2); }
    
    // *** Center
    inline Point<T> Center() const     { return Point<T>(HCenter(), VCenter()); }
    inline T HCenter() const           { return (x2+x1)/2; }
    inline T VCenter() const           { return (y2+y1)/2; }

    inline void SetCenter(const Point<T> &nc);
    inline void SetCenter(T x, T y)             { SetCenter(Point<T>(x,y)); }
    inline void SetHCenter(T x);
    inline void SetVCenter(T y);


    // *** Size functions
    inline T Width() const     { return x2-x1; }
    inline T Height() const    { return y2-y1; }
    // Checked functions return 0 size for non-normal coordinate        
    inline T CheckedWidth() const  { return (x2>=x1) ? Width() : 0; }
    inline T CheckedHeight() const { return (y2>=y1) ? Height() : 0; }

    inline Size<T> GetSize() const        { return Size<T>(Width(), Height()); }
    inline Size<T> CheckedSize() const    { return Size<T>(CheckedWidth(), CheckedHeight()); }
                                                            
    inline void SetSize(T width, T height)     { SetWidth(width); SetHeight(height); }
    inline void SetSize(const Size<T> &sz)     { SetSize(sz.Width, sz.Height); }

    inline void SetWidth(T width)      { x2 = (x1+width); }
    inline void SetHeight(T height)    { y2 = (y1+height); }


    // *** Area
    inline T Area() const  { return Width()*Height(); }                        

    // *** Ranges
    inline void SetHRange(T l, T r)    { x1 = l; x2  = r; }
    inline void SetVRange(T t, T b)    { x1 = t; x2  = b; }
    
    // *** Sizing/Movement
    // Offset the rect
    inline Rect<T>& Offset(T x, T y)              { x1+=x; x2+=x; y1+=y; y2+=y; return *this; }
    inline Rect<T>& Offset(const Point<T> &pt)    { return Offset(pt.x, pt.y); }
    inline Rect<T>& OffsetX(T x)                  { x1+=x; x2+=x; return *this; }
    inline Rect<T>& OffsetY(T y)                  { y1+=y; y2+=y; return *this; }
                        
    // Expand (inflate) rect
    inline Rect<T>& Expand(T l, T t, T r, T b)    { x1-=l; x2+=r; y1-=t; y2+=b; return *this; }
    inline Rect<T>& Expand(T val)                 { return Expand(val,val,val,val); }
    // Applies the value to both sides (h = x1-h & x2+h)                   
    inline Rect<T>& Expand(T h, T v)  { return Expand(h,v,h,v); }
    inline Rect<T>& HExpand(T h)      { x1-=h; x2+=h; return *this; }
    inline Rect<T>& HExpand(T l, T r) { x1-=l; x2+=r; return *this; }    
    inline Rect<T>& VExpand(T v)      { y1-=v; y2+=v; return *this; }
    inline Rect<T>& VExpand(T t, T b) { y1-=t; y2+=b; return *this; }    

    // Expand this rectangle to enclose the given point.
    inline Rect<T>& ExpandToPoint(const Point<T> &pt)    { return ExpandToPoint(pt.x, pt.y); }
    inline Rect<T>& ExpandToPoint(T x, T y);    
                
    // Contract (deflate) rect          
    inline Rect<T>& Contract(T l, T t, T r, T b)  { x1+=l; x2-=r; y1+=t; y2-=b; return *this; } 
    inline Rect<T>& Contract(T val)               { return Contract(val,val,val,val); }
    // Applies the value to both sides (w = x1+w & x2-w)       
    inline Rect<T>& Contract(T h, T v)    { return Contract(h,v,h,v); }
    inline Rect<T>& HContract(T h)        { x1+=h; x2-=h; return *this; }
    inline Rect<T>& HContract(T l, T r)   { x1+=l; x2-=r; return *this; }    
    inline Rect<T>& VContract(T v)        { y1+=v; y2-=v; return *this; }
    inline Rect<T>& VContract(T t, T b)   { y1+=t; y2-=b; return *this; }
        
    // Clamp rect
    inline Rect<T>& Clamp(const Rect<T> &r)      { return Clamp(r.x1, r.y1, r.x2, r.y2); }
    inline Rect<T>& Clamp(T l, T t, T r, T b);
    inline Rect<T>& HClamp(T l, T r);
    inline Rect<T>& HClamp(const Rect<T> &r)     { return HClamp(r.x1, r.x2); } 
    inline Rect<T>& VClamp(T t, T b);
    inline Rect<T>& VClamp(const Rect<T> &r)     { return VClamp(r.y1, r.y2); } 

    // Rectangle becomes the sum of this and r
    inline Rect<T>& Union(T l, T t, T r, T b);
    inline Rect<T>& Union(const Rect<T> &r)      { return Union(r.x1,r.y1,r.x2,r.y2); }

    // Stores the area inside both rectangles
    // - clears the rectangle if they don抰 intersect
    inline Rect<T>& Intersect(T l, T t, T r, T b);
    inline Rect<T>& Intersect(const Rect<T> &r)      { return Intersect(r.x1,r.y1,r.x2,r.y2); }

    // Normalize the rectangle
    inline void Normalize();
    inline void NormalizeX();
    inline void NormalizeY();
    // Returns a normalized copy 
    inline Rect<T> Normal() const;

    // *** Area Testing
    // Returns 1 if the passed geometry is completely inside the rectangle
    inline bool Contains(T x, T y) const              { return ((x<=x2)&&(x>=x1)&&(y<=y2)&&(y>=y1)); }
    inline bool Contains(const Point<T> &pt) const    { return ((pt.x<=x2)&&(pt.x>=x1)&&(pt.y<=y2)&&(pt.y>=y1)); }
    inline bool Contains(const Rect<T> &r) const      { return ((x2>=r.x2)&&(y2>=r.y2)&&(x1<=r.x1)&&(y1<=r.y1)); }

    // Returns 1 if rectangles overlap at all
    // - r maybe completely inside
    inline bool Intersects(const Rect<T> &r) const;
    
    // Returns 1 if the rectangle touches one of the edges
    // - can optionally adjust the edge width
    inline bool IntersectsEdge(const Rect<T> &r, T ewidth=1) const;
    // Handles separate widths for each sides edge
    inline bool IntersectsEdge(const Rect<T> &r, T leftw, T topw, T rightw, T bottomw) const;

    // *** General calculations
    // - store new rectangle inside pdest

    // Clamps a point to the rectangle
    // - returns a new value that is inside the rect
    inline Point<T> ClampPoint(const Point<T> &pt) const;

    // Calculates area inside both rectangles
    // Returns 0 if rectangles don抰 intersect
    inline bool IntersectRect(Rect<T> *pdest, const Rect<T> &r) const;
    // Calculates the area enclosing both rectangles
    inline void UnionRect(Rect<T> *pdest, const Rect<T> &r) const;
    
    // *** Operator overrides
    inline const Rect<T>& operator =  (const Rect<T> &r)   { SetRect(r); return *this; }
    inline const Rect<T>& operator =  (BoundsType bt)      { SetRect(bt); return *this; }
                
    inline const Rect<T>& operator += (const Point<T> &pt) { return Offset(pt); }  
    inline const Rect<T>& operator += (T val)              { return Offset(val,val); }
          
    inline Rect<T>& operator -= (const Point<T> &pt)       { return Offset(-pt); } 
    inline Rect<T>& operator -= (T val)                    { return Offset(-val,-val); }
          
    inline const Rect<T>& operator |= (const Rect<T> &r);
    inline const Rect<T>& operator &= (const Rect<T> &r);

    inline bool operator == (const Rect<T> &r) const       { return ((x1==r.x1)&&(x2==r.x2)&&(y1==r.y1)&&(y2==r.y2) ); }
    inline bool operator != (const Rect<T> &r) const       { return ((x1!=r.x1)||(x2!=r.x2)||(y1!=r.y1)||(y2!=r.y2) ); }

    inline const Rect<T>    operator -  ()                    const   { return Rect<T>(-x1,-y1,-x2,-y2); }
    inline const Rect<T>    operator +  ( const Point<T> &pt) const   { return Rect<T>(*this).Offset(pt); }
    inline const Rect<T>    operator +  (T val)               const   { return Rect<T>(x1+val, y1+val, x2+val, y2+val); }
    inline const Rect<T>    operator -  (const Point<T> &pt)  const   { return Rect<T>(*this).Offset(-pt); }
    inline const Rect<T>    operator -  (T val)               const   { return Rect<T>(x1-val, y1-val, x2-val, y2-val); }
    inline const Rect<T>    operator +  (const Rect<T> &r)    const   { Rect<T> dest; UnionRect(&dest, r); return dest; }                                                                                  
    inline const Rect<T>    operator |  (const Rect<T> &r)    const   { Rect<T> dest; UnionRect(&dest, r); return dest; }
    inline const Rect<T>    operator &  (const Rect<T> &r)    const   { Rect<T> dest; IntersectRect(&dest, r); return dest; }
};


// ** Inline Implementation
template <class T>
inline void Rect<T>::SetRect(BoundsType bt)
{
    switch (bt)
    {
        case Min:   SetRect(Point<T>(Point<T>::Min), Size<T>(Size<T>::Min)); break;
        case Max:   SetRect(Point<T>(Point<T>::Min), Size<T>(Size<T>::Max)); break;
    }
}

// Set to a + (b - a) * t
template <class T>
inline Rect<T>& Rect<T>::SetLerp(const Rect<T>& a, const Rect<T>& b, T t)
{
    x1 = (b.x1 - a.x1) * t + a.x1;
    y1 = (b.y1 - a.y1) * t + a.y1;
    x2 = (b.x2 - a.x2) * t + a.x2;
    y2 = (b.y2 - a.y2) * t + a.y2;
    return *this;
}

// Swaps the two rectangles
template <class T>
inline void Rect<T>::Swap(Rect<T> *pr)
{
    Alg::Swap(x1, pr->x1);
    Alg::Swap(x2, pr->x2);
    Alg::Swap(y1, pr->y1);
    Alg::Swap(y2, pr->y2);
}

// Clamp rect
template <class T>
inline Rect<T>& Rect<T>::Clamp(T left, T top, T right, T bottom)
{
    if (left  >x1) x1=left;
    if (top   >y1) y1=top;
    if (right <x2) x2=right;
    if (bottom<y2) y2=bottom;
    return *this;
}

template <class T>
inline Rect<T>& Rect<T>::HClamp(T left, T right)
{
    if (left>x1)  x1=left;
    if (right<x2) x2=right;
    return *this;
}

template <class T>
inline Rect<T>& Rect<T>::VClamp(T top, T bottom)
{
    if (top>y1)    y1=top;
    if (bottom<y2) y2=bottom;
    return *this;
}

template <class T>
inline Rect<T>& Rect<T>::ExpandToPoint(T x, T y) 
{
    x1 = Alg::Min<T>(x1, x);
    y1 = Alg::Min<T>(y1, y);
    x2 = Alg::Max<T>(x2, x);
    y2 = Alg::Max<T>(y2, y);
    return *this;
}


// Stores the sum of this and r
template <class T>
inline Rect<T>& Rect<T>::Union(T left, T top, T right, T bottom)
{
    Rect<T> ur;
    UnionRect(&ur, Rect<T>(left, top, right, bottom));
    SetRect(ur);
    return *this;
}

template <class T>
inline Rect<T>& Rect<T>::Intersect(T left, T top, T right, T bottom)
{
    Rect<T> r (left, top, right, bottom);
    if (!Intersects(r))
    {
        Clear();
        return *this;
    }
    x1 = (x1>r.x1) ? x1    : r.x1;
    x2 = (x2>r.x2) ? r.x2  : x2;
    y1 = (y1>r.y1) ? y1    : r.y1;
    y2 = (y2>r.y2) ? r.y2  : y2;
    return *this;
}


// Normalize the rectangle
template <class T>
inline void    Rect<T>::Normalize()
{       
    if (x1>x2) Alg::Swap(x1, x2);
    if (y1>y2) Alg::Swap(y1, y2);
}
template <class T>
inline void    Rect<T>::NormalizeX()
{       
    if (x1>x2) Alg::Swap(x1, x2);    
}
template <class T>
inline void    Rect<T>::NormalizeY()
{       
    if (y1>y2) Alg::Swap(y1, y2);
}


// Normalize the rectangle
template <class T>
inline Rect<T>  Rect<T>::Normal() const
{       
    Rect<T> r(*this);
    r.Normalize();
    return r;
}

// Sets the center
template <class T>
inline void    Rect<T>::SetCenter(const Point<T> &nc)
{
    Point<T> oc = Center();
    Offset(nc.x-oc.x, nc.y-oc.y);
}

// Sets the horizontal center
template <class T>
inline void    Rect<T>::SetHCenter(T x)
{ Offset(x-HCenter(), 0); }

// Sets the vertical center
template <class T>
inline void    Rect<T>::SetVCenter(T y)
{ Offset(0, y-VCenter()); }


// Returns 1 if the rectangles overlap
template <class T>
inline bool    Rect<T>::Intersects(const Rect<T> &r) const
{
    if ( (y2>=r.y1)&&(r.y2>=y1) )
        if ( (r.x2>=x1)&&(x2>=r.x1) )
            return 1;
    return 0;
}

// Returns 1 if the passed rectangle overlaps the edge
template <class T>
inline bool    Rect<T>::IntersectsEdge(const Rect<T> &r, T ew) const
{
    return IntersectsEdge(r, ew, ew, ew, ew);
}

template <class T>
inline bool    Rect<T>::IntersectsEdge(const Rect<T> &r, T lw, T tw, T rw, T bw) const
{
    Rect<T> inside = *this;
    inside.Contract(lw, tw, rw, bw);
    return (Intersects(r) && !inside.Contains(r));
}


// Calculates area inside both GRectangles
// Return 0 if GRectangles don抰 intersect
template <class T>
inline bool    Rect<T>::IntersectRect(Rect<T> *pdest, const Rect<T> &r) const
{
    if (!Intersects(r)) return 0;
    pdest->x1 = (x1>r.x1) ? x1   : r.x1;
    pdest->x2 = (x2>r.x2) ? r.x2 : x2;
    pdest->y1 = (y1>r.y1) ? y1   : r.y1;
    pdest->y2 = (y2>r.y2) ? r.y2 : y2;
    return 1;
}

// Calculates the area enclosing both GRectangles
template <class T>
inline void    Rect<T>::UnionRect(Rect<T> *pdest, const Rect<T> &r) const
{
    pdest->x1 = (x1>r.x1) ? r.x1 : x1;
    pdest->x2 = (x2>r.x2) ? x2   : r.x2;
    pdest->y1 = (y1>r.y1) ? r.y1 : y1;
    pdest->y2 = (y2>r.y2) ? y2   : r.y2;
}

template <class T>
inline Point<T> Rect<T>::ClampPoint(const Point<T> &pt) const
{
    Point<T> npt;
    npt.x = (x1>=pt.x) ? x1 : (x2<=pt.x) ? x2 : pt.x;
    npt.y = (y1>=pt.y) ? y1 : (y2<=pt.y) ? y2 : pt.y;
    return npt;
}

template <class T>
inline const Rect<T>& Rect<T>::operator |= (const Rect<T> &r)
{
    UnionRect(this, r); 
    return *this;
}

template <class T>
inline const Rect<T>& Rect<T>::operator &= (const Rect<T> &r)
{
    if (!IntersectRect(this, r))
        this->Clear();
    return *this;
}
                                                                                           
// ** End Inline Implementation

// Ref-coutanble Rect
template <class T>
class RectRef : public Rect<T>, public RefCountBase<Rect<T>, Stat_Default_Mem>
{
public:
    using RectData<T>::x1;  // GCC 3.4 compatibility.
    using RectData<T>::y1;  // GCC 3.4 compatibility.
    using RectData<T>::x2;  // GCC 3.4 compatibility.
    using RectData<T>::y2;  // GCC 3.4 compatibility.

    inline void operator = (const Rect<T> &r)
    {
        x1 = r.x1;
        y1 = r.y1;
        x2 = r.x2;
        y2 = r.y2;
    }
};

// ****************************************************************************
// Backwards compatible typedefs 
// 

// float structures 
typedef Point<float> PointF;
typedef Size<float>  SizeF;
typedef Rect<float>  RectF;
typedef RectRef<float>  RectFRef;

// Double structures
typedef Point<Double> PointD;
typedef Size<Double>  SizeD;
typedef Rect<Double>  RectD;

// Alignment specialization
#ifdef SF_ENABLE_SIMD
template<>
class RectData<float>
{
public:
    // *** Rect data members
    SF_SIMD_ALIGN( float x1 );
    float y1, x2, y2;
};
#endif

}} // Scaleform::Render

#endif
