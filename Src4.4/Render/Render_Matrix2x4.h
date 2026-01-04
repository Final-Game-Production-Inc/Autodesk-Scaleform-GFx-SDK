/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Matrix2x4.h
Content     :   2D Matrix class
Created     :   November 3, 2010
Authors     :   Michael Antonov, Brendan Iribe, Maxim Shemanarev, Mustafa Thamer

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Matrix2x4_H
#define INC_SF_Render_Matrix2x4_H

#include "Kernel/SF_Alg.h"
#include "Kernel/SF_SIMD.h"

#include "Render/Render_Types2D.h"
#include "Kernel/SF_Math.h"

    // SF_ISFINITE() is provided in case the compiler lacks the standard version
#define SF_ISFINITE(x)  (sizeof(x) == sizeof(float) ? SF_ISFINITE_F(x) : SF_ISFINITE_D(x))
#define SF_ISFINITE_F(x) ((x) >= -FLT_MAX && (x) <= FLT_MAX)    // NAN should fail this?
#define SF_ISFINITE_D(x) ((x) >= -DBL_MAX && (x) <= DBL_MAX)

namespace Scaleform { namespace Render {

//
//    This matrix is composed of 2 rows and 4 columns with translation in the 4th column.
//    It is laid out in row-major order (rows are stored one after the other in memory)
//
//    This Matrix represents the matrix of type:
//
//    | sx      shx     02      tx |    // this is row #1 which has 4 values
//    | shy     sy      12      ty |    // this is row #2 which has 4 values
//
//    | 00      01     02       03 |
//    | 10      11     12       13 |
//
//    in memory it looks like this {00, 01, 02, 03, 10, 11, 12, 13},
//    ((float *)M)[4] = 10, and ((float *)M)[7] = 13, in general M[row][col] = M[row * 4 + col] (since 4 is the number of columns)
//
//    The rotational matrix (2x2) for this kind of matrix is
//    | cos   -sin |
//    | sin    cos |
//
//    Scaling matrix:
//    | sx     0  |
//    | 0      sy |
//
//    Shearing matrix:
//    | 0      shx |
//    | shy     0  |
//
//    Flash (flash.geom.Matrix) uses the matrix of type:
//    |    a   c   tx  |  ==  | sx  shx  tx |
//    |    b   d   ty  |  ==  | shy sy   ty |
//    where:   a = sx      (M[0][0])
//             b = shy     (M[1][0])
//             c = shx     (M[0][1])
//             d = sy      (M[1][1])
//             tx = Tx     (M[0][3])
//             ty = Ty     (M[1][3])
//
//    Basis vectors are the 1st 2 columns
//      B1 = {00, 10} X axis
//      B2 = {01, 11} Y axis
//

// Extract data into a base class template, so it can be specialized when needed for alignment.
template <typename T>
class Matrix2x4Data
{
public:
    static const unsigned Rows = 2;
    T   M[Rows][4];
};

template <typename T>
class Matrix2x4 : public Matrix2x4Data<T>
{
 public:
    using Matrix2x4Data<T>::M;  // GCC 3.4 compatibility.

    static Matrix2x4    Identity;

    enum NoInitType { NoInit };

    // Construct an identity matrix
    inline Matrix2x4();
    // Construct matrix with no initializer; later assignment expected.
    inline Matrix2x4(NoInitType) {  }

    // Construct a matrix by copying the passed matrix
    inline Matrix2x4(const Matrix2x4 &m);
    inline Matrix2x4(const T *d);
    inline Matrix2x4(const Matrix2x4 &m0, const Matrix2x4 &m1) { SetToAppend(m0, m1); }
    inline Matrix2x4(const Matrix2x4 &m0, const Matrix2x4 &m1, const Matrix2x4 &m2) { SetToAppend(m0, m1, m2); }
    // Construct a matrix using the passed values
    inline Matrix2x4(T v0_Sx, T v1_ShX, T v2_0, T v3_Tx, T v4_Shy, T v5_Sy, T v6_0, T v7_Ty);
    inline Matrix2x4(T v0_Sx, T v1_ShX, T v2_Tx, T v3_Shy, T v4_Sy, T v5_Ty);

    // element accessors
    T &Sx()     { return M[0][0]; }
    T &Shx()    { return M[0][1]; }
    T &Tx()     { return M[0][3]; }
    T &Shy()    { return M[1][0]; }
    T &Sy()     { return M[1][1]; }
    T &Ty()     { return M[1][3]; }

    // const element accessors
    const T &Sx()   const   { return M[0][0]; }
    const T &Shx()  const   { return M[0][1]; }
    const T &Tx()   const   { return M[0][3]; }
    const T &Shy()  const   { return M[1][0]; }
    const T &Sy()   const   { return M[1][1]; }
    const T &Ty()   const   { return M[1][3]; }

    // Set the matrix by copying the passed matrix
    inline void        SetMatrix(const Matrix2x4 &m);
    // Set the matrix using the passed values
    inline void        SetMatrix(T v0_Sx, T v1_ShX, T v2_0, T v3_Tx, T v4_Shy, T v5_Sy, T v6_0, T v7_Ty);
    inline void        SetMatrix(T v0_Sx, T v1_ShX, T v2_Tx, T v3_Shy, T v4_Sy, T v5_Ty);
    inline void        SetMatrix(const T *d);

    // Set the matrix to the identity matrix
    SF_EXPORT void     SetIdentity();
    // Set the matrix to all zero components
    SF_EXPORT void     SetZero();

    // Copy matrix to float rows[2][4]
    inline void        GetAsFloat2x4(float (*rows)[4]) const;

    // Set the matrix to the inverse of the passed matrix
    SF_EXPORT void     SetInverse(const Matrix2x4& m);
    // Return the inverse matrix
    inline Matrix2x4    GetInverse() const;

    // Set this matrix to a blend of m1 and m2, parameterized by t
    SF_EXPORT void     SetLerp(const Matrix2x4& m1, const Matrix2x4& m2, T t);

    // Reset back to the identity matrix
    inline void        Reset();

    // Swap the matrix values
    SF_EXPORT void     Swap(Matrix2x4 *pm);

    // Multiply this matrix by another one
    SF_EXPORT Matrix2x4&  Append        (const Matrix2x4& m);
    SF_EXPORT Matrix2x4&  Append_NonOpt (const Matrix2x4& m);
    SF_EXPORT Matrix2x4&  Prepend       (const Matrix2x4& m);

    // Initializes this matrix to the result of m0.Append(m1); more efficient
    // than assignment followed by Append.
    void     SetToAppend(const Matrix2x4& m0, const Matrix2x4& m1);
    void     SetToAppend_NonOpt(const Matrix2x4& m0, const Matrix2x4& m1);
    void     SetToAppend(const Matrix2x4& m0, const Matrix2x4& m1, const Matrix2x4& m2);
    void     SetToAppend_NonOpt(const Matrix2x4& m0, const Matrix2x4& m1, const Matrix2x4& m2);

    // Scaling, Shearing, Rotation, Translation operations
    inline Matrix2x4&  AppendScaling (T scale);
    inline Matrix2x4&  AppendScaling (T sx, T sy);
    inline Matrix2x4&  AppendScaling (Size<T> scale);
    inline Matrix2x4&  PrependScaling(T scale);
    inline Matrix2x4&  PrependScaling(T sx, T sy);
    inline Matrix2x4&  PrependScaling (Size<T> scale);

    inline Matrix2x4&  AppendShearing (T sh, T sv);
    inline Matrix2x4&  PrependShearing(T sh, T sv);

    inline Matrix2x4&  AppendRotation (T radians);
    inline Matrix2x4&  PrependRotation(T radians);

    inline Matrix2x4&  AppendTranslation (T sx, T sy);
    inline Matrix2x4&  PrependTranslation(T sx, T sy);
    inline Matrix2x4&  AppendTranslation (Point<T> pt) { return AppendTranslation(pt.x, pt.y); }
    inline Matrix2x4&  PrependTranslation(Point<T> pt) { return PrependTranslation(pt.x, pt.y); }

    // Matrices defined by parallelograms. Src and dst are
    // pointers to arrays of three points (T[6], x1,y1,...) that
    // identify three corners of the parallelograms assuming implicit
    // fourth point. The arguments are arrays of T[6] mapped
    // to x1,y1, x2,y2, x3,y3  where the coordinates are:
    //        *-----------------*
    //       /          (x3,y3)/
    //      /                 /
    //     /(x1,y1)   (x2,y2)/
    //    *-----------------*
    Matrix2x4& SetParlToParl(const T* src, const T* dst);
    Matrix2x4& SetRectToParl(T x1, T y1, T x2, T y2,
                            const T* parl);
    Matrix2x4& SetParlToRect(const T* parl,
                            T x1, T y1, T x2, T y2);
    Matrix2x4& SetRectToRect(T srcX1, T srcY1, T srcX2, T srcY2,
                            T dstX1, T dstY1, T dstX2, T dstY2);
    // Inverts the matrix
    inline Matrix2x4&      Invert();

    // Checks if all matrix values are within the -MAX..MAX value range
    SF_EXPORT bool        IsValid() const;

    // Return true if we flip handedness
    SF_EXPORT bool        DoesFlip() const;

    // Determinant of the 2x2 rotation/scale part only
    SF_EXPORT T       GetDeterminant() const;

    // Return the isotropic scale, proportional to the diagonal (0,0)-(1,1)
    SF_EXPORT T       GetScale() const;

    // Return the maximum scale factor that this transform applies
    SF_EXPORT T       GetMaxScale() const;

    // Set x scale
    SF_EXPORT void    SetXScale(T x);
    // Set y scale
    SF_EXPORT void    SetYScale(T y);

    // Return the magnitude scale of our x coord output
    SF_EXPORT T       GetXScale() const;
    // Return the magnitude scale of our y coord output
    SF_EXPORT T       GetYScale() const;
    // Return our rotation component (in radians)
    SF_EXPORT T       GetRotation() const;

    // Return the magnitude scale of our x coord output as double
    SF_EXPORT Double  GetXScaleDouble() const;
    // Return the magnitude scale of our y coord output as double
    SF_EXPORT Double  GetYScaleDouble() const;
    // Return our rotation component (in radians) as double
    SF_EXPORT Double  GetRotationDouble() const;

    // Check whether the matrix has arbitrary rotation,
    // that is, NOT a multiple of 90 degrees.
    inline bool        IsFreeRotation(T epsilon=1e-6f) const;

    // Decompose matrix into translation, scale, rotation and skew matrix.
    // If the columns of the matrix are orthogonal, the output matrix
    // should be the identity.  This follows the 2D matrix decomposition
    // in the W3 css working group document on css-transforms.
    inline void       Decompose(Point<T>* t, Point<T>*s, T* angle, Matrix2x4<T>* m) const;
    inline void       Recompose(const Point<T>& t, const Point<T>&s, T angle, const Matrix2x4<T>& m);
    inline void       Interpolate(const Matrix2x4<T>& a, const Matrix2x4<T>& b, T t);

    // *** Transform points

    // Transform the point by our matrix
    SF_EXPORT void		Transform(Point<T>* result, const Point<T>& p) const;
    inline Point<T>		Transform(const Point<T>& p) const;
    inline void			Transform(T* x, T* y) const;
    inline void			Transform2x2(T* x, T* y) const;

    // Transform the vector by our matrix, but does not apply translation
    SF_EXPORT void     TransformVector(Point<T>* result, const Point<T>& p) const;

    // Transform the point by the inverse of our matrix
    SF_EXPORT void     TransformByInverse(Point<T>* result, const Point<T>& p) const;
    inline Point<T>    TransformByInverse(const Point<T>& p) const;
    inline void        TransformByInverse(T* x, T* y) const;

    // Transform passed rectangle 'r' and return its bounding area
    SF_EXPORT void     EncloseTransform(Rect<T> *pr, const Rect<T>& r) const;
    SF_EXPORT void     EncloseTransform_NonOpt(Rect<T> *pr, const Rect<T>& r) const;
    Rect<T>            EncloseTransform(const Rect<T>& r) const;

#if defined(SF_ENABLE_SIMD) || ((defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG))
    void     SetToAppend_Opt(const Matrix2x4& m0, const Matrix2x4& m1, float* mask);
    void     SetToAppend_Opt(const Matrix2x4& m0, const Matrix2x4& m1, const Matrix2x4& m2, float* mask);
    SF_EXPORT Matrix2x4&  Append_Opt (const Matrix2x4& m, float* mask);
#endif

    // Formats Matrix2x4 message to a buffer, at least 512 bytes
    //SF_EXPORT void        Format(char *pbuffer) const;


    // *** Static matrix initializers

    // Creates a translation matrix
    inline static Matrix2x4        Translation(const Point<T>& p);
    inline static Matrix2x4        Translation(T dx, T dy);
    // Creates a scaling matrix
    inline static Matrix2x4        Scaling(T scale);
    inline static Matrix2x4        Scaling(T sx, T sy);
    inline static Matrix2x4        Scaling(Size<T> scale);
    // Creates a shearing matrix
    inline static Matrix2x4        Shearing(T sh, T sv);
    // Creates a rotation matrix
    inline static Matrix2x4        Rotation(T radians);


    // *** Operator overrides

    // Assignment
    inline const Matrix2x4&            operator =  (const Matrix2x4 &m)
	{
		SetMatrix(m);
		return *this;
	}
    inline const Matrix2x4&            operator *= (const Matrix2x4 &m)
	{
		return Prepend(m);
	}

    // Matrix equality
    inline friend bool                operator == (const Matrix2x4 &m1, const Matrix2x4 &m2)
	{
        return memcmp(m1.M, m2.M, sizeof(Matrix2x4<T>)) == 0;
	}
    inline friend bool                operator != (const Matrix2x4 &m1, const Matrix2x4 &m2)
	{
		return  !(m1 == m2);
	}

    // Matrix multiplication (transform values)
    // - note that Matrix2x4 multiplication is not commutative
    inline friend const Matrix2x4      operator * (const Matrix2x4 &m1, const Matrix2x4 &m2)
	{
		return Matrix2x4<T>(m1).Prepend(m2);
	}
    inline friend const PointF        operator * (const Matrix2x4 &m,  const PointF &pt)
	{
		return m.Transform(pt);
	}
    inline friend const RectF         operator * (const Matrix2x4 &m,  const RectF &r)
	{
		return m.EncloseTransform(r);
	}

private:
	static double _cos(double arg)
	{
		return cos(arg);
	}

	static float _cos(float arg)
	{
		return cosf(arg);
	}

	static double _sin(double arg)
	{
		return sin(arg);
	}

	static float _sin(float arg)
	{
		return sinf(arg);
	}

	static double _tan(double arg)
	{
		return tan(arg);
	}

	static float _tan(float arg)
	{
		return tanf(arg);
	}

	static double _abs(double arg)
	{
		return fabs(arg);
	}

	static float _abs(float arg)
	{
		return fabsf(arg);
	}
	static float _atan2(float arg1, float arg2)
	{
		return atan2f(arg1, arg2);
	}
	static double _atan2(double arg1, double arg2)
	{
		return atan2(arg1, arg2);
	}
	static float _sqrt(float arg)
	{
		return sqrtf(arg);
	}
	static double _sqrt(double arg)
	{
		return sqrt(arg);
	}
};


template<typename T>
Matrix2x4<T>   Matrix2x4<T>::Identity;
// ** Inline Implementation

template<typename T>
inline Matrix2x4<T>::Matrix2x4()
{
    SetIdentity();
}

template<typename T>
inline Matrix2x4<T>::Matrix2x4(const Matrix2x4<T> &m)
{
    SetMatrix(m);
}

template<typename T>
inline Matrix2x4<T>::Matrix2x4(const T *d)
{
    SetMatrix(d);
}

template<typename T>
inline Matrix2x4<T>::Matrix2x4(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7)
{
    SetMatrix(v0,v1,v2,v3,v4,v5,v6,v7);
}

template<typename T>
inline Matrix2x4<T>::Matrix2x4(T v0, T v1, T v2, T v3, T v4, T v5)
{
    SetMatrix(v0,v1,v2,v3,v4,v5);
}

template<typename T>
inline void    Matrix2x4<T>::SetMatrix(const Matrix2x4<T> &m)
{
    M[0][0] = m.M[0][0];
    M[0][1] = m.M[0][1];
    M[0][2] = m.M[0][2];
    M[0][3] = m.M[0][3];
    M[1][0] = m.M[1][0];
    M[1][1] = m.M[1][1];
    M[1][2] = m.M[1][2];
    M[1][3] = m.M[1][3];
}

template<typename T>
inline void    Matrix2x4<T>::SetMatrix(T v0_Sx, T v1_ShX, T v2_0, T v3_Tx, T v4_Shy, T v5_Sy, T v6_0, T v7_Ty)
{
    M[0][0] = v0_Sx;
    M[0][1] = v1_ShX;
    M[0][2] = v2_0;
    M[0][3] = v3_Tx;
    M[1][0] = v4_Shy;
    M[1][1] = v5_Sy;
    M[1][2] = v6_0;
    M[1][3] = v7_Ty;
}

template<typename T>
inline void    Matrix2x4<T>::SetMatrix(T v0_Sx, T v1_ShX, T v2_Tx, T v3_Shy, T v4_Sy, T v5_Ty)
{
    M[0][0] = v0_Sx;
    M[0][1] = v1_ShX;
    M[0][2] = 0;
    M[0][3] = v2_Tx;
    M[1][0] = v3_Shy;
    M[1][1] = v4_Sy;
    M[1][2] = 0;
    M[1][3] = v5_Ty;
}

template<typename T>
inline void    Matrix2x4<T>::SetMatrix(const T *d)
{
    M[0][0] = d[0];
    M[0][1] = d[1];
    M[0][2] = d[2];
    M[0][3] = d[3];
    M[1][0] = d[4];
    M[1][1] = d[5];
    M[1][2] = d[6];
    M[1][3] = d[7];
}

template<typename T>
inline void Matrix2x4<T>::GetAsFloat2x4(float (*rows)[4]) const
{
    rows[0][0] = (float)M[0][0];
    rows[0][1] = (float)M[0][1];
    rows[0][2] = (float)M[0][2];
    rows[0][3] = (float)M[0][3];

    rows[1][0] = (float)M[1][0];
    rows[1][1] = (float)M[1][1];
    rows[1][2] = (float)M[1][2];
    rows[1][3] = (float)M[1][3];
}

template<typename T>
inline void    Matrix2x4<T>::Reset()
{
    SetIdentity();
}

template<typename T>
inline void    Matrix2x4<T>::Swap(Matrix2x4 *pm)
{
    Scaleform::Alg::Swap<T>(M[0][0], pm->M[0][0]);
    Scaleform::Alg::Swap<T>(M[0][1], pm->M[0][1]);
    Scaleform::Alg::Swap<T>(M[0][2], pm->M[0][2]);
    Scaleform::Alg::Swap<T>(M[0][3], pm->M[0][3]);
    Scaleform::Alg::Swap<T>(M[1][0], pm->M[1][0]);
    Scaleform::Alg::Swap<T>(M[1][1], pm->M[1][1]);
    Scaleform::Alg::Swap<T>(M[1][2], pm->M[1][2]);
    Scaleform::Alg::Swap<T>(M[1][3], pm->M[1][3]);
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::Invert()
{
    SetInverse( Matrix2x4<T>(*this) );
    return *this;
}

template<typename T>
inline Matrix2x4<T>       Matrix2x4<T>::GetInverse() const
{
    Matrix2x4<T> m;
    m.SetInverse( *this );
    return m;
}

template<typename T>
inline Point<T> Matrix2x4<T>::Transform(const Point<T>& p) const
{
    Point<T> pt;
    Transform(&pt, p);
    return pt;
}

template<typename T>
inline Point<T> Matrix2x4<T>::TransformByInverse(const Point<T>& p) const
{
    Point<T> pt;
    TransformByInverse(&pt, p);
    return pt;
}

template<typename T>
inline void Matrix2x4<T>::Transform(T* x, T* y) const
{
    T tx = *x;
    T ty = *y;
    *x = M[0][0] * tx + M[0][1] * ty + M[0][3];
    *y = M[1][0] * tx + M[1][1] * ty + M[1][3];
}

template<typename T>
inline void Matrix2x4<T>::Transform2x2(T* x, T* y) const
{
    T tx = *x;
    T ty = *y;
    *x = M[0][0] * tx + M[0][1] * ty;
    *y = M[1][0] * tx + M[1][1] * ty;
}

template<typename T>
inline void Matrix2x4<T>::TransformByInverse(T* x, T* y) const
{
    Matrix2x4<T>(*this).Invert().Transform(x, y);
}

template<typename T>
inline Rect<T>  Matrix2x4<T>::EncloseTransform(const Rect<T>& r) const
{
    Rect<T> d(Rect<T>::NoInit);
    EncloseTransform(&d, r);
    return d;
}

template<typename T>
inline Matrix2x4<T>   Matrix2x4<T>::Translation(const Point<T>& p)
{
    return Matrix2x4<T>(1.0, 0.0, 0.0, p.x,
                        0.0, 1.0, 0.0, p.y);
}

template<typename T>
inline Matrix2x4<T>       Matrix2x4<T>::Translation(T dx, T dy)
{
    return Matrix2x4<T>(1.0, 0.0, 0.0, dx,
                        0.0, 1.0, 0.0, dy);
}

template<typename T>
inline Matrix2x4<T>       Matrix2x4<T>::Scaling(T s)
{
    return Matrix2x4<T>(s,   0.0, 0.0, 0.0,
                        0.0, s,   0.0, 0.0);
}

template<typename T>
inline Matrix2x4<T>       Matrix2x4<T>::Scaling(T sx, T sy)
{
    return Matrix2x4<T>(sx,  0.0, 0.0, 0.0,
                        0.0, sy,  0.0, 0.0);
}

template<typename T>
inline Matrix2x4<T>       Matrix2x4<T>::Scaling(Size<T> scale)
{
    return Scaling(scale.Width, scale.Height);
}

template<typename T>
inline Matrix2x4<T>       Matrix2x4<T>::Shearing(T sh, T sv)
{
    return Matrix2x4<T>(1.0,      _tan(sv), 0.0, 0.0,
                        tan(sh), 1.0,       0.0, 0.0);
}

template<typename T>
inline Matrix2x4<T>       Matrix2x4<T>::Rotation(T r)
{
    T cosa = _cos(r);
    T sina = _sin(r);
    return Matrix2x4<T>(cosa, -sina, 0.0, 0.0,
                        sina,  cosa, 0.0, 0.0);
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::PrependScaling(T sx, T sy)
{
    return Prepend(Scaling(sx, sy));
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::PrependScaling(T scale)
{
    return PrependScaling(scale, scale);
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::PrependScaling (Size<T> scale)
{
    return PrependScaling (scale.Width, scale.Height);
}


template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::AppendScaling (T sx, T sy)
{
    M[0][0] *= sx;
    M[0][1] *= sx;
    M[0][2] *= sx;
    M[0][3] *= sx;
    M[1][0] *= sy;
    M[1][1] *= sy;
    M[1][2] *= sy;
    M[1][3] *= sy;
    return *this;
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::AppendScaling (T scale)
{
    return AppendScaling (scale, scale);
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::AppendScaling (Size<T> scale)
{
    return AppendScaling (scale.Width, scale.Height);
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::PrependShearing(T sh, T sv)
{
    return Prepend(Shearing(sh, sv));
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::AppendShearing (T sh, T sv)
{
    return Append (Shearing(sh, sv));
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::PrependRotation(T radians)
{
    return Prepend(Rotation(radians));
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::AppendRotation (T radians)
{
    return Append (Rotation(radians));
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::PrependTranslation(T dx, T dy)
{
    M[0][3] += M[0][0] * dx + M[0][1] * dy;
    M[1][3] += M[1][0] * dx + M[1][1] * dy;
    return *this;
}

template<typename T>
inline Matrix2x4<T>&      Matrix2x4<T>::AppendTranslation (T dx, T dy)
{
    M[0][3] += dx;
    M[1][3] += dy;
    return *this;
}

template<typename T>
inline bool Matrix2x4<T>::IsFreeRotation(T epsilon) const
{
    Matrix2x4<T> mat2(*this);
    mat2.M[0][3] = 0;
    mat2.M[1][3] = 0;
    Point<T> p10 = mat2.Transform(Point<T>(1,0));
    return _abs(p10.x) > epsilon && _abs(p10.y) > epsilon;
}

template<typename T>
bool    Matrix2x4<T>::IsValid() const
{
    return SF_ISFINITE(M[0][0])
        && SF_ISFINITE(M[0][1])
        && SF_ISFINITE(M[0][2])
        && SF_ISFINITE(M[0][3])
        && SF_ISFINITE(M[1][0])
        && SF_ISFINITE(M[1][1])
        && SF_ISFINITE(M[1][2])
        && SF_ISFINITE(M[1][3]);
}

template<typename T>
void    Matrix2x4<T>::Decompose(Point<T>* t, Point<T>* s, T* angle, Matrix2x4<T>* k) const
{
    if (!t || !s || !angle || !k)
        return;

    // Decompose Matrix as M = T * R * K * S

    // Set K to M, then decompose until we are left with K.
    k->SetMatrix(*this);

    // Find the translation components
    t->x = Tx();
    t->y = Ty();

    // T^(-1) M = R * K * S
    k->Tx()  = 0;
    k->Ty()  = 0;

    // Find the scale factors
    if ( DoesFlip() )
    {
        if (Sx() < Sy())
        {
            s->x = -GetXScale();
            s->y =  GetYScale();
        }
        else
        {
            s->x =  GetXScale();
            s->y = -GetYScale();
        }
    }
    else
    {
        s->x = GetXScale();
        s->y = GetYScale();
    }

    // T^(-1) M S^(-1) = R * K
    if ( SF_MATH_EPSILON <= _abs(s->x) )
    {
        k->SetXScale( (T)1 );
    }
    
    if ( SF_MATH_EPSILON <= _abs(s->y) )
    {
        k->SetYScale( (T)1 );
    }

    // Find the Rotation
    *angle = GetRotation();

    // R^(-1) T^(-1) M S^(-1) = K
    if ( SF_MATH_EPSILON <= _abs(angle) )
    {
        k->AppendRotation( -(*angle) );
    }
}

template <typename T>
void    Matrix2x4<T>::Interpolate(const Matrix2x4<T>& a, const Matrix2x4<T>& b, T t)
{
    Point<T> Sa, Sb, Ta, Tb;
    T angleA, angleB;
    Matrix2x4<T> ka, kb;

    a.Decompose(Ta, Sa, angleA, ka);
    b.Decompose(Tb, Sb, angleB, kb);

    // Modify scales to match signs
    if (Sa.x < 0 && Sa.y < 0)
        Sa = -Sa;
    if (Sb.x < 0 && Sb.y < 0)
        Sb = -Sb;

    if ((Sa.x < 0 && Sb.y < 0) || (Sa.y < 0 && Sb.x < 0))
    {
        Sa = -Sa;
        angleA += (angleA <= 0 ? SF_MATH_PI : -SF_MATH_PI);
    }

    // Modify angleA and angleB to be within 180 degrees

    // The following is in the CSS spec, but seems superfluous.
    // Am I just missing something?
    //if (_abs(angleA) < SF_MATH_EPSILON)
    //    angleA = static_cast<T>(2.0*SF_MATH_PI);
    //
    //if (_abs(angleB) < SF_MATH_EPSILON)
    //    angleB = static_cast<T>(2.0*SF_MATH_PI);

    if (SF_MATH_PI < _abs(angleA - angleB))
    {
        if (angleB < angleA)
            if (angleB < SF_MATH_EPSILON)
                angleB += static_cast<T>(2.0*SF_MATH_PI);
            else
                angleA -= static_cast<T>(2.0*SF_MATH_PI);
        else
            if (angleA < SF_MATH_EPSILON)
                angleA += static_cast<T>(2.0*SF_MATH_PI);
            else
                angleB -= static_cast<T>(2.0*SF_MATH_PI);
    }

    Recompose(SetLerp(Ta, Tb, t), SetLerp(Sa, Sb, t), SetLerp(angleA, angleB, t), SetLerp(ka, kb, t));
}

template<typename T>
void    Matrix2x4<T>::Recompose(const Point<T>& t, const Point<T>&s, T angle, const Matrix2x4<T>& k)
{
    // Recompose the Matrix = T * R * K * S
    SetMatrix(k);
    PrependScaling(s.x, s.y);
    AppendRotation(angle);
    AppendTranslation(t.x, t.y);
}

// Set the Matrix2x4 to identity.
template<typename T>
void    Matrix2x4<T>::SetIdentity()
{
    M[0][0] = 1.0f;
    M[0][1] = 0.0f;
    M[0][2] = 0.0f;
    M[0][3] = 0.0f;
    M[1][0] = 0.0f;
    M[1][1] = 1.0f;
    M[1][2] = 0.0f;
    M[1][3] = 0.0f;
}

// Set the Matrix2x4 to identity.
template<typename T>
void Matrix2x4<T>::SetZero()
{
    M[0][0] = 0.0f;
    M[0][1] = 0.0f;
    M[0][2] = 0.0f;
    M[0][3] = 0.0f;
    M[1][0] = 0.0f;
    M[1][1] = 0.0f;
    M[1][2] = 0.0f;
    M[1][3] = 0.0f;
}

// Concatenate m and the current matrix.  When transforming points,
// m happens first, then the original transformation.
template<typename T>
Matrix2x4<T>&  Matrix2x4<T>::Prepend(const Matrix2x4<T>& m)
{
    Matrix2x4<T> t = *this;
    M[0][0] = t.M[0][0] * m.M[0][0] + t.M[0][1] * m.M[1][0];
    M[1][0] = t.M[1][0] * m.M[0][0] + t.M[1][1] * m.M[1][0];
    M[0][1] = t.M[0][0] * m.M[0][1] + t.M[0][1] * m.M[1][1];
    M[1][1] = t.M[1][0] * m.M[0][1] + t.M[1][1] * m.M[1][1];
    M[0][2] = M[1][2] = 0;
    M[0][3] = t.M[0][0] * m.M[0][3] + t.M[0][1] * m.M[1][3] + t.M[0][3];
    M[1][3] = t.M[1][0] * m.M[0][3] + t.M[1][1] * m.M[1][3] + t.M[1][3];
    return *this;
}

// Initializes this matrix to the result of m0.Append(m1); more efficient
// then assignment followed by Append.
template<typename T>
void Matrix2x4<T>::SetToAppend(const Matrix2x4<T>& m0, const Matrix2x4<T>& m1)
{
    // Call non-optimized version, in non-specialized case.
    SetToAppend_NonOpt(m0, m1);
}

template<typename T>
void Matrix2x4<T>::SetToAppend_NonOpt(const Matrix2x4<T>& m0, const Matrix2x4<T>& m1)
{
    M[0][0] = m1.M[0][0] * m0.M[0][0] + m1.M[0][1] * m0.M[1][0];
    M[0][1] = m1.M[0][0] * m0.M[0][1] + m1.M[0][1] * m0.M[1][1];
    M[0][2] = 0;
    M[0][3] = m1.M[0][0] * m0.M[0][3] + m1.M[0][1] * m0.M[1][3] + m1.M[0][3];

    M[1][0] = m1.M[1][0] * m0.M[0][0] + m1.M[1][1] * m0.M[1][0];
    M[1][1] = m1.M[1][0] * m0.M[0][1] + m1.M[1][1] * m0.M[1][1];
    M[1][2] = 0;
    M[1][3] = m1.M[1][0] * m0.M[0][3] + m1.M[1][1] * m0.M[1][3] + m1.M[1][3];
}

template<typename T>
void Matrix2x4<T>::SetToAppend(const Matrix2x4<T>& m0, const Matrix2x4<T>& m1,
                               const Matrix2x4<T>& m2)
{
    // Call non-optimized version, in non-specialized case.
    SetToAppend_NonOpt(m0, m1, m2);
}

template<typename T>
void Matrix2x4<T>::SetToAppend_NonOpt(const Matrix2x4<T>& m0,
                               const Matrix2x4<T>& m1,
                               const Matrix2x4<T>& m2)
{
    // First Prepend multiply: m1 * m2.
    T m00 = m2.M[0][0] * m1.M[0][0] + m2.M[0][1] * m1.M[1][0];
    T m01 = m2.M[0][0] * m1.M[0][1] + m2.M[0][1] * m1.M[1][1];
    //    T m02 = 0;
    T m03 = m2.M[0][0] * m1.M[0][3] + m2.M[0][1] * m1.M[1][3] + m2.M[0][3];

    T m10 = m2.M[1][0] * m1.M[0][0] + m2.M[1][1] * m1.M[1][0];
    T m11 = m2.M[1][0] * m1.M[0][1] + m2.M[1][1] * m1.M[1][1];
    //    T m12 = 0;
    T m13 = m2.M[1][0] * m1.M[0][3] + m2.M[1][1] * m1.M[1][3] + m2.M[1][3];

    // Second append multiply.
    M[0][0] = m00 * m0.M[0][0] + m01 * m0.M[1][0];
    M[0][1] = m00 * m0.M[0][1] + m01 * m0.M[1][1];
    M[0][2]  = 0;
    M[0][3] = m00 * m0.M[0][3] + m01 * m0.M[1][3] + m03;

    M[1][0] = m10 * m0.M[0][0] + m11 * m0.M[1][0];
    M[1][1] = m10 * m0.M[0][1] + m11 * m0.M[1][1];
    M[1][2] = 0;
    M[1][3] = m10 * m0.M[0][3] + m11 * m0.M[1][3] + m13;
}



// Concatenate the current matrix and m.  When transforming points,
// the original transformation happens first, then m.
template<typename T>
Matrix2x4<T>&  Matrix2x4<T>::Append(const Matrix2x4<T>& m)
{
    return Append_NonOpt(m);
}

template<typename T>
Matrix2x4<T>&  Matrix2x4<T>::Append_NonOpt(const Matrix2x4<T>& m)
{
    Matrix2x4 t = *this;
    M[0][0] = m.M[0][0] * t.M[0][0] + m.M[0][1] * t.M[1][0];
    M[1][0] = m.M[1][0] * t.M[0][0] + m.M[1][1] * t.M[1][0];
    M[0][1] = m.M[0][0] * t.M[0][1] + m.M[0][1] * t.M[1][1];
    M[1][1] = m.M[1][0] * t.M[0][1] + m.M[1][1] * t.M[1][1];
    M[0][2] = 0;
    M[1][2] = 0;
    M[0][3] = m.M[0][0] * t.M[0][3] + m.M[0][1] * t.M[1][3] + m.M[0][3];
    M[1][3] = m.M[1][0] * t.M[0][3] + m.M[1][1] * t.M[1][3] + m.M[1][3];
    return *this;
}

// Set this Matrix2x4 to a blend of m1 and m2, parameterized by t.
template<typename T>
void    Matrix2x4<T>::SetLerp(const Matrix2x4<T>& m1, const Matrix2x4<T>& m2, T t)
{
    M[0][0] = Alg::Lerp<T>(m1.M[0][0], m2.M[0][0], t);
    M[1][0] = Alg::Lerp<T>(m1.M[1][0], m2.M[1][0], t);
    M[0][1] = Alg::Lerp<T>(m1.M[0][1], m2.M[0][1], t);
    M[1][1] = Alg::Lerp<T>(m1.M[1][1], m2.M[1][1], t);
    M[0][2] = Alg::Lerp<T>(m1.M[0][2], m2.M[0][2], t);
    M[1][2] = Alg::Lerp<T>(m1.M[1][2], m2.M[1][2], t);
    M[0][3] = Alg::Lerp<T>(m1.M[0][3], m2.M[0][3], t);
    M[1][3] = Alg::Lerp<T>(m1.M[1][3], m2.M[1][3], t);
}


// Transform PointF 'p' by our Matrix2x4.  Put the result in *result.
template<typename T>
void    Matrix2x4<T>::Transform(Point<T>* result, const Point<T>& p) const
{
    SF_ASSERT(result);

    result->x = M[0][0] * p.x + M[0][1] * p.y + M[0][3];
    result->y = M[1][0] * p.x + M[1][1] * p.y + M[1][3];
}

// Transform vector 'v' by our Matrix2x4. Doesn't apply translation.
// Put the result in *result.
template<typename T>
void    Matrix2x4<T>::TransformVector(Point<T>* result, const Point<T>& v) const
{
    SF_ASSERT(result);

    result->x = M[0][0] * v.x + M[0][1] * v.y;
    result->y = M[1][0] * v.x + M[1][1] * v.y;
}

// Transform PointF 'p' by the inverse of our Matrix2x4.  Put result in *result.
template<typename T>
void    Matrix2x4<T>::TransformByInverse(Point<T>* result, const Point<T>& p) const
{
    Matrix2x4<T>(*this).Invert().Transform(result, p);
}


// Set this Matrix2x4 to the inverse of the given Matrix2x4.
template<typename T>
void    Matrix2x4<T>::SetInverse(const Matrix2x4& m)
{
    SF_ASSERT(this != &m);

    // Invert the rotation part.
    T   det = m.M[0][0] * m.M[1][1] - m.M[0][1] * m.M[1][0];
    if (det == 0.0f)
    {
        // Not invertible - this happens sometimes (ie. sample6.Swf)

        // Arbitrary fallback
        SetIdentity();
        M[0][3] = -m.M[0][3];
        M[1][3] = -m.M[1][3];
    }
    else
    {
        T   invDet = 1.0f / det;
        M[0][0] =  m.M[1][1] * invDet;
        M[1][1] =  m.M[0][0] * invDet;
        M[0][1] = -m.M[0][1] * invDet;
        M[1][0] = -m.M[1][0] * invDet;

        M[0][3] = -(M[0][0] * m.M[0][3] + M[0][1] * m.M[1][3]);
        M[1][3] = -(M[1][0] * m.M[0][3] + M[1][1] * m.M[1][3]);
    }
}


// Return true if this Matrix2x4 reverses handedness.
template<typename T>
bool    Matrix2x4<T>::DoesFlip() const
{
    T   det = M[0][0] * M[1][1] - M[0][1] * M[1][0];
    return det < 0.F;
}


// Return the determinant of the 2x2 rotation/scale part only.
template<typename T>
T   Matrix2x4<T>::GetDeterminant() const
{
    return M[0][0] * M[1][1] - M[1][0] * M[0][1];
}

// Return the maximum scale factor that this transform
// applies.  For assessing scale, when determining acceptable
// errors in tessellation.
template<typename T>
T   Matrix2x4<T>::GetMaxScale() const
{
    // @@ take the max length of the two basis vectors.
    T   basis0Length2 = M[0][0] * M[0][0] + M[1][0] * M[1][0];
    T   basis1Length2 = M[0][1] * M[0][1] + M[1][1] * M[1][1];
    T   maxLength2 = Alg::Max<T>(basis0Length2, basis1Length2);
    return _sqrt(maxLength2);
}

// Set x scale
template<typename T>
void    Matrix2x4<T>::SetXScale(T s)
{
    s /= GetXScale();
    M[0][0] *= s;
    M[1][0] *= s;
}

// Set y scale
template<typename T>
void    Matrix2x4<T>::SetYScale(T s)
{
    s /= GetYScale();
    M[0][1] *= s;
    M[1][1] *= s;
}

template<typename T>
T  Matrix2x4<T>::GetXScale() const
{
    return _sqrt((M[0][0]) * (M[0][0]) + (M[1][0]) * (M[1][0]));
}

template<typename T>
T  Matrix2x4<T>::GetYScale() const
{
    return _sqrt((M[1][1]) * (M[1][1]) + (M[0][1]) * (M[0][1]));
}

template<typename T>
T  Matrix2x4<T>::GetRotation() const
{
    return _atan2((M[1][0]), (M[0][0]));
}

template<typename T>
Double  Matrix2x4<T>::GetXScaleDouble() const
{
    return _sqrt(Double(M[0][0]) * Double(M[0][0]) + Double(M[1][0]) * Double(M[1][0]));
}

template<typename T>
Double Matrix2x4<T>::GetYScaleDouble() const
{
    return _sqrt(Double(M[1][1]) * Double(M[1][1]) + Double(M[0][1]) * Double(M[0][1]));
}

template<typename T>
Double  Matrix2x4<T>::GetRotationDouble() const
{
    return _atan2(Double(M[1][0]), Double(M[0][0]));
}


template<typename T>
T Matrix2x4<T>::GetScale() const
{
    T x = 0.707106781f * M[0][0] + 0.707106781f * M[0][1];
    T y = 0.707106781f * M[1][0] + 0.707106781f * M[1][1];
    return _sqrt(x*x + y*y);
}


// This is an axial bound of an oriented (and/or
// sheared, scaled, etc) box.
template<typename T>
void    Matrix2x4<T>::EncloseTransform(Rect<T> *pr, const Rect<T>& r) const
{
    // Call non-optimized version, in non-specialized case.
    EncloseTransform_NonOpt(pr, r);
}

template<typename T>
void    Matrix2x4<T>::EncloseTransform_NonOpt(Rect<T> *pr, const Rect<T>& r) const
{
    /*	// Get the transformed bounding box.
    Point<T> p0, p1, p2, p3;
    Transform(&p0, r.TopLeft());
    Transform(&p1, r.TopRight());
    Transform(&p2, r.BottomRight());
    Transform(&p3, r.BottomLeft());

    pr->SetRect(p0, p0);
    pr->ExpandToPoint(p1);
    pr->ExpandToPoint(p2);
    pr->ExpandToPoint(p3);*/

    // Inline/optimized since EncloseTransform is
    // performance-critical in new renderer.
    Point<T> p0, p1, p2, p3;

    // TopLeft
    p0.x = M[0][0] * r.x1 + M[0][1] * r.y1;
    p0.y = M[1][0] * r.x1 + M[1][1] * r.y1;
    // TopRight
    p1.x = M[0][0] * r.x2 + M[0][1] * r.y1;
    p1.y = M[1][0] * r.x2 + M[1][1] * r.y1;
    // Bottom Right
    p2.x = M[0][0] * r.x2 + M[0][1] * r.y2;
    p2.y = M[1][0] * r.x2 + M[1][1] * r.y2;
    // Bottom Left
    p3.x = M[0][0] * r.x1 + M[0][1] * r.y2;
    p3.y = M[1][0] * r.x1 + M[1][1] * r.y2;

    T xmin1, xmin2, xmax1, xmax2;
    T ymin1, ymin2, ymax1, ymax2;

    if (p0.x < p1.x) { xmin1 = p0.x; xmax1 = p1.x; }
    else             { xmin1 = p1.x; xmax1 = p0.x; }
    if (p2.x < p3.x) { xmin2 = p2.x; xmax2 = p3.x; }
    else             { xmin2 = p3.x; xmax2 = p2.x; }

    if (p0.y < p1.y) { ymin1 = p0.y; ymax1 = p1.y; }
    else             { ymin1 = p1.y; ymax1 = p0.y; }
    if (p2.y < p3.y) { ymin2 = p2.y; ymax2 = p3.y; }
    else             { ymin2 = p3.y; ymax2 = p2.y; }

    T xmin = (xmin1 < xmin2) ? xmin1 : xmin2;
    T xmax = (xmax1 > xmax2) ? xmax1 : xmax2;
    T ymin = (ymin1 < ymin2) ? ymin1 : ymin2;
    T ymax = (ymax1 > ymax2) ? ymax1 : ymax2;

    xmin += M[0][3];
    xmax += M[0][3];
    ymin += M[1][3];
    ymax += M[1][3];
    pr->SetRect(xmin, ymin, xmax, ymax);
}

//-------------------
// The code of this functions was taken from the Anti-Grain Geometry
// Project and modified for the use by Scaleform/Autodesk.
// Permission to use without restrictions is hereby granted to
// Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
// See http://antigrain.com for details.
//------------------------------------------------------------------------
template<typename T>
Matrix2x4<T>& Matrix2x4<T>::SetParlToParl(const T* src, const T* dst)
{
    SetMatrix(src[2] - src[0], src[4] - src[0],  0, src[0],
              src[3] - src[1], src[5] - src[1], 0, src[1]);
    Matrix2x4 d(dst[2] - dst[0], dst[4] - dst[0], 0, dst[0],
                dst[3] - dst[1], dst[5] - dst[1], 0, dst[1]);
    Invert();
    Append(d);
    return *this;
}

//------------------------------------------------------------------------
template<typename T>
Matrix2x4<T>& Matrix2x4<T>::SetRectToParl(T x1, T y1, T x2, T y2,
                                          const T* parl)
{
    T src[6];
    src[0] = x1; src[1] = y1;
    src[2] = x2; src[3] = y1;
    src[4] = x2; src[5] = y2;
    return SetParlToParl(src, parl);
}

//------------------------------------------------------------------------
template<typename T>
Matrix2x4<T>& Matrix2x4<T>::SetParlToRect(const T* parl,
                                          T x1, T y1, T x2, T y2)
{
    T dst[6];
    dst[0] = x1; dst[1] = y1;
    dst[2] = x2; dst[3] = y1;
    dst[4] = x2; dst[5] = y2;
    return SetParlToParl(parl, dst);
}

//------------------------------------------------------------------------
template<typename T>
Matrix2x4<T>& Matrix2x4<T>::SetRectToRect(T srcX1, T srcY1, T srcX2, T srcY2,
                                          T dstX1, T dstY1, T dstX2, T dstY2)
{
	T src[6];
	T dst[6];
	src[0] = srcX1; src[1] = srcY1;
	src[2] = srcX2; src[3] = srcY1;
	src[4] = srcX2; src[5] = srcY2;
	dst[0] = dstX1; dst[1] = dstY1;
	dst[2] = dstX2; dst[3] = dstY1;
	dst[4] = dstX2; dst[5] = dstY2;
	return SetParlToParl(src, dst);
}

// ** End Inline Implementation

typedef Matrix2x4<float> Matrix2F;

#ifdef SF_ENABLE_SIMD

// Alignment specialization for class data.
template<>
class Matrix2x4Data<float>
{
public:
    static const unsigned Rows = 2;
    SF_SIMD_ALIGN( float M[Rows][4] );
};
#endif

#if defined(SF_ENABLE_SIMD) || ((defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG))
// Now pre-declare any explicit specializations found in source files.
template<> void Matrix2F::SetToAppend( const Matrix2F & m0, const Matrix2F & m1 );
template<> void Matrix2F::SetToAppend( const Matrix2F & m0, const Matrix2F & m1, const Matrix2F & m2 );
template<> Matrix2F& Matrix2F::Append( const Matrix2F & m );
#endif

#if defined(SF_ENABLE_SIMD)
template<> void Matrix2F::EncloseTransform(RectF *pr, const RectF& r) const;
#endif

}} // Scaleform<T>::Render

#endif      // INC_SF_Render_Matrix2x4_H
