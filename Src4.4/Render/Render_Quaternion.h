/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Quaternion.h
Content     :   Quaternion class
Created     :   March 2014
Authors     :   TSpline Quaternion code ported to Scaleform by Douglas Cooper

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Quaternion_H
#define INC_SF_Render_Quaternion_H

#include <cmath>

#include "../Kernel/SF_Alg.h"
#include "../Kernel/SF_Math.h"
#include "Render_Point3.h"

#ifdef SF_BUILD_DEBUG
#include "../Kernel/SF_Debug.h"
#endif


namespace Scaleform { namespace Render {


// Quaternion class for representing rotations.
template <typename T>
class Quaternion
{
public:

    // Constructors

    // Default Constructor
    Quaternion() : X( (T)0.0 ), Y( (T)0.0 ), Z( (T)0.0 ), W( (T)1.0 ) {}

    // Constructor
    Quaternion( T x, T y, T z, T w ) : X( x ), Y( y ), Z( z ), W( w ) {}

    // Constructor
    Quaternion( const T values[4] ) : X( values[0] ), Y( values[1]  ), Z( values[2]  ), W( values[3]  ) {}

    // Constructor which converts Euler angles (radians) to a Quaternion
    Quaternion( T x, T y, T z ) { Set( x, y, z ); }

    // Constructor which converts Euler angles (radians) to a Quaternion
    Quaternion( const Point3<T>& pt ) { Set( pt.x, pt.y, pt.z ); }

    // The implicit copy constructor should be appropriate....
    //Quaternion( const Quaternion& quat );


    // Element Accessors
    T GetX() const { return X; }
    T GetY() const { return Y; }
    T GetZ() const { return Z; }
    T GetW() const { return W; }

    void SetX(T val) { X = val; }
    void SetY(T val) { Y = val; }
    void SetZ(T val) { Z = val; }
    void SetW(T val) { W = val; }

    // Conversion Operations

    // Converts to an angle ( radians ) around an axis ( unit vector )
    // 0 <= angle <= 2*PI
    void GetAngleAxis( T &angle, Point3<T>& axis ) const
    {
        const Double scale = sqrt(X*X + Y*Y + Z*Z);
        if( ( abs(scale) < SF_MATH_EPSILON ) || W < -1.0 || 1.0 < W )
        {
            angle  = 0.0;
            axis.x = 0.0;
            axis.y = 1.0;
            axis.z = 0.0;
        }
        else
        {
            SF_ASSERT(-1.0 <= W && W <= 1.0);
            const Double invscale = ( 1.0 / scale );
            angle  = 2.0 * acos( W );

            axis.x = X * invscale;
            axis.y = Y * invscale;
            axis.z = Z * invscale;
        }
    }

    // Output this Quaternion as Euler angles ( radians )
    void GetEulerAngles( Point3<T>& angles ) const
    {
        const Double sqw = W*W;
        const Double sqx = X*X;
        const Double sqy = Y*Y;
        const Double sqz = Z*Z;

        // heading = rotation about z-axis
        angles.z = static_cast<T>( atan2( 2.0*(X*Y + W*Z), sqx - sqy - sqz + sqw ) );

        // bank = rotation about x-axis
        angles.x = static_cast<T>( atan2( 2.0*(Y*Z + W*X), -sqx - sqy + sqz + sqw ) );

        // attitude = rotation about y-axis
        const Double magSq = MagnitudeSquared();
        Double y = -2.0 * (X*Z - W*Y);
        Double tol = (Double)SF_MATH_EPSILON;
        if (tol < Alg::Abs(magSq) && !Alg::ToleranceEqual<Double>(magSq, 1.0, tol))
            y /= magSq;

        if( y < -1.0 )
            y = -1.0;
        else if( y > 1.0 )
            y = 1.0;

        angles.y = static_cast<T>( asin( y ) );
    }


    T DotProduct( const Quaternion& rhs ) const
    {
        return ( X * rhs.X ) + ( Y * rhs.Y ) + ( Z * rhs.Z ) + ( W * rhs.W );
    }


    T Magnitude() const
    {
        const Double magSq = X*X + Y*Y + Z*Z + W*W;

        if (Alg::ToleranceEqual(magSq, (Double)1.0, (Double)SF_MATH_EPSILON))
            return 1;
        else
            return static_cast< T >(sqrt(magSq));
    }

    Double MagnitudeD() const
    {
        const Double magSq = X*X + Y*Y + Z*Z + W*W;

        if (Alg::ToleranceEqual<Double>(magSq, (Double)1.0, (Double)SF_MATH_EPSILON))
            return 1;
        else
            return static_cast<Double>(sqrt(magSq));
    }

    T MagnitudeSquared() const
    {
        return (X*X + Y*Y + Z*Z + W*W);
    }

    bool IsZero(T tol) const
    {
        return (X*X + Y*Y + Z*Z + W*W <= tol*tol);
    }

    bool IsIdentity(T tol) const
    {
        return (X*X + Y*Y + Z*Z <= tol*tol);
    }


    // Operators

    // Assignment operator
    Quaternion& operator=( const Quaternion& rhs )
    {
        if (&rhs == this)
            return *this;

        X = rhs.X;
        Y = rhs.Y;
        Z = rhs.Z;
        W = rhs.W;

        return *this;
    }

    // Equality operator
    bool operator==( const Quaternion& rhs ) const
    {
        return (( X == rhs.X ) &&
                ( Y == rhs.Y ) &&
                ( Z == rhs.Z ) &&
                ( W == rhs.W ) );
    }


    // returns true when this Quaternion equals the rhs Quaternion, within a tolerance
    bool Equals( const Quaternion& rhs, T tolerance ) const
    {
        return  Alg::ToleranceEqual( X, (T)rhs.X, tolerance ) &&
                Alg::ToleranceEqual( Y, (T)rhs.Y, tolerance ) &&
                Alg::ToleranceEqual( Z, (T)rhs.Z, tolerance ) &&
                Alg::ToleranceEqual( W, (T)rhs.W, tolerance );
    }

    // Inequality operator
    bool operator!=( const Quaternion& rhs ) const
    {
        return !( *this == rhs );
    }


    // Add operator
    Quaternion operator+( const Quaternion& rhs ) const
    {
        return Quaternion( X+rhs.X, Y+rhs.Y, Z+rhs.Z, W+rhs.W );
    }

    // Multiplication operator
    Quaternion operator*( const Quaternion& rhs ) const
    {
        Quaternion tmp;

        // Rotate by left hand side first, then by right hand side
        tmp.W = ( rhs.W * W ) - ( rhs.X * X ) - ( rhs.Y * Y ) - ( rhs.Z * Z );
        tmp.X = ( rhs.W * X ) + ( rhs.X * W ) + ( rhs.Y * Z ) - ( rhs.Z * Y );
        tmp.Y = ( rhs.W * Y ) + ( rhs.Y * W ) + ( rhs.Z * X ) - ( rhs.X * Z );
        tmp.Z = ( rhs.W * Z ) + ( rhs.Z * W ) + ( rhs.X * Y ) - ( rhs.Y * X );

        //// Rotate by right hand side, then by left hand side:
        //tmp.W = ( rhs.W * W ) - ( rhs.X * X ) - ( rhs.Y * Y ) - ( rhs.Z * Z );
        //tmp.X = ( rhs.W * X ) + ( rhs.X * W ) - ( rhs.Y * Z ) + ( rhs.Z * Y );
        //tmp.Y = ( rhs.W * Y ) + ( rhs.Y * W ) - ( rhs.Z * X ) + ( rhs.X * Z );
        //tmp.Z = ( rhs.W * Z ) + ( rhs.Z * W ) - ( rhs.X * Y ) + ( rhs.Y * X );

        return tmp;
    }

    //Multiplication operator with Point3
    Point3<T> operator*( const Point3<T>& v ) const
    {
        Point3<T> uv, uuv;
        Point3<T> qvec(X, Y, Z);

        uv.Cross(qvec, v);
        uuv.Cross(qvec, uv);
        uv  *= static_cast<T>(2.0 * W);
        uuv *= static_cast<T>(2.0);

        return v + uv + uuv;
    }

    // Multiplication operator with scalar
    Quaternion operator*( T s ) const
    {
        return Quaternion( s*X, s*Y, s*Z, s*W );
    }

    // Multiplication operator with scalar
    Quaternion& operator*=( T s )
    {
        X *= s;
        Y *= s;
        Z *= s;
        W *= s;

        return *this;
    }

    // Multiplication operator
    Quaternion& operator*=( const Quaternion& rhs )
    {
        return ( *this = rhs * ( *this ) );
    }


    // Set Quaternion
    void Set( T x, T y, T z, T w )
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }

    // Set Quaternion based on Euler angles ( radians )
    void Set( T x, T y, T z )
    {
        Double angle;

        angle = x * 0.5;
        const Double sr = sin( angle );
        const Double cr = cos( angle );

        angle = y * 0.5;
        const Double sp = sin( angle );
        const Double cp = cos( angle );

        angle = z * 0.5;
        const Double sy = sin( angle );
        const Double cy = cos( angle );

        const Double cpcy = cp * cy;
        const Double spcy = sp * cy;
        const Double cpsy = cp * sy;
        const Double spsy = sp * sy;

        X = static_cast<T>( sr * cpcy - cr * spsy );
        Y = static_cast<T>( cr * spcy + sr * cpsy );
        Z = static_cast<T>( cr * cpsy - sr * spcy );
        W = static_cast<T>( cr * cpcy + sr * spsy );

        Normalize();
    }

    // Set Quaternion based on Euler angles ( radians )
    void Set( const Point3<T>& angles ) { return Set( angles.x, angles.y, angles.z ); }


    // Set Quaternion to represent a rotation from one vector to another.
    void Set( const Point3<T>& from, const Point3<T>& to )
    {
        // Based on Stan Melax's article in Game Programming Gems

        // Copy, since cannot modify input
        Point3<T> v0 = from;
        Point3<T> v1 = to;

        v0.Normalize();
        v1.Normalize();

        const Double d = v0.Dot( v1 );
        if( d + SF_MATH_EPSILON >= 1.0 )
        {
            // If DotProduct == 1, vectors are the same
            SetIdentity();
            return;
        }
        else if( d <= -1.0 + SF_MATH_EPSILON )
        {
            // nearly (or exactly) opposite
            Point3<T> axis;
            Point3<T> axisY(0, 1, 0);
            axis.Cross( axisY, Point3<T>(X, Y, Z) );
            if( axis.DistanceSquared() == (T)0.0 )
            {
                Point3<T> axisZ(0, 0, 1);
                axis.Cross( axisZ, Point3<T>(X, Y, Z) );
            }
            SetFromAngleAxis( (T)SF_MATH_PI, axis );
            return;
        }

        const T s = static_cast<T>(sqrt( ( 1.0 + d ) * 2.0 ));
        Point3<T> c;
        c.Cross(v0, v1);
        c.Div(s);
        X = c.x;
        Y = c.y;
        Z = c.z;
        W = s / 2.0;
    }


    // Normalize the Quaternion
    void Normalize()
    {
        const Double magSq = X*X + Y*Y + Z*Z + W*W;

        // Don't attempt to normalize a zero magnitude quaternion
        if (magSq <= SF_MATH_EPSILON)
            return;

        if (Alg::ToleranceEqual<Double>(magSq, 1.0, SF_MATH_EPSILON ))
            return;

        (*this *= static_cast< T >( 1.0 / sqrt(magSq) ));
    }

    void Invert()
    {
        X = -X; Y = -Y; Z = -Z;
    }

    // Set to the Identity
    void SetIdentity()
    {
        W = static_cast<T>(1.0);
        X = static_cast<T>(0.0);
        Y = static_cast<T>(0.0);
        Z = static_cast<T>(0.0);
    }


    Quaternion& nlerp( Quaternion<T> q1, Quaternion<T> q2, T t )
    {
        X = Alg::Lerp(q1.GetX(), q2.GetX(), t);
        Y = Alg::Lerp(q1.GetY(), q2.GetY(), t);
        Z = Alg::Lerp(q1.GetZ(), q2.GetZ(), t);
        W = Alg::Lerp(q1.GetW(), q2.GetW(), t);
        Normalize();
    }

    Quaternion& slerp( Quaternion<T> q1, Quaternion<T> q2, T t )
    {
        Double tol     = (Double)SF_MATH_EPSILON;
        Double tolSq   = tol*tol;
        Double mag1_sq = (Double)q1.MagnitudeSquared();
        Double mag2_sq = (Double)q2.MagnitudeSquared();
        if (mag1_sq <= tolSq)
            if (mag2_sq <= tolSq)
                return ( *this = q1*0 );
            else
                return ( *this = (q2*t) );
        else if (mag2_sq <= tolSq)
            return ( *this = (q1*(1 - t)) );

        Double mag1  = (Double)sqrt(mag1_sq);
        Double mag2  = (Double)sqrt(mag2_sq);
        Double angle = q1.DotProduct( q2 );

        if (!Alg::ToleranceEqual<Double>(mag1, (Double)1.0, tol))
            angle /= mag1;
        if (!Alg::ToleranceEqual<Double>(mag2, (Double)1.0, tol))
            angle /= mag2;

        if ( angle < 0.0 )
        {
            q1    *= static_cast<T>(-1.0);
            angle *= static_cast<T>(-1.0);
        }

        T scale;
        T invscale;

        // Because 0 <= angle, the else block will never be executed.
        // Come back and reconsider the logic in this routine.
        // TODO: Rethink the handling of q1 == -q2.
        if ( ( angle + 1.0 ) > 0.05 )
        {
            if ( ( 1.0 - angle ) >= 0.05 ) // spherical interpolation
            {
                SF_ASSERT( -1.0 < angle && angle < 1.0 );
                const Double theta = acos( angle );
                const Double invsintheta = ( 1.0 / sin( theta ) );
                scale = static_cast<T>( sin( theta * ( 1.0 - t ) ) * invsintheta );
                invscale = static_cast<T>( sin( theta * t ) * invsintheta );
            }
            else // linear interpolation
            {
                scale = static_cast<T>( 1.0 - t );
                invscale = static_cast<T>( t );
            }
        }
        else
        {
            q2.Set( -q1.GetY(), q1.GetX(), -q1.GetW(), q1.GetZ() );
            scale = static_cast<T>( sin(SF_MATH_PI * ( 0.5 - t )) );
            invscale = static_cast<T>( sin(SF_MATH_PI * t) );
        }

        return ( *this = ( q1*scale ) + ( q2*invscale ) );
    }

    //// Convenience function to slerp two vectors.
    //static Point3<T> slerp(const Point3<T>& v1, const Point3<T>& v2, T interpolate ) const
    //{
    //    Quaternion<T> q1; // No rotation - v1 to v1
    //    Quaternion<T> q2; // v1 to v2.
    //    q2.Set(v1, v2);
    //
    //    Quaternion<T> qlerp;
    //    qlerp.slerp(q1, q2, interpolate);
    //    Matrix4x4<T> mat;
    //    mat.GetMatrix(qlerp);
    //    return mat.TransformVector(v1);
    //}

//protected:
    // Create a Quaternion from a rotation angle and a rotation axis.
    // Axis should preferably be of unit length.
    // The Quaternion representing the rotation is
    // q = cos( A/2 )+sin( A/2 )*( x*i+y*j+z*k ).
    // parameter 1) angle is the Rotation Angle in radians.
    // parameter 2) axis is the Rotation axis.  It is more efficient
    //              if this is a unit vector.  If it is not, then the
    //              code creates a unit vector from the input axis.
    void SetFromAngleAxis( T angle, const Point3<T>& axis )
    {
        // Expecting an angle in the [0, 2*PI) range
        Double half_angle = 0.5 * angle;
        if (half_angle < 0) // Angle must have been in the [-PI, PI) interval
            half_angle += SF_MATH_PI;

        // 0 <= half_angle <= PI ==> -1 <= W <= 1
        W = static_cast<T>( cos( half_angle ) );

        const T s = static_cast<T>( sin( half_angle ) );
        if ( Alg::Abs(axis.DistanceSquared() - 1) <= SF_MATH_EPSILON )
        {
            X = s * axis.x;
            Y = s * axis.y;
            Z = s * axis.z;
        }
        else
        {
            Point3<T> unitAxis = axis;
            unitAxis.Normalize();
            X = s * unitAxis.x;
            Y = s * unitAxis.y;
            Z = s * unitAxis.z;
        }
    }


private:

	// Quaternion Elements.
	T X, Y, Z, W;
};


//////////////////////////////////////////////////////////////////////////
// begin INLINE implementation
//////////////////////////////////////////////////////////////////////////


typedef Quaternion<float> QuaternionF;
typedef Quaternion<double> QuaternionD;

}}  // namespace Scaleform::Render

#endif //INC_SF_Render_Quaternion_H
