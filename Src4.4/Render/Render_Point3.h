#ifndef INC_GPOINT3_H
#define INC_GPOINT3_H

/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Point3.h
Content     :   3D Point template class 
Created     :   Jan 15, 2010
Authors     :   Mustafa Thamer

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

namespace Scaleform { namespace Render {

template <class T>
class Point3
{
public:
    T   x;
    T   y;
    T   z;

    SF_INLINE Point3()                               { }
    SF_INLINE explicit Point3(T value)               { SetPoint(value); }
    SF_INLINE Point3(T x2, T y2, T z2)               { SetPoint(x2,y2, z2); }
    SF_INLINE Point3(const Point3<T> &pt)           { SetPoint(pt); }

    // *** Initialization                                                                                               
    SF_INLINE void SetPoint(T val)                    { x=y=z=val; }
    SF_INLINE void SetPoint(T x2, T y2, T z2)         { x=x2; y=y2; z=z2;}
    SF_INLINE void SetPoint(const Point3<T> &pt)     { SetPoint(pt.x, pt.y, pt.z); }
    SF_INLINE void Clear()                            { x=y=z=0; }

    // Offset the position
    SF_INLINE Point3<T>& Offset(const Point3<T> &pt) { return Offset(pt.x, pt.y, pt.z); }
    SF_INLINE Point3<T>& Offset(T val)               { return Offset(val,val, val); }
    SF_INLINE Point3<T>& Offset(T x2, T y2, T z2)    { x+=x2; y+=y2; z+=z2; return *this; }

    // Multiply the position
    SF_INLINE Point3<T>& Mul(const Point3<T> &pt)   { return Mul(pt.x, pt.y, pt.z); }
    SF_INLINE Point3<T>& Mul(T val)                  { return Mul(val,val, val); }
    SF_INLINE Point3<T>& Mul(T x2, T y2, T z2)       { x*=x2; y*=y2; z*=z2; return *this; }

    // Divide the position                                                      
    SF_INLINE Point3<T>& Div(const Point3<T> &pt)   { return Div(pt.x, pt.y, pt.z); }
    SF_INLINE Point3<T>& Div(T val)                  { return Div(val,val, val); }    
    SF_INLINE Point3<T>& Div(T x2, T y2, T z2)       { x/=x2; y/=y2; z/=z2; return *this; }

    // Distance squared (between this and the passed point)
    SF_INLINE T DistanceSquared(T x2,T y2, T z2) const        { return ((x2-x)*(x2-x)+(y2-y)*(y2-y)+(z2-z)*(z2-z)); }
    SF_INLINE T DistanceSquared(const Point3<T> &pt) const   { return DistanceSquared(pt.x, pt.y, pt.z); }
    SF_INLINE T DistanceSquared() const                       { return Dot(*this); }

    // Distance
    SF_INLINE T Distance(T x2,T y2, T z2) const       { return T(sqrt(DistanceSquared(x2,y2,z2))); }
    SF_INLINE T Distance(const Point3<T> &pt) const  { return Distance(pt.x, pt.y, pt.z); }
    SF_INLINE T Distance() const                      { return T(sqrt(DistanceSquared())); }

    SF_INLINE bool IsZero(T tol = FLT_EPSILON) const
    {
        T magSq = x*x + y*y + z*z;
        if (magSq <= tol*tol)
            return true;
        else
            return false;
    }

    SF_INLINE T Magnitude() const                     { return sqrt(x*x+y*y+z*z); }
    SF_INLINE void Normalize() 
    {
        Double lengthSq = x*x + y*y + z*z;
        if (lengthSq < SF_MATH_EPSILON*SF_MATH_EPSILON)
            return;

        T length = (T)sqrt(lengthSq);
        x = x/length;
        y = y/length;
        z = z/length;
    }

    // Dot product
    SF_INLINE T Dot(T x2,T y2, T z2) const            { return (x*x2 + y*y2 + z*z2); }
    SF_INLINE T Dot(const Point3<T> &pt) const       { return Dot(pt.x, pt.y, pt.z); }

    // cross product
    SF_INLINE void Cross(const Point3<T> &a, const Point3<T> &b)
    {
        x = a.y*b.z-a.z*b.y;
        y = a.z*b.x-a.x*b.z;
        z = a.x*b.y-a.y*b.x;
    }

    SF_INLINE T AngleBetween(const Point3<T>& pt) const
    {
        Point3<T> tmp;
        tmp.Cross(*this, pt);
        double s = tmp.Magnitude();
        double c = Dot(pt);
        double angle = atan2(s, c);
        return (T)angle;
    }
    SF_INLINE const Point3<T>& operator = (const Point3<T> &pt)  { SetPoint(pt); return *this; }
    SF_INLINE const Point3<T>& operator = (T val)    {  SetPoint(val); return *this; }

    SF_INLINE const Point3<T>   operator - () const  { return Point3<T>(-x, -y, -z); }

    SF_INLINE const Point3<T>& operator += (const Point3<T> &pt)    { return Offset(pt); }
    SF_INLINE const Point3<T>& operator += (T val)                   { return Offset(val); }

    SF_INLINE const Point3<T>& operator -= (const Point3<T> &pt)    { return Offset(-pt.x, -pt.y, -pt.z); }
    SF_INLINE const Point3<T>& operator -= (T val)                   { return Offset(-val); }

    SF_INLINE const Point3<T>& operator *= (const Point3<T> &pt)    { return Mul(pt); }
    SF_INLINE const Point3<T>& operator *= (T val)                   { return Mul(val); }

    SF_INLINE const Point3<T>& operator /= (const Point3<T> &pt)    { return Div(pt); }
    SF_INLINE const Point3<T>& operator /= (T val)                   { return Div(val); }

    SF_INLINE bool operator == (const Point3<T> &pt) const    { return ( (x==pt.x)&&(y==pt.y)&&(z==pt.z) ); }
    SF_INLINE bool operator != (const Point3<T> &pt) const    { return ( (x!=pt.x)||(y!=pt.y) || (z!=pt.z)); }

    SF_INLINE const Point3<T>   operator + (const Point3<T> &pt) const  { return Point3<T>(x+pt.x, y+pt.y, z+pt.z); }
    SF_INLINE const Point3<T>   operator + (T val) const                 { return Point3<T>(x+val, y+val, z+val); }
    SF_INLINE const Point3<T>   operator - (const Point3<T> &pt) const  { return Point3<T>(x-pt.x, y-pt.y, z-pt.z); }
    SF_INLINE const Point3<T>   operator - (T val) const                 { return Point3<T>(x-val, y-val, z-val); }
    SF_INLINE const Point3<T>   operator * (const Point3<T> &pt) const  { return Point3<T>(x*pt.x, y*pt.y, z*pt.z); }
    SF_INLINE const Point3<T>   operator * (T val) const                 { return Point3<T>(x*val, y*val, z*val); }
    //SF_INLINE const Point3<T>   operator * (T val, const Point3<T> &pt)               { return Point3<T>(pt.x*val, pt.y*val); }
    SF_INLINE const Point3<T>   operator / (const Point3<T> &pt) const  { return Point3<T>(x/pt.x, y/pt.y, z/pt.z); }
    SF_INLINE const Point3<T>   operator / (T val) const                 { return Point3<T>(x/val, y/val, z/val); }
};

typedef Point3<float> Point3F;

}}

#endif      // #ifndef INC_GPOINT3_H

