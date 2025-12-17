/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Matrix4x4.h
Content     :   3D Matrix class
Created     :   November 15, 2010
Authors     :   Mustafa Thamer

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Matrix4x4_H
#define INC_SF_Render_Matrix4x4_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Math.h"
#include "Render_Types2D.h"
#include "Render_Matrix2x4.h"
#include "Render_Matrix3x4.h"
#include "Render_Point3.h"

#include <string.h>     // memcpy

namespace Scaleform { namespace Render {


    //
    //    This matrix is composed of 4 rows and 4 columns with translation in the 4th column.
    //    It is laid out in row-major order (rows are stored one after the other in memory)
    //    The data is in the same format as Matrix2x4 and Matrix3x4.
    //    Note: this matrix is TRANSPOSED from the Flash Geom Matrix3D class.
    //
    //    Column vectors are used when applying matrix multiplications.
    //    A vector is represented as a single column, 4-row matrix.
    //
    //    TRANSFORMATIONS HAPPEN RIGHT TO LEFT, so M3 * M2 * M1 * V
    //    means that Vector V is transformed by M1 then M2 then M3.
    //    This is the same as OpenGL but opposite of Direct3D
    //
    //    This Matrix represents the matrix of type:
    //
    //    | sx      01      02      tx |    // this is row #1 which has 4 values
    //    | 10      sy      12      ty |    // this is row #2 which has 4 values
    //    | 20      21      sz      tz |    // this is row #3 which has 4 values
    //    | 30      31      32      tw |    // this is row #4 which has 4 values
    //
    //    | 00      01     02       03 |
    //    | 10      11     12       13 |
    //    | 20      21     22       23 |
    //    | 30      31     32       33 |
    //
    //    in memory it looks like this {00, 01, 02, 03,  10, 11, 12, 13,  20, 21, 22, 23,  30, 31, 32, 33},
    //    ((float *)M)[4] = 10, and ((float *)M)[7] = 13, in general M[row][col] = M[row * 4 + col]
    //          (since 4 is the number of columns)
    //
    //    Basis vectors are the 1st 3 columns
    //      B1 = {00, 10, 20, 30} X axis
    //      B2 = {01, 11, 21, 31} Y axis
    //      B3 = {02, 12, 22, 32} Z axis
    //

// Extract data into a base class template, so it can be specialized when needed for alignment.
template <typename T>
class Matrix4x4Data
{
public:
    static const unsigned Rows = 4;
    T   M[Rows][4];
};

#ifdef SF_ENABLE_SIMD
    // Alignment specialization
    template<>
    class Matrix4x4Data<float>
    {
    public:
        static const unsigned Rows = 4;
        SF_SIMD_ALIGN( float M[Rows][4] );
    };
#endif

template <typename T>
class Matrix4x4 : public Matrix4x4Data<T>
{
public:
    using Matrix4x4Data<T>::M;  // GCC 3.4 compatibility.

    static Matrix4x4    Identity;

    enum NoInitType { NoInit };

    // Construct matrix with no initializer; later assignment expected.
    Matrix4x4(NoInitType) {  }
    // construct identity
    Matrix4x4()                              { SetIdentity(); }
    // construct from data
    Matrix4x4(const T *pVals, int count=16)  { Set(pVals, count);  }
    Matrix4x4(const T pVals[4][4])           { Set(&pVals[0][0], 16);  }
    inline Matrix4x4(
        T v1, T v2, T v3, T v4,
        T v5, T v6, T v7, T v8,
        T v9, T v10, T v11, T v12,
        T v13, T v14, T v15, T v16);
    // copy constructor
    Matrix4x4(const Matrix4x4 &mat)          { *this = mat;  }
    // construct from Matrix2x4
    Matrix4x4(const Matrix2x4<T> &m);
    // construct from Matrix3x4
    Matrix4x4(const Matrix3x4<T> &m);

    // constructors for multiplication
    Matrix4x4(const Matrix4x4 &m1, const Matrix4x4 &m2)     { MultiplyMatrix(m1, m2); }
    Matrix4x4(const Matrix4x4 &m1, const Matrix3x4<T> &m2)  { MultiplyMatrix(m1, m2); }
    Matrix4x4(const Matrix4x4 &m1, const Matrix2x4<T> &m2)  { MultiplyMatrix(m1, m2); }
    Matrix4x4(const Matrix3x4<T> &m1, const Matrix4x4 &m2)  { MultiplyMatrix(m1, m2); }
    Matrix4x4(const Matrix2x4<T> &m1, const Matrix4x4 &m2)  { MultiplyMatrix(m1, m2); }

    // Checks if all matrix values are within the -MAX..MAX value range
    SF_EXPORT bool IsValid() const;

    // array accessors, [i][j] is equivalent to [i*4+j]
    operator T * ()                     { return Data(); }      // conversion operator
    operator const T * () const         { return Data(); }      // conversion operator
    operator Matrix3F ()                                        // convert to Matrix3F
    {
        Matrix3F m(Matrix3F::NoInit);
        m.M[0][0] = (float)M[0][0];   // SX
        m.M[0][1] = (float)M[0][1];
        m.M[0][2] = (float)M[0][2];
        m.M[0][3] = (float)M[0][3];   // TX

        m.M[1][0] = (float)M[1][0];
        m.M[1][1] = (float)M[1][1];   // SY
        m.M[1][2] = (float)M[1][2];
        m.M[1][3] = (float)M[1][3];   // TY

        m.M[2][0] = (float)M[2][0];
        m.M[2][1] = (float)M[2][1];
        m.M[2][2] = (float)M[2][2];   // SZ
        m.M[2][3] = (float)M[2][3];   // TZ
        return m;
    }

    T *Data()                           { return (T*)M;  }
    const T *Data() const               { return (T*)M;  }
    T operator [] (int pos)             { return Data()[pos]; }
    T operator [] (int pos) const       { return Data()[pos]; }

    inline void        GetAsFloat2x4(float (*rows)[4]) const;
    inline void        GetAsFloat4x4(float (*rows)[4]) const;

    void Set(const T *pVals, int count) { SF_ASSERT(count<=16); memcpy(M, pVals, count*sizeof(T));   }
    void Set(const T pVals[4][4])       { Set(&pVals[0][0], 16); }
    void Set(const Matrix4x4 &mat)      { Set(&mat.M[0][0], 16); }

    SF_EXPORT void Transpose();
    SF_EXPORT void SetIdentity();
    void Clear()                        {  memset(M, 0, sizeof(M)); }

    // Scaling
    // use basis vectors
    T GetXScale() const                 { return sqrtf(M[0][0]*M[0][0]+M[1][0]*M[1][0]+M[2][0]*M[2][0]); }
    T GetYScale() const                 { return sqrtf(M[0][1]*M[0][1]+M[1][1]*M[1][1]+M[2][1]*M[2][1]); }
    T GetZScale() const                 { return sqrtf(M[0][2]*M[0][2]+M[1][2]*M[1][2]+M[2][2]*M[2][2]); }

    void SetXScale(T s)                 { s /= GetXScale(); M[0][0] *= s; M[1][0] *= s; M[2][0] *= s; }
    void SetYScale(T s)                 { s /= GetYScale(); M[0][1] *= s; M[1][1] *= s; M[2][1] *= s; }
    void SetZScale(T s)                 { s /= GetZScale(); M[0][2] *= s; M[1][2] *= s; M[2][2] *= s; }

    SF_EXPORT void GetScale(T *tX, T *tY, T *tZ) const;

    // static, create scale matrix
    static inline Matrix4x4<T> Scaling(T sX, T sY, T sZ);

    // Translation
    SF_EXPORT void GetTranslation(T *tX, T *tY, T *tZ) const;

    T &Tx()                             { return M[0][3]; }
    T &Ty()                             { return M[1][3]; }
    T &Tz()                             { return M[2][3]; }

    const T &Tx() const                 { return M[0][3]; }
    const T &Ty() const                 { return M[1][3]; }
    const T &Tz() const                 { return M[2][3]; }

    // static, create Translation matrix
    static inline Matrix4x4<T> Translation(T tX, T tY, T tZ);

    // Multiplication (matches Matrix2x4 ordering as well)
    SF_EXPORT void MultiplyMatrix(const Matrix4x4 &m1, const Matrix4x4 &m2);
    SF_EXPORT void MultiplyMatrix_NonOpt(const Matrix4x4 &m1, const Matrix4x4 &m2);

    // Multiplication 3x4 * 4x4
    SF_EXPORT void MultiplyMatrix(const Matrix3x4<T> &m1, const Matrix4x4 &m2);
    SF_EXPORT void MultiplyMatrix_NonOpt(const Matrix3x4<T> &m1, const Matrix4x4 &m2);

    // Multiplication 4x4 * 3x4
    SF_EXPORT void MultiplyMatrix(const Matrix4x4 &m1, const Matrix3x4<T> &m2);
    SF_EXPORT void MultiplyMatrix_NonOpt(const Matrix4x4 &m1, const Matrix3x4<T> &m2);

    // Multiplication 2x4 * 4x4
    SF_EXPORT void MultiplyMatrix(const Matrix2x4<T> &m1, const Matrix4x4 &m2);
    SF_EXPORT void MultiplyMatrix_NonOpt(const Matrix2x4<T> &m1, const Matrix4x4 &m2);

    // Multiplication 4x4 * 2x4
    SF_EXPORT void MultiplyMatrix(const Matrix4x4 &m1, const Matrix2x4<T> &m2);
    SF_EXPORT void MultiplyMatrix_NonOpt(const Matrix4x4 &m1, const Matrix2x4<T> &m2);

#if (defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG)
    SF_EXPORT void MultiplyMatrix_Opt(const Matrix4x4 &m1, const Matrix3x4<T> &m2, float* mask);
#endif

    void Prepend(const Matrix4x4 &matrixA)
    {
        Matrix4x4 matrixThis = *this;
        MultiplyMatrix(matrixThis, matrixA);
    }

    void Prepend(const Matrix3x4<T> &matrixA)
    {
        Matrix4x4 matrixThis = *this;
        MultiplyMatrix(matrixThis, matrixA);
    }

    void Prepend(const Matrix2x4<T> &matrixA)
    {
        Matrix4x4 matrixThis = *this;
        MultiplyMatrix(matrixThis, matrixA);
    }

    void Append(const Matrix4x4 &matrixA)
    {
        Matrix4x4 matrixThis = *this;
        MultiplyMatrix(matrixA, matrixThis);
    }

    void Append(const Matrix3x4<T> &matrixA)
    {
        Matrix4x4 matrixThis = *this;
        MultiplyMatrix(matrixA, matrixThis);
    }

    void Append(const Matrix2x4<T> &matrixA)
    {
        Matrix4x4 matrixThis = *this;
        MultiplyMatrix(matrixA, matrixThis);
    }

    void SetToAppend(const Matrix4x4 &matrixA, const Matrix4x4 &matrixB)
    {
        MultiplyMatrix(matrixB, matrixA);
    }

    void SetToAppend(const Matrix4x4 &matrixA, const Matrix3x4<T> &matrixB)
    {
        MultiplyMatrix(matrixB, matrixA);
    }

    void SetToAppend(const Matrix4x4<T> &matrixA, const Matrix2x4<T> &matrixB)
    {
        MultiplyMatrix(matrixB, matrixA);
    }

    void SetToAppend(const Matrix3x4<T> &matrixA, const Matrix4x4 &matrixB)
    {
        MultiplyMatrix(matrixB, matrixA);
    }

    void SetToAppend(const Matrix2x4<T> &matrixA, const Matrix4x4 &matrixB)
    {
        MultiplyMatrix(matrixB, matrixA);
    }

    void SetToPrepend(const Matrix4x4 &matrixA, const Matrix4x4 &matrixB)
    {
        MultiplyMatrix(matrixA, matrixB);
    }

    void SetToPrepend(const Matrix4x4 &matrixA, const Matrix3x4<T> &matrixB)
    {
        MultiplyMatrix(matrixA, matrixB);
    }

    void SetToPrepend(const Matrix4x4<T> &matrixA, const Matrix2x4<T> &matrixB)
    {
        MultiplyMatrix(matrixA, matrixB);
    }

    void SetToPrepend(const Matrix3x4<T> &matrixA, const Matrix4x4 &matrixB)
    {
        MultiplyMatrix(matrixA, matrixB);
    }

    void SetToPrepend(const Matrix2x4<T> &matrixA, const Matrix4x4 &matrixB)
    {
        MultiplyMatrix(matrixA, matrixB);
    }

    inline friend bool                operator == (const Matrix4x4 &m1, const Matrix4x4 &m2)
    {
        return memcmp(m1.M, m2.M, sizeof(Matrix4x4<T>)) == 0;
    }
    inline friend bool                operator != (const Matrix4x4 &m1, const Matrix4x4 &m2)
    {
        return  !(m1 == m2);
    }

    // assignment
    inline const Matrix4x4&            operator =  (const Matrix4x4 &m)
    {
        Set(m);
        return *this;
    }

    // Inverse
    inline Matrix4x4<T> GetInverse() const;
    void  SetInverse(const Matrix4x4& mIn)  { *this = mIn.GetInverse(); }
    void  Invert()                          { *this = GetInverse(); }

    // Transform
    SF_EXPORT void  Transform(Point3<T>* result, const Point3<T>& p) const { *result = Transform(p); }
    Point3<T>       Transform(const Point3<T>& p) const
    {
        return Point3<T>(
            M[0][0] * p.x + M[0][1] * p.y + M[0][2] * p.z + M[0][3],
            M[1][0] * p.x + M[1][1] * p.y + M[1][2] * p.z + M[1][3],
            M[2][0] * p.x + M[2][1] * p.y + M[2][2] * p.z + M[2][3]
        );
    }

    // Transform, and divide by the W component.
    void            TransformHomogeneous(Point3<T>* result, const Point3<T>& p) const { *result = TransformHomogeneous(p); }
    Point3<T>       TransformHomogeneous(const Point3<T>& p) const
    {
        Point3<T> result(
            M[0][0] * p.x + M[0][1] * p.y + M[0][2] * p.z + M[0][3],
            M[1][0] * p.x + M[1][1] * p.y + M[1][2] * p.z + M[1][3],
            M[2][0] * p.x + M[2][1] * p.y + M[2][2] * p.z + M[2][3]
        );
        float w = M[3][0] * p.x + M[3][1] * p.y + M[3][2] * p.z + M[3][3];
        result /= w;
        return result;
    }


    Point3<T>     TransformNormal(const Point3<T>& p) const
    {
        Matrix4x4 invMat = GetInverse();
        invMat.Transpose();
        return invMat.Transform(p);
    }

    void TransformHomogeneousAndScaleCorners(const Rect<T>& bounds, T sx, T sy, T* dest) const;
    void TransformHomogeneousAndScaleCorners_NonOpt(const Rect<T>& bounds, T sx, T sy, T* dest) const;

    SF_EXPORT void      Transform(Point<T>* result, const Point<T>& p) const { *result = Transform(p); }

    SF_EXPORT Point<T>  Transform(const Point<T>& p) const
    {
        T pZ = 1.f;
        return Point<T>(
            M[0][0] * p.x + M[0][1] * p.y + M[0][2] * pZ + M[0][3],
            M[1][0] * p.x + M[1][1] * p.y + M[1][2] * pZ + M[1][3]
        );
    }

    Point<T>     TransformByInverse(const Point<T>& p) const
    {
        Matrix4x4 matCopy = GetInverse();
        return matCopy.Transform(p);
    }

    SF_EXPORT void      EncloseTransform(Rect<T> *pr, const Rect<T>& r) const;
    SF_EXPORT Rect<T>   EncloseTransform(const Rect<T>& r) const  { Rect<T> d; return EncloseTransform(&d, r); }

    SF_EXPORT void      EncloseTransformHomogeneous(Rect<T> *pr, const Rect<T>& r) const;
    SF_EXPORT void      EncloseTransformHomogeneous_NonOpt(Rect<T> *pr, const Rect<T>& r) const;

    inline T GetDeterminant() const;

    // Rotation
    // returns euler angles in radians
    void GetRotation(T *eX, T *eY, T *eZ) const { return GetEulerAngles(eX, eY, eZ); }

    // static initializers, create Rotation matrices
    static inline Matrix4x4<T> RotationX(T angleRad);
    static inline Matrix4x4<T> RotationY(T angleRad);
    static inline Matrix4x4<T> RotationZ(T angleRad);
    static inline Matrix4x4<T> Rotation(T angleRad, const Point3<T>& axis);
    static inline Matrix4x4<T> Rotation(T angleRad, const Point3<T>& axis, const Point3<T>& pivot);

    // create camera view matrix, world to view transform. Right or Left-handed.
    SF_EXPORT void ViewRH(const Point3<T>& eyePt, const Point3<T>& lookAtPt, const Point3<T>& upVec);
    SF_EXPORT void ViewLH(const Point3<T>& eyePt, const Point3<T>& lookAtPt, const Point3<T>& upVec);
    SF_EXPORT void View  (const Point3<T>& eyePt, const Point3<T>& viewVector, const Point3<T>& upVec);

    // create perspective matrix, view to screen transform.  Right or Left-handed.
    SF_EXPORT void PerspectiveRH(T fovYRad, T fAR, T fNearZ, T fFarZ);
    SF_EXPORT void PerspectiveLH(T fovYRad, T fAR, T fNearZ, T fFarZ);
    SF_EXPORT void PerspectiveFocalLengthRH(T focalLength, T DisplayWidth, T DisplayHeight, T fNearZ, T fFarZ);
    SF_EXPORT void PerspectiveFocalLengthLH(T focalLength, T DisplayWidth, T DisplayHeight, T fNearZ, T fFarZ);
    SF_EXPORT void PerspectiveViewVolRH(T viewW, T viewH, T fNearZ, T fFarZ);
    SF_EXPORT void PerspectiveViewVolLH(T viewW, T viewH, T fNearZ, T fFarZ);
    SF_EXPORT void PerspectiveOffCenterLH(T focalLength, T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ);
    SF_EXPORT void PerspectiveOffCenterRH(T focalLength, T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ);

    // create orthographic projection matrix.  Right or Left-handed.
    SF_EXPORT void OrthoRH(T viewW, T viewH, T fNearZ, T fFarZ);
    SF_EXPORT void OrthoLH(T viewW, T viewH, T fNearZ, T fFarZ);
    SF_EXPORT void OrthoOffCenterRH(T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ);
    SF_EXPORT void OrthoOffCenterLH(T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ);

private:

    void GetEulerAngles(T *eX, T *eY, T *eZ) const;
    T GetMinor(const Matrix4x4<T>& m, const size_t r0, const size_t r1, const size_t r2,
        const size_t c0, const size_t c1, const size_t c2) const;
    Matrix4x4<T> Adjoint() const;
};


//////////////////////////////////////////////////////////////////////////
// begin INLINE implementation
//////////////////////////////////////////////////////////////////////////

// static identity
template<typename T>
Matrix4x4<T>   Matrix4x4<T>::Identity;

// ctor
template<typename T>
inline Matrix4x4<T>::Matrix4x4(const Matrix2x4<T> &m)
{
    M[0][0] = m.M[0][0];   // SX
    M[0][1] = m.M[0][1];
    M[0][2] = m.M[0][2];
    M[0][3] = m.M[0][3];   // TX

    M[1][0] = m.M[1][0];
    M[1][1] = m.M[1][1];   // SY
    M[1][2] = m.M[1][2];
    M[1][3] = m.M[1][3];   // TY

    M[2][0] = 0;
    M[2][1] = 0;
    M[2][2] = 1.0f;
    M[2][3] = 0;

    M[3][0] = 0;
    M[3][1] = 0;
    M[3][2] = 0;
    M[3][3] = 1.0f;
}

// construct from a 3x4
template<typename T>
inline Matrix4x4<T>::Matrix4x4(const Matrix3x4<T> &m)
{
    M[0][0] = m.M[0][0];
    M[0][1] = m.M[0][1];
    M[0][2] = m.M[0][2];
    M[0][3] = m.M[0][3];

    M[1][0] = m.M[1][0];
    M[1][1] = m.M[1][1];
    M[1][2] = m.M[1][2];
    M[1][3] = m.M[1][3];

    M[2][0] = m.M[2][0];
    M[2][1] = m.M[2][1];
    M[2][2] = m.M[2][2];
    M[2][3] = m.M[2][3];

    M[3][0] = 0;
    M[3][1] = 0;
    M[3][2] = 0;
    M[3][3] = 1.0f;
}

// construct from data elements
template<typename T>
inline Matrix4x4<T>::Matrix4x4(
                               T v1, T v2, T v3, T v4,
                               T v5, T v6, T v7, T v8,
                               T v9, T v10, T v11, T v12,
                               T v13, T v14, T v15, T v16)
{
    M[0][0] = v1;
    M[0][1] = v2;
    M[0][2] = v3;
    M[0][3] = v4;

    M[1][0] = v5;
    M[1][1] = v6;
    M[1][2] = v7;
    M[1][3] = v8;

    M[2][0] = v9;
    M[2][1] = v10;
    M[2][2] = v11;
    M[2][3] = v12;

    M[3][0] = v13;
    M[3][1] = v14;
    M[3][2] = v15;
    M[3][3] = v16;
}

// is the matrix valid?
template<typename T>
inline bool    Matrix4x4<T>::IsValid() const
{
    return SF_ISFINITE(M[0][0])
        && SF_ISFINITE(M[0][1])
        && SF_ISFINITE(M[0][2])
        && SF_ISFINITE(M[0][3])

        && SF_ISFINITE(M[1][0])
        && SF_ISFINITE(M[1][1])
        && SF_ISFINITE(M[1][2])
        && SF_ISFINITE(M[1][3])

        && SF_ISFINITE(M[2][0])
        && SF_ISFINITE(M[2][1])
        && SF_ISFINITE(M[2][2])
        && SF_ISFINITE(M[2][3])

        && SF_ISFINITE(M[3][0])
        && SF_ISFINITE(M[3][1])
        && SF_ISFINITE(M[3][2])
        && SF_ISFINITE(M[3][3]);
}

template<typename T>
inline void Matrix4x4<T>::GetAsFloat2x4(float (*rows)[4]) const
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
inline void Matrix4x4<T>::GetAsFloat4x4(float (*rows)[4]) const
{
    rows[0][0] = (float)M[0][0];
    rows[0][1] = (float)M[0][1];
    rows[0][2] = (float)M[0][2];
    rows[0][3] = (float)M[0][3];

    rows[1][0] = (float)M[1][0];
    rows[1][1] = (float)M[1][1];
    rows[1][2] = (float)M[1][2];
    rows[1][3] = (float)M[1][3];

    rows[2][0] = (float)M[2][0];
    rows[2][1] = (float)M[2][1];
    rows[2][2] = (float)M[2][2];
    rows[2][3] = (float)M[2][3];

    rows[3][0] = (float)M[3][0];
    rows[3][1] = (float)M[3][1];
    rows[3][2] = (float)M[3][2];
    rows[3][3] = (float)M[3][3];
}


// transpose the matrix
template<typename T>
inline void Matrix4x4<T>::Transpose()
{
    Matrix4x4<T> matDest(NoInit);
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            matDest.M[j][i] = M[i][j];
    *this = matDest;
}

// set to identity
template<typename T>
inline void Matrix4x4<T>::SetIdentity()
{
    Clear();

    M[0][0] = 1;
    M[1][1] = 1;
    M[2][2] = 1;
    M[3][3] = 1;
}

// call the non-optimized version in the general case.
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix(const Matrix4x4<T> &m1, const Matrix4x4<T> &m2)
{
    MultiplyMatrix_NonOpt(m1, m2);
}

// call the non-optimized version in the general case.
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix(const Matrix3x4<T> &m1, const Matrix4x4<T> &m2)
{
    MultiplyMatrix_NonOpt(m1, m2);
}

// call the non-optimized version in the general case.
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix(const Matrix4x4<T> &m1, const Matrix3x4<T> &m2)
{
    MultiplyMatrix_NonOpt(m1, m2);
}

// call the non-optimized version in the general case.
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix(const Matrix2x4<T> &m1, const Matrix4x4<T> &m2)
{
    MultiplyMatrix_NonOpt(m1, m2);
}

// call the non-optimized version in the general case.
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix(const Matrix4x4<T> &m1, const Matrix2x4<T> &m2)
{
    MultiplyMatrix_NonOpt(m1, m2);
}

// multiply 2 matrices
template<typename T>
inline const Matrix4x4<T>      operator * (const Matrix4x4<T> &m1, const Matrix4x4<T> &m2)
{
    Matrix4x4<T> outMat(Matrix4x4<T>::NoInit);
    outMat.MultiplyMatrix(m1, m2);          // m1.Prepend(m2) or m2.append(m1), apply m2 then m1
    return outMat;
}

template<typename T>
inline const Matrix4x4<T>      operator * (const Matrix3x4<T> &m1, const Matrix4x4<T> &m2)
{
    Matrix4x4<T> outMat(Matrix4x4<T>::NoInit);
    outMat.MultiplyMatrix(m1, m2);          // m1.Prepend(m2) or m2.append(m1), apply m2 then m1
    return outMat;
}

template<typename T>
inline const Matrix4x4<T>      operator * (const Matrix4x4<T> &m1, const Matrix3x4<T> &m2)
{
    Matrix4x4<T> outMat(Matrix4x4<T>::NoInit);
    outMat.MultiplyMatrix(m1, m2);          // m1.Prepend(m2) or m2.append(m1), apply m2 then m1
    return outMat;
}

template<typename T>
inline const Matrix4x4<T>      operator * (const Matrix2x4<T> &m1, const Matrix4x4<T> &m2)
{
    Matrix4x4<T> outMat(Matrix4x4<T>::NoInit);
    outMat.MultiplyMatrix(m1, m2);          // m1.Prepend(m2) or m2.append(m1), apply m2 then m1
    return outMat;
}

template<typename T>
inline const Matrix4x4<T>      operator * (const Matrix4x4<T> &m1, const Matrix2x4<T> &m2)
{
    Matrix4x4<T> outMat(Matrix4x4<T>::NoInit);
    outMat.MultiplyMatrix(m1, m2);          // m1.Prepend(m2) or m2.append(m1), apply m2 then m1
    return outMat;
}

template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix_NonOpt(const Matrix4x4<T> &m1, const Matrix4x4<T> &m2)
{
    M[0][0] = m1.M[0][0] * m2.M[0][0] + m1.M[0][1] * m2.M[1][0] + m1.M[0][2] * m2.M[2][0] + m1.M[0][3] * m2.M[3][0];
    M[0][1] = m1.M[0][0] * m2.M[0][1] + m1.M[0][1] * m2.M[1][1] + m1.M[0][2] * m2.M[2][1] + m1.M[0][3] * m2.M[3][1];
    M[0][2] = m1.M[0][0] * m2.M[0][2] + m1.M[0][1] * m2.M[1][2] + m1.M[0][2] * m2.M[2][2] + m1.M[0][3] * m2.M[3][2];
    M[0][3] = m1.M[0][0] * m2.M[0][3] + m1.M[0][1] * m2.M[1][3] + m1.M[0][2] * m2.M[2][3] + m1.M[0][3] * m2.M[3][3];

    M[1][0] = m1.M[1][0] * m2.M[0][0] + m1.M[1][1] * m2.M[1][0] + m1.M[1][2] * m2.M[2][0] + m1.M[1][3] * m2.M[3][0];
    M[1][1] = m1.M[1][0] * m2.M[0][1] + m1.M[1][1] * m2.M[1][1] + m1.M[1][2] * m2.M[2][1] + m1.M[1][3] * m2.M[3][1];
    M[1][2] = m1.M[1][0] * m2.M[0][2] + m1.M[1][1] * m2.M[1][2] + m1.M[1][2] * m2.M[2][2] + m1.M[1][3] * m2.M[3][2];
    M[1][3] = m1.M[1][0] * m2.M[0][3] + m1.M[1][1] * m2.M[1][3] + m1.M[1][2] * m2.M[2][3] + m1.M[1][3] * m2.M[3][3];

    M[2][0] = m1.M[2][0] * m2.M[0][0] + m1.M[2][1] * m2.M[1][0] + m1.M[2][2] * m2.M[2][0] + m1.M[2][3] * m2.M[3][0];
    M[2][1] = m1.M[2][0] * m2.M[0][1] + m1.M[2][1] * m2.M[1][1] + m1.M[2][2] * m2.M[2][1] + m1.M[2][3] * m2.M[3][1];
    M[2][2] = m1.M[2][0] * m2.M[0][2] + m1.M[2][1] * m2.M[1][2] + m1.M[2][2] * m2.M[2][2] + m1.M[2][3] * m2.M[3][2];
    M[2][3] = m1.M[2][0] * m2.M[0][3] + m1.M[2][1] * m2.M[1][3] + m1.M[2][2] * m2.M[2][3] + m1.M[2][3] * m2.M[3][3];

    M[3][0] = m1.M[3][0] * m2.M[0][0] + m1.M[3][1] * m2.M[1][0] + m1.M[3][2] * m2.M[2][0] + m1.M[3][3] * m2.M[3][0];
    M[3][1] = m1.M[3][0] * m2.M[0][1] + m1.M[3][1] * m2.M[1][1] + m1.M[3][2] * m2.M[2][1] + m1.M[3][3] * m2.M[3][1];
    M[3][2] = m1.M[3][0] * m2.M[0][2] + m1.M[3][1] * m2.M[1][2] + m1.M[3][2] * m2.M[2][2] + m1.M[3][3] * m2.M[3][2];
    M[3][3] = m1.M[3][0] * m2.M[0][3] + m1.M[3][1] * m2.M[1][3] + m1.M[3][2] * m2.M[2][3] + m1.M[3][3] * m2.M[3][3];
}

//specialize for 3x4 * 4x4
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix_NonOpt(const Matrix3x4<T> &m1, const Matrix4x4<T> &m2)
{
    M[0][0] = m1.M[0][0] * m2.M[0][0] + m1.M[0][1] * m2.M[1][0] + m1.M[0][2] * m2.M[2][0] + m1.M[0][3] * m2.M[3][0];
    M[0][1] = m1.M[0][0] * m2.M[0][1] + m1.M[0][1] * m2.M[1][1] + m1.M[0][2] * m2.M[2][1] + m1.M[0][3] * m2.M[3][1];
    M[0][2] = m1.M[0][0] * m2.M[0][2] + m1.M[0][1] * m2.M[1][2] + m1.M[0][2] * m2.M[2][2] + m1.M[0][3] * m2.M[3][2];
    M[0][3] = m1.M[0][0] * m2.M[0][3] + m1.M[0][1] * m2.M[1][3] + m1.M[0][2] * m2.M[2][3] + m1.M[0][3] * m2.M[3][3];

    M[1][0] = m1.M[1][0] * m2.M[0][0] + m1.M[1][1] * m2.M[1][0] + m1.M[1][2] * m2.M[2][0] + m1.M[1][3] * m2.M[3][0];
    M[1][1] = m1.M[1][0] * m2.M[0][1] + m1.M[1][1] * m2.M[1][1] + m1.M[1][2] * m2.M[2][1] + m1.M[1][3] * m2.M[3][1];
    M[1][2] = m1.M[1][0] * m2.M[0][2] + m1.M[1][1] * m2.M[1][2] + m1.M[1][2] * m2.M[2][2] + m1.M[1][3] * m2.M[3][2];
    M[1][3] = m1.M[1][0] * m2.M[0][3] + m1.M[1][1] * m2.M[1][3] + m1.M[1][2] * m2.M[2][3] + m1.M[1][3] * m2.M[3][3];

    M[2][0] = m1.M[2][0] * m2.M[0][0] + m1.M[2][1] * m2.M[1][0] + m1.M[2][2] * m2.M[2][0] + m1.M[2][3] * m2.M[3][0];
    M[2][1] = m1.M[2][0] * m2.M[0][1] + m1.M[2][1] * m2.M[1][1] + m1.M[2][2] * m2.M[2][1] + m1.M[2][3] * m2.M[3][1];
    M[2][2] = m1.M[2][0] * m2.M[0][2] + m1.M[2][1] * m2.M[1][2] + m1.M[2][2] * m2.M[2][2] + m1.M[2][3] * m2.M[3][2];
    M[2][3] = m1.M[2][0] * m2.M[0][3] + m1.M[2][1] * m2.M[1][3] + m1.M[2][2] * m2.M[2][3] + m1.M[2][3] * m2.M[3][3];

    M[3][0] = m2.M[3][0];
    M[3][1] = m2.M[3][1];
    M[3][2] = m2.M[3][2];
    M[3][3] = m2.M[3][3];
}

// specialize for 4x4 * 3x4
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix_NonOpt(const Matrix4x4<T> &m1, const Matrix3x4<T> &m2)
{
    M[0][0] = m1.M[0][0] * m2.M[0][0] + m1.M[0][1] * m2.M[1][0] + m1.M[0][2] * m2.M[2][0];
    M[0][1] = m1.M[0][0] * m2.M[0][1] + m1.M[0][1] * m2.M[1][1] + m1.M[0][2] * m2.M[2][1];
    M[0][2] = m1.M[0][0] * m2.M[0][2] + m1.M[0][1] * m2.M[1][2] + m1.M[0][2] * m2.M[2][2];
    M[0][3] = m1.M[0][0] * m2.M[0][3] + m1.M[0][1] * m2.M[1][3] + m1.M[0][2] * m2.M[2][3] + m1.M[0][3];

    M[1][0] = m1.M[1][0] * m2.M[0][0] + m1.M[1][1] * m2.M[1][0] + m1.M[1][2] * m2.M[2][0];
    M[1][1] = m1.M[1][0] * m2.M[0][1] + m1.M[1][1] * m2.M[1][1] + m1.M[1][2] * m2.M[2][1];
    M[1][2] = m1.M[1][0] * m2.M[0][2] + m1.M[1][1] * m2.M[1][2] + m1.M[1][2] * m2.M[2][2];
    M[1][3] = m1.M[1][0] * m2.M[0][3] + m1.M[1][1] * m2.M[1][3] + m1.M[1][2] * m2.M[2][3] + m1.M[1][3];

    M[2][0] = m1.M[2][0] * m2.M[0][0] + m1.M[2][1] * m2.M[1][0] + m1.M[2][2] * m2.M[2][0];
    M[2][1] = m1.M[2][0] * m2.M[0][1] + m1.M[2][1] * m2.M[1][1] + m1.M[2][2] * m2.M[2][1];
    M[2][2] = m1.M[2][0] * m2.M[0][2] + m1.M[2][1] * m2.M[1][2] + m1.M[2][2] * m2.M[2][2];
    M[2][3] = m1.M[2][0] * m2.M[0][3] + m1.M[2][1] * m2.M[1][3] + m1.M[2][2] * m2.M[2][3] + m1.M[2][3];

    M[3][0] = m1.M[3][0] * m2.M[0][0] + m1.M[3][1] * m2.M[1][0] + m1.M[3][2] * m2.M[2][0];
    M[3][1] = m1.M[3][0] * m2.M[0][1] + m1.M[3][1] * m2.M[1][1] + m1.M[3][2] * m2.M[2][1];
    M[3][2] = m1.M[3][0] * m2.M[0][2] + m1.M[3][1] * m2.M[1][2] + m1.M[3][2] * m2.M[2][2];
    M[3][3] = m1.M[3][0] * m2.M[0][3] + m1.M[3][1] * m2.M[1][3] + m1.M[3][2] * m2.M[2][3] + m1.M[3][3];
}

//specialize for 2x4 * 4x4
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix_NonOpt(const Matrix2x4<T> &m1, const Matrix4x4<T> &m2)
{
    M[0][0] = m1.M[0][0] * m2.M[0][0] + m1.M[0][1] * m2.M[1][0] + m1.M[0][2] * m2.M[2][0] + m1.M[0][3] * m2.M[3][0];
    M[0][1] = m1.M[0][0] * m2.M[0][1] + m1.M[0][1] * m2.M[1][1] + m1.M[0][2] * m2.M[2][1] + m1.M[0][3] * m2.M[3][1];
    M[0][2] = m1.M[0][0] * m2.M[0][2] + m1.M[0][1] * m2.M[1][2] + m1.M[0][2] * m2.M[2][2] + m1.M[0][3] * m2.M[3][2];
    M[0][3] = m1.M[0][0] * m2.M[0][3] + m1.M[0][1] * m2.M[1][3] + m1.M[0][2] * m2.M[2][3] + m1.M[0][3] * m2.M[3][3];

    M[1][0] = m1.M[1][0] * m2.M[0][0] + m1.M[1][1] * m2.M[1][0] + m1.M[1][2] * m2.M[2][0] + m1.M[1][3] * m2.M[3][0];
    M[1][1] = m1.M[1][0] * m2.M[0][1] + m1.M[1][1] * m2.M[1][1] + m1.M[1][2] * m2.M[2][1] + m1.M[1][3] * m2.M[3][1];
    M[1][2] = m1.M[1][0] * m2.M[0][2] + m1.M[1][1] * m2.M[1][2] + m1.M[1][2] * m2.M[2][2] + m1.M[1][3] * m2.M[3][2];
    M[1][3] = m1.M[1][0] * m2.M[0][3] + m1.M[1][1] * m2.M[1][3] + m1.M[1][2] * m2.M[2][3] + m1.M[1][3] * m2.M[3][3];

    M[2][0] = m2.M[2][0];
    M[2][1] = m2.M[2][1];
    M[2][2] = m2.M[2][2];
    M[2][3] = m2.M[2][3];

    M[3][0] = m2.M[3][0];
    M[3][1] = m2.M[3][1];
    M[3][2] = m2.M[3][2];
    M[3][3] = m2.M[3][3];
}

// specialize for 4x4 * 2x4
template<typename T>
inline void Matrix4x4<T>::MultiplyMatrix_NonOpt(const Matrix4x4<T> &m1, const Matrix2x4<T> &m2)
{
    M[0][0] = m1.M[0][0] * m2.M[0][0] + m1.M[0][1] * m2.M[1][0];
    M[0][1] = m1.M[0][0] * m2.M[0][1] + m1.M[0][1] * m2.M[1][1];
    M[0][2] = m1.M[0][0] * m2.M[0][2] + m1.M[0][1] * m2.M[1][2] + m1.M[0][2];
    M[0][3] = m1.M[0][0] * m2.M[0][3] + m1.M[0][1] * m2.M[1][3] + m1.M[0][3];

    M[1][0] = m1.M[1][0] * m2.M[0][0] + m1.M[1][1] * m2.M[1][0];
    M[1][1] = m1.M[1][0] * m2.M[0][1] + m1.M[1][1] * m2.M[1][1];
    M[1][2] = m1.M[1][0] * m2.M[0][2] + m1.M[1][1] * m2.M[1][2] + m1.M[1][2];
    M[1][3] = m1.M[1][0] * m2.M[0][3] + m1.M[1][1] * m2.M[1][3] + m1.M[1][3];

    M[2][0] = m1.M[2][0] * m2.M[0][0] + m1.M[2][1] * m2.M[1][0];
    M[2][1] = m1.M[2][0] * m2.M[0][1] + m1.M[2][1] * m2.M[1][1];
    M[2][2] = m1.M[2][0] * m2.M[0][2] + m1.M[2][1] * m2.M[1][2] + m1.M[2][2];
    M[2][3] = m1.M[2][0] * m2.M[0][3] + m1.M[2][1] * m2.M[1][3] + m1.M[2][3];

    M[3][0] = m1.M[3][0] * m2.M[0][0] + m1.M[3][1] * m2.M[1][0];
    M[3][1] = m1.M[3][0] * m2.M[0][1] + m1.M[3][1] * m2.M[1][1];
    M[3][2] = m1.M[3][0] * m2.M[0][2] + m1.M[3][1] * m2.M[1][2] + m1.M[3][2];
    M[3][3] = m1.M[3][0] * m2.M[0][3] + m1.M[3][1] * m2.M[1][3] + m1.M[3][3];
}

// Get euler rot angles
template<typename T>
inline void Matrix4x4<T>::GetEulerAngles(T *eX, T *eY, T *eZ) const
{
    // Unscale the matrix before extracting term
    Matrix4x4<T> copy(*this);

    // knock out the scale
    copy.SetXScale(1.0f);
    copy.SetYScale(1.0f);
    copy.SetZScale(1.0f);

    // Assuming the angles are in radians.
    if (copy.M[1][0] > 0.998f) {                        // singularity at north pole
        if (eY)
            *eY = atan2f(copy.M[0][2],copy.M[2][2]);
        if (eZ)
            *eZ = (T)SF_MATH_PI/2.f;
        if (eX)
            *eX = 0;
        return;
    }
    if (copy.M[1][0] < -0.998f) {                       // singularity at south pole
        if (eY)
            *eY = atan2f(copy.M[0][2],copy.M[2][2]);
        if (eZ)
            *eZ = (T)-SF_MATH_PI/2.f;
        if (eX)
            *eX = 0;
        return;
    }

    if (eY)
        *eY = atan2f(-copy.M[2][0],copy.M[0][0]);
    if (eX)
        *eX = atan2f(-copy.M[1][2],copy.M[1][1]);
    if (eZ)
        *eZ = asinf(copy.M[1][0]);
}

// Return X rotation matrix
template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::RotationX(T angle)
{
    Matrix3x4<T> mat;

    T c = cosf((float)angle);
    T s = sinf((float)angle);
    mat.M[0][0] = 1.0f;
    mat.M[1][0] = 0.0f;
    mat.M[2][0] = 0.0f;
    mat.M[0][1] = 0.0f;
    mat.M[1][1] = c;
    mat.M[2][1] = s;
    mat.M[0][2] = 0.0f;
    mat.M[1][2] = -s;
    mat.M[2][2] = c;

    return mat;
}

// Return Y rotation matrix
template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::RotationY(T angle)
{
    Matrix3x4<T> mat;

    T c = cosf((float)angle);
    T s = sinf((float)angle);
    mat.M[0][0] = c;
    mat.M[1][0] = 0.0f;
    mat.M[2][0] = -s;
    mat.M[0][1] = 0.0f;
    mat.M[1][1] = 1;
    mat.M[2][1] = 0.0f;
    mat.M[0][2] = s;
    mat.M[1][2] = 0.0f;
    mat.M[2][2] = c;

    return mat;
}

// Return Z rotation matrix
template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::RotationZ(T angle)
{
    Matrix4x4<T> mat;

    T c = cosf((float)angle);
    T s = sinf((float)angle);
    mat.M[0][0] = c;
    mat.M[1][0] = s;
    mat.M[2][0] = 0.0f;
    mat.M[0][1] = -s;
    mat.M[1][1] = c;
    mat.M[2][1] = 0.0f;
    mat.M[0][2] = 0.0f;
    mat.M[1][2] = 0.0f;
    mat.M[2][2] = 1.0f;

    return mat;
}

// Return rotation matrix around arbitrary axis
template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::Rotation(T angle, const Point3<T>& axis)
{
    Matrix4x4<T> mat;

    T c = cosf((float)angle);
    T s = sinf((float)angle);
    T oneMinusC = 1.0f - c;
    T xx = axis.x * axis.x;
    T yy = axis.y * axis.y;
    T zz = axis.z * axis.z;
    T xymc = axis.x * axis.y * oneMinusC;
    T xzmc = axis.x * axis.z * oneMinusC;
    T yzmc = axis.y * axis.z * oneMinusC;
    T xs = axis.x * s;
    T ys = axis.y * s;
    T zs = axis.z * s;

    mat.M[0][0] = xx * oneMinusC + c;
    mat.M[0][1] = xymc - zs;
    mat.M[0][2] = xzmc + ys;
    mat.M[1][0] = xymc + zs;
    mat.M[1][1] = yy * oneMinusC + c;
    mat.M[1][2] = yzmc - xs;
    mat.M[2][0] = xzmc - ys;
    mat.M[2][1] = yzmc + xs;
    mat.M[2][2] = zz * oneMinusC + c;

    return mat;
}

// Return rotation matrix around arbitrary axis using pivot point
template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::Rotation(T angle, const Point3<T>& axis, const Point3<T>& pivot)
{
    // TODO - compute pre-multiplied version
    return
        Matrix4x4<T>(
        Matrix4x4<T>::Translation(-pivot.x, -pivot.y, -pivot.z),
        Matrix4x4<T>(
        Matrix4x4<T>::Rotation(angle, axis),
        Matrix4x4<T>::Translation(pivot.x, pivot.y, pivot.z)
        )
        );
}

template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::Translation(T tX, T tY, T tZ)
{
    Matrix4x4<T> mat;

    mat.M[0][3] = tX;
    mat.M[1][3] = tY;
    mat.M[2][3] = tZ;

    return mat;
}

// Get translation values
template<typename T>
inline void Matrix4x4<T>::GetTranslation(T *tX, T *tY, T *tZ) const
{
    if (tX)
        *tX = Tx();
    if (tY)
        *tY = Ty();
    if (tZ)
        *tZ = Tz();
}

// create scale matrix
template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::Scaling(T sX, T sY, T sZ)
{
    Matrix4x4<T> mat;
    mat.M[0][0] = sX;
    mat.M[1][1] = sY;
    mat.M[2][2] = sZ;
    return mat;
}

// Get scale values
template<typename T>
inline void Matrix4x4<T>::GetScale(T *tX, T *tY, T *tZ) const
{
    if (tX)
        *tX = GetXScale();
    if (tY)
        *tY = GetYScale();
    if (tZ)
        *tZ = GetZScale();
}

// create camera view matrix
template<typename T>
inline void Matrix4x4<T>::ViewRH(const Point3<T>& eyePt, const Point3<T>& lookAtPt, const Point3<T>& upVec)
{
    // view direction
    Point3<T> zAxis = (eyePt - lookAtPt);
    zAxis.Normalize();
    View(eyePt, zAxis, upVec);
}

// create camera view matrix
template<typename T>
inline void Matrix4x4<T>::ViewLH(const Point3<T>& eyePt, const Point3<T>& lookAtPt, const Point3<T>& upVec)
{
    // view direction
    Point3<T> zAxis = (lookAtPt - eyePt);
    zAxis.Normalize();
    View(eyePt, zAxis, upVec);
}

template<typename T>
inline void Matrix4x4<T>::View(const Point3<T>& eyePt, const Point3<T>& zAxis, const Point3<T>& upVec)
{
    Matrix3x4<T>* view = new(M) Matrix3x4<T>(NoInit);
    view->View(eyePt, zAxis, upVec);
    M[3][0] = 0;
    M[3][1] = 0;
    M[3][2] = 0;
    M[3][3] = 1;
}

// create right handed perspective matrix
template<typename T>
inline void Matrix4x4<T>::PerspectiveRH(T fovYRad, T fAR, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    T dz = fNearZ - fFarZ;
    T yScale = cosf((float)fovYRad*0.5f) / sinf((float)fovYRad*0.5f);
    T xScale = yScale / fAR;

    M[0][0] = xScale;
    M[1][1] = yScale;
    M[2][2] = fFarZ / dz;
    M[3][2] = -1;
    M[2][3] = fNearZ * fFarZ / dz;
}

// create left handed perspective matrix
template<typename T>
inline void Matrix4x4<T>::PerspectiveLH(T fovYRad, T fAR, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    T dz = fFarZ - fNearZ;
    T yScale = cosf((float)fovYRad*0.5f) / sinf((float)fovYRad*0.5f);
    T xScale = yScale / fAR;

    M[0][0] = xScale;
    M[1][1] = yScale;
    M[2][2] = fFarZ / dz;
    M[3][2] = 1;
    M[2][3] = -fNearZ * fFarZ / dz;
}

// create right handed perspective matrix based on focal length
template<typename T>
inline void Matrix4x4<T>::PerspectiveFocalLengthRH(T focalLength, T DisplayWidth, T DisplayHeight, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    float dz = fNearZ - fFarZ;
    float xScale = 2.f*focalLength/DisplayWidth;
    float yScale = 2.f*focalLength/DisplayHeight;

    M[0][0] = xScale;
    M[1][1] = yScale;
    M[2][2] = fFarZ / dz;
    M[3][2] = -1;
    M[2][3] = fNearZ * fFarZ / dz;
}

// create left handed perspective matrix based on focal length
template<typename T>
inline void Matrix4x4<T>::PerspectiveFocalLengthLH(T focalLength, T DisplayWidth, T DisplayHeight, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    float dz = fFarZ - fNearZ;
    float xScale = 2.f*focalLength/DisplayWidth;
    float yScale = 2.f*focalLength/DisplayHeight;

    M[0][0] = xScale;
    M[1][1] = yScale;
    M[2][2] = fFarZ / dz;
    M[3][2] = 1;
    M[2][3] = -fNearZ * fFarZ / dz;
}

// create right handed perspective matrix based on view vol
template<typename T>
inline void Matrix4x4<T>::PerspectiveViewVolRH(T viewW, T viewH, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    T dz = fNearZ - fFarZ;
    M[0][0] = 2.f * fNearZ / viewW;
    M[1][1] = 2.f * fNearZ / viewH;
    M[2][2] = fFarZ / dz;
    M[3][2] = -1;
    M[2][3] = fNearZ * fFarZ / dz;
}

// create left handed perspective matrix based on view vol
template<typename T>
inline void Matrix4x4<T>::PerspectiveViewVolLH(T viewW, T viewH, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    T dz = fFarZ - fNearZ;
    M[0][0] = 2.f * fNearZ / viewW;
    M[1][1] = 2.f * fNearZ / viewH;
    M[2][2] = fFarZ / dz;
    M[3][2] = 1;
    M[2][3] = fNearZ * fFarZ / (fNearZ-fFarZ);
}

// create customized right handed perspective matrix based on view vol
template<typename T>
inline void Matrix4x4<T>::PerspectiveOffCenterRH(T focalLength, T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    T dz = fNearZ - fFarZ;
    M[0][0] = 2.f * focalLength / (viewMaxX-viewMinX);
    M[1][1] = 2.f * focalLength / (viewMaxY-viewMinY);
    M[2][2] = fFarZ / dz;
    M[3][2] = -1;
    M[2][3] = fNearZ * fFarZ / dz;
    M[0][2] = (viewMinX+viewMaxX) / (viewMaxX-viewMinX);
    M[1][2] = (viewMinY+viewMaxY) / (viewMaxY-viewMinY);
}

// create customized left handed perspective matrix based on view vol
template<typename T>
inline void Matrix4x4<T>::PerspectiveOffCenterLH(T focalLength, T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    M[0][0] = 2.f * focalLength / (viewMaxX-viewMinX);
    M[1][1] = 2.f * focalLength / (viewMaxY-viewMinY);
    M[2][2] = fFarZ / (fFarZ - fNearZ);
    M[3][2] = 1;
    M[2][3] = fNearZ * fFarZ / (fNearZ - fFarZ);
    M[0][2] = (viewMinX+viewMaxX) / (viewMinX-viewMaxX);
    M[1][2] = (viewMinY+viewMaxY) / (viewMinY-viewMaxY);
}

// create orthographic projection matrix.  Right handed.
template<typename T>
inline void Matrix4x4<T>::OrthoRH(T viewW, T viewH, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    T dz = fNearZ - fFarZ;
    M[0][0] = 2.f / viewW;
    M[1][1] = 2.f / viewH;
    M[2][2] = 1.f / dz;
    M[2][3] = fNearZ / dz;
    M[3][3] = 1;
}

// create orthographic projection matrix.  Left handed.
template<typename T>
inline void Matrix4x4<T>::OrthoLH(T viewW, T viewH, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    T dz = fFarZ - fNearZ;
    M[0][0] = 2.f / viewW;
    M[1][1] = 2.f / viewH;
    M[2][2] = 1.f / dz;
    M[2][3] = -fNearZ / dz;
    M[3][3] = 1;
}

// create customized orthographic projection matrix.  Right handed.
template<typename T>
inline void Matrix4x4<T>::OrthoOffCenterRH(T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    T dz = fNearZ - fFarZ;
    M[0][0] = 2.f / (viewMaxX-viewMinX);
    M[1][1] = 2.f / (viewMaxY-viewMinY);
    M[2][2] = 1.f / dz;
    M[2][3] = fNearZ / dz;
    M[3][3] = 1;
    M[0][3] = (viewMinX+viewMaxX) / (viewMinX-viewMaxX);
    M[1][3] = (viewMinY+viewMaxY) / (viewMinY-viewMaxY);
}

// create customized orthographic projection matrix.  Left handed.
template<typename T>
inline void Matrix4x4<T>::OrthoOffCenterLH(T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ)
{
    Clear();            // zero out

    M[0][0] = 2.f / (viewMaxX-viewMinX);
    M[1][1] = 2.f / (viewMaxY-viewMinY);
    M[2][2] = 1.f / (fFarZ - fNearZ);
    M[2][3] = fNearZ / (fNearZ - fFarZ);
    M[3][3] = 1;
    M[0][3] = (viewMinX+viewMaxX) / (viewMinX-viewMaxX);
    M[1][3] = (viewMinY+viewMaxY) / (viewMinY-viewMaxY);
}

// This is an axial bound of an oriented (and/or sheared, scaled, etc) box.
template<typename T>
inline void    Matrix4x4<T>::EncloseTransform(Rect<T> *pr, const Rect<T>& r) const
{
    // Get the transformed bounding box.
    Point<T> p0, p1, p2, p3;
    Transform(&p0, r.TopLeft());
    Transform(&p1, r.TopRight());
    Transform(&p2, r.BottomRight());
    Transform(&p3, r.BottomLeft());

    pr->SetRect(p0, p0);
    pr->ExpandToPoint(p1);
    pr->ExpandToPoint(p2);
    pr->ExpandToPoint(p3);
}

// call the non-optimized version in the general case.
template<typename T>
inline void Matrix4x4<T>::EncloseTransformHomogeneous(Rect<T> *pr, const Rect<T>& r) const
{
    EncloseTransformHomogeneous_NonOpt(pr, r);
}

// This is an axial bound of an oriented (and/or sheared, scaled, etc) box, in homogeneous coordinates.
// Because we are only getting a 2D projection of the box, this function returns true if any of the bounds
// falls within the near/far clip space (and false if they are all outside). Takes into account clipspace
// straddling.
template<typename T>
inline void      Matrix4x4<T>::EncloseTransformHomogeneous_NonOpt(Rect<T> *pr, const Rect<T>& r) const
{
    Point3<T> p0, p1, p2, p3;
    TransformHomogeneous(&p0, Point3<T>(r.TopLeft().x, r.TopLeft().y, 0));
    TransformHomogeneous(&p1, Point3<T>(r.TopRight().x, r.TopRight().y, 0));
    TransformHomogeneous(&p2, Point3<T>(r.BottomRight().x, r.BottomRight().y, 0));
    TransformHomogeneous(&p3, Point3<T>(r.BottomLeft().x, r.BottomLeft().y, 0));

    pr->SetRect(p0.x, p0.y, p0.x, p0.y);
    pr->ExpandToPoint(p1.x, p1.y);
    pr->ExpandToPoint(p2.x, p2.y);
    pr->ExpandToPoint(p3.x, p3.y);
}

// The minor of a matrix is the determinant of the smaller square matrix
template<typename T>
T Matrix4x4<T>::GetMinor(const Matrix4x4<T>& m, const size_t r0, const size_t r1, const size_t r2,
           const size_t c0, const size_t c1, const size_t c2) const
{
    return m.M[r0][c0] * (m.M[r1][c1] * m.M[r2][c2] - m.M[r2][c1] * m.M[r1][c2]) -
        m.M[r0][c1] * (m.M[r1][c0] * m.M[r2][c2] - m.M[r2][c0] * m.M[r1][c2]) +
        m.M[r0][c2] * (m.M[r1][c0] * m.M[r2][c1] - m.M[r2][c0] * m.M[r1][c1]);
}

// Returns the adjoint matrix
template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::Adjoint() const
{
    return Matrix4x4<T>(
        GetMinor(*this, 1, 2, 3, 1, 2, 3),
        -GetMinor(*this, 0, 2, 3, 1, 2, 3),
        GetMinor(*this, 0, 1, 3, 1, 2, 3),
        -GetMinor(*this, 0, 1, 2, 1, 2, 3),

        -GetMinor(*this, 1, 2, 3, 0, 2, 3),
        GetMinor(*this, 0, 2, 3, 0, 2, 3),
        -GetMinor(*this, 0, 1, 3, 0, 2, 3),
        GetMinor(*this, 0, 1, 2, 0, 2, 3),

        GetMinor(*this, 1, 2, 3, 0, 1, 3),
        -GetMinor(*this, 0, 2, 3, 0, 1, 3),
        GetMinor(*this, 0, 1, 3, 0, 1, 3),
        -GetMinor(*this, 0, 1, 2, 0, 1, 3),

        -GetMinor(*this, 1, 2, 3, 0, 1, 2),
        GetMinor(*this, 0, 2, 3, 0, 1, 2),
        -GetMinor(*this, 0, 1, 3, 0, 1, 2),
        GetMinor(*this, 0, 1, 2, 0, 1, 2)
        );
}

// Returns the determinant
template<typename T>
inline T Matrix4x4<T>::GetDeterminant() const
{
    return
        M[0][0] * GetMinor(*this, 1, 2, 3, 1, 2, 3) -
        M[0][1] * GetMinor(*this, 1, 2, 3, 0, 2, 3) +
        M[0][2] * GetMinor(*this, 1, 2, 3, 0, 1, 3) -
        M[0][3] * GetMinor(*this, 1, 2, 3, 0, 1, 2);
}

// Returns the inverse matrix
template<typename T>
inline Matrix4x4<T> Matrix4x4<T>::GetInverse() const
{
    T m00 = M[0][0], m01 = M[0][1], m02 = M[0][2], m03 = M[0][3];
    T m10 = M[1][0], m11 = M[1][1], m12 = M[1][2], m13 = M[1][3];
    T m20 = M[2][0], m21 = M[2][1], m22 = M[2][2], m23 = M[2][3];
    T m30 = M[3][0], m31 = M[3][1], m32 = M[3][2], m33 = M[3][3];

    T v0 = m20 * m31 - m21 * m30;
    T v1 = m20 * m32 - m22 * m30;
    T v2 = m20 * m33 - m23 * m30;
    T v3 = m21 * m32 - m22 * m31;
    T v4 = m21 * m33 - m23 * m31;
    T v5 = m22 * m33 - m23 * m32;

    T t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
    T t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
    T t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
    T t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

    T det = (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);
    if (det == 0)
    {
        // Not invertible - this happens sometimes (ie. sample6.Swf)
        // Arbitrary fallback, set to identity and negate translation
        Matrix4x4<T> tmp;   // identity
        tmp.Tx() = -Tx();
        tmp.Ty() = -Ty();
        tmp.Tz() = -Tz();
        return tmp;
    }
    T invDet = 1 / det;

    T d00 = t00 * invDet;
    T d10 = t10 * invDet;
    T d20 = t20 * invDet;
    T d30 = t30 * invDet;

    T d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    T d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    T d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    T d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m10 * m31 - m11 * m30;
    v1 = m10 * m32 - m12 * m30;
    v2 = m10 * m33 - m13 * m30;
    v3 = m11 * m32 - m12 * m31;
    v4 = m11 * m33 - m13 * m31;
    v5 = m12 * m33 - m13 * m32;

    T d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    T d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    T d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    T d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m21 * m10 - m20 * m11;
    v1 = m22 * m10 - m20 * m12;
    v2 = m23 * m10 - m20 * m13;
    v3 = m22 * m11 - m21 * m12;
    v4 = m23 * m11 - m21 * m13;
    v5 = m23 * m12 - m22 * m13;

    T d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    T d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    T d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    T d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    return Matrix4x4<T>(
        d00, d01, d02, d03,
        d10, d11, d12, d13,
        d20, d21, d22, d23,
        d30, d31, d32, d33);
}

template<typename T>
inline void Matrix4x4<T>::TransformHomogeneousAndScaleCorners(const Rect<T>& bounds, T sx, T sy, T* dest) const
{
    return TransformHomogeneousAndScaleCorners_NonOpt(bounds, sx, sy, dest);
}

template<typename T>
inline void Matrix4x4<T>::TransformHomogeneousAndScaleCorners_NonOpt(const Rect<T>& bounds, T sx, T sy, T* dest) const
{
    Point3F p1 = TransformHomogeneous(Point3F(bounds.x1, bounds.y1, 0));
    Point3F p2 = TransformHomogeneous(Point3F(bounds.x2, bounds.y1, 0));
    Point3F p3 = TransformHomogeneous(Point3F(bounds.x2, bounds.y2, 0));

    dest[0] = (sx * ( p1.x+1)/2.f);
    dest[1] = (sy * (-p1.y+1)/2.f);
    dest[2] = (sx * ( p2.x+1)/2.f);
    dest[3] = (sy * (-p2.y+1)/2.f);
    dest[4] = (sx * ( p3.x+1)/2.f);
    dest[5] = (sy * (-p3.y+1)/2.f);
}

//////////////////////////////////////////////////////////////////////////
// ** End Inline Implementation
//////////////////////////////////////////////////////////////////////////


//
// refcountable Matrix44
//
template <typename T>
class Matrix4x4Ref: public Matrix4x4<T>, public RefCountBase<Matrix4x4<T>, Stat_Default_Mem>
{
public:
    inline void operator = (const Matrix4x4<T> &mat)
    {
        memcpy(this->M, mat.M, sizeof(this->M));
    }
};

typedef Matrix4x4<float>    Matrix4F;
typedef Matrix4x4Ref<float> Matrix4FRef;


#if defined(SF_ENABLE_SIMD) || ((defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG))
// Now pre-declare any explicit specializations found in source files.
template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix4F &m2);
template<> void Matrix4F::MultiplyMatrix(const Matrix3F &m1, const Matrix4F &m2);
template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix3F &m2);
template<> void Matrix4F::MultiplyMatrix(const Matrix2F &m1, const Matrix4F &m2);
template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix2F &m2);
#endif

#if defined(SF_ENABLE_SIMD)
template<> void Matrix4F::EncloseTransformHomogeneous(RectF *pr, const RectF& r) const;
template<> void Matrix4F::TransformHomogeneousAndScaleCorners(const RectF& bounds, float sx, float sy, float* dest) const;
#endif

}} // Scaleform<T>::Render

#endif      // INC_SF_Render_Matrix4x4_H
