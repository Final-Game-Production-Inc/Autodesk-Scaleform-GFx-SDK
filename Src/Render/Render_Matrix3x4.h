/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Matrix3x4.h
Content     :   3D Matrix class
Created     :   November 15, 2010
Authors     :   Mustafa Thamer

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Matrix3x4_H
#define INC_SF_Render_Matrix3x4_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_SIMD.h"

#include "Render/Render_Types2D.h"
#include "Render/Render_Matrix2x4.h"
#include "Render/Render_Point3.h"
#include "Render/Render_Stats.h"

#include <string.h>     // memcpy

namespace Scaleform { namespace Render {

    //
    //    This matrix is composed of 3 rows and 4 columns with translation in the 4th column.
    //    It is laid out in row-major order (rows are stored one after the other in memory)
    //    The data is in the same format as Matrix2x4 and Matrix4x4.
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
    //
    //    | 00      01      02      03 |
    //    | 10      11      12      13 |
    //    | 20      21      22      23 |
    //
    //    in memory it looks like this {00, 01, 02, 03,  10, 11, 12, 13,  20, 21, 22, 23},
    //    ((float *)M)[4] = 10, and ((float *)M)[7] = 13, in general M[row][col] = M[row * 4 + col]
    //          (since 4 is the number of columns)
    //
    //    Basis vectors are the 1st 3 columns:
    //      B1 = {00, 10, 20} X axis
    //      B2 = {01, 11, 21} Y axis
    //      B3 = {02, 12, 22} Z axis
    //

    // fwd decl
    template <typename T>
    class Matrix4x4;

    // Extract data into a base class template, so it can be specialized when needed for alignment.
    template <typename T>
    class Matrix3x4Data
    {
    public:
        static const unsigned Rows = 3;
        T   M[Rows][4];
    };

    template <typename T>
    class Matrix3x4 : public Matrix3x4Data<T>
    {
    public:
        using Matrix3x4Data<T>::M;  // GCC 3.4 compatibility.

        static Matrix3x4    Identity;

        enum NoInitType { NoInit };

        // Construct matrix with no initializer; later assignment expected.
        Matrix3x4(NoInitType) {  }
        // construct identity
        Matrix3x4()                              { SetIdentity(); }
        // construct from data
        Matrix3x4(const T *pVals, int count=12)  { Set(pVals, count);  }
        Matrix3x4(const T pVals[3][4])           { Set(&pVals[0][0], 12);  }
        inline Matrix3x4(
            T v1, T v2, T v3, T v4,
            T v5, T v6, T v7, T v8,
            T v9, T v10, T v11, T v12);
        // copy constructor
        Matrix3x4(const Matrix3x4 &mat)          { *this = mat;  }
        // construct from Matrix2x4
        Matrix3x4(const Matrix2x4<T> &m);

        // construct from Matrix4x4
        Matrix3x4(const Matrix4x4<T> &m);

        // constructors for multiplication
        Matrix3x4(const Matrix3x4 &m1, const Matrix3x4 &m2)     { MultiplyMatrix(m1, m2); }
        Matrix3x4(const Matrix3x4 &m1, const Matrix2x4<T> &m2)  { MultiplyMatrix(m1, m2); }
        Matrix3x4(const Matrix2x4<T> &m1, const Matrix3x4 &m2)  { MultiplyMatrix(m1, m2); }

        // Checks if all matrix values are within the -MAX..MAX value range
        SF_EXPORT bool IsValid() const;

        // array accessors, [i][j] is equivalent to [i*4+j]
        operator T * ()                 { return Data(); }    // conversion operator
        operator const T * () const     { return Data(); }    // conversion operator
        T *Data()                       { return (T*)M;  }
        const T *Data() const           { return (T*)M;  }
        T operator [] (int pos)         { return Data()[pos]; }
        T operator [] (int pos) const   { return Data()[pos]; }

        void Set(const T *pVals, int count) { SF_ASSERT(count<=12); memcpy(M, pVals, count*sizeof(T));   }
        void Set(const T pVals[3][4])       { Set(&pVals[0][0], 12); }
        void Set(const Matrix3x4 &mat)      { Set(&mat.M[0][0], 12); }

        SF_EXPORT void Transpose();
        SF_EXPORT void Transpose(Matrix4x4<T> *dest);
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
        SF_EXPORT T    GetScale() const;

        // static, create scale matrix
        static inline Matrix3x4<T> Scaling(T sX, T sY, T sZ);

        // Translation
        SF_EXPORT void GetTranslation(T *tX, T *tY, T *tZ) const;

        T &Tx()                             { return M[0][3]; }
        T &Ty()                             { return M[1][3]; }
        T &Tz()                             { return M[2][3]; }

        const T &Tx() const                 { return M[0][3]; }
        const T &Ty() const                 { return M[1][3]; }
        const T &Tz() const                 { return M[2][3]; }

        // static, create translation matrix
        static inline Matrix3x4<T> Translation(T tX, T tY, T tZ);

        // Multiplication (matches Matrix2x4 ordering as well)
        SF_EXPORT void MultiplyMatrix(const Matrix3x4 &m1, const Matrix3x4 &m2);
        SF_EXPORT void MultiplyMatrix_NonOpt(const Matrix3x4 &m1, const Matrix3x4 &m2);

        // Multiplication 3x4 * 2x4
        SF_EXPORT void MultiplyMatrix(const Matrix3x4 &m1, const Matrix2x4<T> &m2);
        SF_EXPORT void MultiplyMatrix_NonOpt(const Matrix3x4 &m1, const Matrix2x4<T> &m2);

        // Multiplication 2x4 * 3x4
        SF_EXPORT void MultiplyMatrix(const Matrix2x4<T> &m1, const Matrix3x4 &m2);
        SF_EXPORT void MultiplyMatrix_NonOpt(const Matrix2x4<T> &m1, const Matrix3x4 &m2);

#if (defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG)
        SF_EXPORT void MultiplyMatrix_Opt(const Matrix2x4<T> &m1, const Matrix3x4 &m2, float* mask);
        SF_EXPORT void MultiplyMatrix_Opt(const Matrix3x4 &m1, const Matrix3x4 &m2, float* mask);
#endif

        friend const Matrix3x4      operator * (const Matrix3x4 &m1, const Matrix3x4 &m2)
        {
            Matrix3x4 outMat(NoInit);
            outMat.MultiplyMatrix(m1, m2);          // m1.Prepend(m2) or m2.append(m1), apply m2 then m1
            return outMat;
        }

        friend const Matrix3x4      operator * (const Matrix3x4 &m1, const Matrix2x4<T> &m2)
        {
            Matrix3x4 outMat(NoInit);
            outMat.MultiplyMatrix(m1, m2);          // m1.Prepend(m2) or m2.append(m1), apply m2 then m1
            return outMat;
        }

        friend const Matrix3x4      operator * (const Matrix2x4<T> &m1, const Matrix3x4 &m2)
        {
            Matrix3x4 outMat(NoInit);
            outMat.MultiplyMatrix(m1, m2);          // m1.Prepend(m2) or m2.append(m1), apply m2 then m1
            return outMat;
        }

        void Prepend(const Matrix3x4 &matrixA)
        {
            Matrix3x4 matrixThis = *this;
            MultiplyMatrix(matrixThis, matrixA);
        }

        void Prepend(const Matrix2x4<T> &matrixA)
        {
            Matrix3x4 matrixThis = *this;
            MultiplyMatrix(matrixThis, matrixA);
        }

        void Append(const Matrix3x4 &matrixA)
        {
            Matrix3x4 matrixThis = *this;
            MultiplyMatrix(matrixA, matrixThis);
        }

        void Append(const Matrix2x4<T> &matrixA)
        {
            Matrix3x4 matrixThis = *this;
            MultiplyMatrix(matrixA, matrixThis);
        }

        void SetToAppend(const Matrix3x4 &matrixA, const Matrix3x4 &matrixB)
        {
            MultiplyMatrix(matrixB, matrixA);
        }

        void SetToAppend(const Matrix3x4 &matrixA, const Matrix2x4<T> &matrixB)
        {
            MultiplyMatrix(matrixB, matrixA);
        }

        void SetToAppend(const Matrix2x4<T> &matrixA, const Matrix3x4 &matrixB)
        {
            MultiplyMatrix(matrixB, matrixA);
        }

        void SetToPrepend(const Matrix3x4 &matrixA, const Matrix3x4 &matrixB)
        {
            MultiplyMatrix(matrixA, matrixB);
        }

        void SetToPrepend(const Matrix3x4 &matrixA, const Matrix2x4<T> &matrixB)
        {
            MultiplyMatrix(matrixA, matrixB);
        }

        void SetToPrepend(const Matrix2x4<T> &matrixA, const Matrix3x4 &matrixB)
        {
            MultiplyMatrix(matrixA, matrixB);
        }

        // Matrix equality
        inline friend bool                operator == (const Matrix3x4 &m1, const Matrix3x4 &m2)
        {
            return memcmp(m1.M, m2.M, sizeof(Matrix3x4<T>)) == 0;
        }
        inline friend bool                operator != (const Matrix3x4 &m1, const Matrix3x4 &m2)
        {
            return  !(m1 == m2);
        }

        // assignment
        inline const Matrix3x4&            operator =  (const Matrix3x4 &m)
        {
            Set(m);
            return *this;
        }

        // Inverse
        inline Matrix3x4<T> GetInverse() const;
        void  SetInverse(const Matrix3x4& mIn)  { *this = mIn.GetInverse(); }
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

        Point3<T>     TransformNormal(const Point3<T>& p) const
        {
            Matrix3x4 invMat = GetInverse();
            invMat.Transpose();
            return invMat.Transform(p);
        }

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
            Matrix3x4 matCopy = GetInverse();
            return matCopy.Transform(p);
        }

        SF_EXPORT void      EncloseTransform(Rect<T> *pr, const Rect<T>& r) const;
        SF_EXPORT Rect<T>   EncloseTransform(const Rect<T>& r) const { Rect<T> d(Rect<T>::NoInit); EncloseTransform(&d, r); return d; }

        // Rotation
        // returns euler angles in radians
        void GetRotation(T *eX, T *eY, T *eZ) const { return GetEulerAngles(eX, eY, eZ); }

        // static initializers, create rotation matrix
        inline static Matrix3x4<T> RotationX(T angleRad);
        inline static Matrix3x4<T> RotationY(T angleRad);
        inline static Matrix3x4<T> RotationZ(T angleRad);
        inline static Matrix3x4<T> Rotation(T angleRad, const Point3<T>& axis);
        inline static Matrix3x4<T> Rotation(T angleRad, const Point3<T>& axis, const Point3<T>& pivot);

        // create camera view matrix, world to view transform. Right or Left-handed.
        SF_EXPORT void ViewRH(const Point3<T>& eyePt, const Point3<T>& lookAtPt, const Point3<T>& upVec);
        SF_EXPORT void ViewLH(const Point3<T>& eyePt, const Point3<T>& lookAtPt, const Point3<T>& upVec);
        SF_EXPORT void View  (const Point3<T>& eyePt, const Point3<T>& viewVector, const Point3<T>& upVec);

        // create perspective matrix, view to screen transform.  Right or Left-handed.
        SF_EXPORT void PerspectiveRH(T fovYRad, T fAR, T fNearZ, T fFarZ);
        SF_EXPORT void PerspectiveLH(T fovYRad, T fAR, T fNearZ, T fFarZ);
        SF_EXPORT void PerspectiveViewVolRH(T viewW, T viewH, T fNearZ, T fFarZ);
        SF_EXPORT void PerspectiveViewVolLH(T viewW, T viewH, T fNearZ, T fFarZ);
        SF_EXPORT void PerspectiveOffCenterLH(T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ);
        SF_EXPORT void PerspectiveOffCenterRH(T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ);

        // create orthographic projection matrix.  Right or Left-handed.
        SF_EXPORT void OrthoRH(T viewW, T viewH, T fNearZ, T fFarZ);
        SF_EXPORT void OrthoLH(T viewW, T viewH, T fNearZ, T fFarZ);
        SF_EXPORT void OrthoOffCenterRH(T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ);
        SF_EXPORT void OrthoOffCenterLH(T viewMinX, T viewMaxX, T viewMinY, T viewMaxY, T fNearZ, T fFarZ);

    private:

        void GetEulerAngles(T *eX, T *eY, T *eZ) const;
    };

    //////////////////////////////////////////////////////////////////////////
    // begin INLINE implementation
    //////////////////////////////////////////////////////////////////////////

    // static identity
    template<typename T>
    Matrix3x4<T>   Matrix3x4<T>::Identity;

    // ctor from Matrix2x4
    template<typename T>
    inline Matrix3x4<T>::Matrix3x4(const Matrix2x4<T> &m)
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
    }

    // construct from Matrix4x4
    template<typename T>
    inline Matrix3x4<T>::Matrix3x4(const Matrix4x4<T> &m)
    {
        M[0][0] = m.M[0][0];   // SX
        M[0][1] = m.M[0][1];
        M[0][2] = m.M[0][2];
        M[0][3] = m.M[0][3];   // TX

        M[1][0] = m.M[1][0];
        M[1][1] = m.M[1][1];   // SY
        M[1][2] = m.M[1][2];
        M[1][3] = m.M[1][3];   // TY

        M[2][0] = m.M[2][0];
        M[2][1] = m.M[2][1];
        M[2][2] = m.M[2][2];   // SZ
        M[2][3] = m.M[2][3];   // TZ
    }

    // construct from data elements
    template<typename T>
    inline Matrix3x4<T>::Matrix3x4(
        T v1, T v2, T v3, T v4,
        T v5, T v6, T v7, T v8,
        T v9, T v10, T v11, T v12
        )
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
    }

    // is the matrix valid?
    template<typename T>
    inline bool    Matrix3x4<T>::IsValid() const
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
            && SF_ISFINITE(M[2][3]);
    }

    // transpose the 3x3 square matrix in place
    template<typename T>
    inline void Matrix3x4<T>::Transpose()
    {
        Matrix3x4<T> matDest(NoInit);
        int i,j;
        for(i=0;i<3;i++)
            for(j=0;j<3;j++)
                matDest.M[j][i] = M[i][j];

        matDest.M[0][3] = matDest.M[1][3] = matDest.M[2][3] = 0;

        *this = matDest;
    }

    // transpose the matrix into the 4x4 provided
    template<typename T>
    inline void Matrix3x4<T>::Transpose(Matrix4x4<T> *matDest)
    {
        int i,j;
        for(i=0;i<3;i++)
            for(j=0;j<3;j++)
                matDest->M[j][i] = M[i][j];

        (*matDest).M[0][3] = (*matDest).M[1][3] = (*matDest).M[2][3] = 0;

        (*matDest).M[3][0] = M[0][3];
        (*matDest).M[3][1] = M[1][3];
        (*matDest).M[3][2] = M[2][3];
        (*matDest).M[3][3] = 1;
    }

    // set to identity
    template<typename T>
    inline void Matrix3x4<T>::SetIdentity()
    {
        Clear();

        M[0][0] = 1;
        M[1][1] = 1;
        M[2][2] = 1;
    }

    template<typename T>
    inline void Matrix3x4<T>::MultiplyMatrix(const Matrix3x4<T> &m1, const Matrix3x4<T> &m2)
    {
        MultiplyMatrix_NonOpt(m1,m2);
    }

    template<typename T>
    inline void Matrix3x4<T>::MultiplyMatrix(const Matrix3x4<T> &m1, const Matrix2x4<T> &m2)
    {
        MultiplyMatrix_NonOpt(m1,m2);
    }

    template<typename T>
    inline void Matrix3x4<T>::MultiplyMatrix(const Matrix2x4<T> &m1, const Matrix3x4<T> &m2)
    {
        MultiplyMatrix_NonOpt(m1,m2);
    }

    // multiply 2 matrices
    template<typename T>
    inline void Matrix3x4<T>::MultiplyMatrix_NonOpt(const Matrix3x4<T> &m1, const Matrix3x4<T> &m2)
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
    }

    // multiply 2 matrices
    template<typename T>
    inline void Matrix3x4<T>::MultiplyMatrix_NonOpt(const Matrix3x4<T> &m1, const Matrix2x4<T> &m2)
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
    }

    // multiply 2 matrices
    template<typename T>
    inline void Matrix3x4<T>::MultiplyMatrix_NonOpt(const Matrix2x4<T> &m1, const Matrix3x4<T> &m2)
    {
        M[0][0] = m1.M[0][0] * m2.M[0][0] + m1.M[0][1] * m2.M[1][0] + m1.M[0][2] * m2.M[2][0];
        M[0][1] = m1.M[0][0] * m2.M[0][1] + m1.M[0][1] * m2.M[1][1] + m1.M[0][2] * m2.M[2][1];
        M[0][2] = m1.M[0][0] * m2.M[0][2] + m1.M[0][1] * m2.M[1][2] + m1.M[0][2] * m2.M[2][2];
        M[0][3] = m1.M[0][0] * m2.M[0][3] + m1.M[0][1] * m2.M[1][3] + m1.M[0][2] * m2.M[2][3] + m1.M[0][3];

        M[1][0] = m1.M[1][0] * m2.M[0][0] + m1.M[1][1] * m2.M[1][0] + m1.M[1][2] * m2.M[2][0];
        M[1][1] = m1.M[1][0] * m2.M[0][1] + m1.M[1][1] * m2.M[1][1] + m1.M[1][2] * m2.M[2][1];
        M[1][2] = m1.M[1][0] * m2.M[0][2] + m1.M[1][1] * m2.M[1][2] + m1.M[1][2] * m2.M[2][2];
        M[1][3] = m1.M[1][0] * m2.M[0][3] + m1.M[1][1] * m2.M[1][3] + m1.M[1][2] * m2.M[2][3] + m1.M[1][3];

        M[2][0] = m2.M[2][0];
        M[2][1] = m2.M[2][1];
        M[2][2] = m2.M[2][2];
        M[2][3] = m2.M[2][3];
    }

    // Get euler rot angles
    template<typename T>
    inline void Matrix3x4<T>::GetEulerAngles(T *eX, T *eY, T *eZ) const
    {
        // Unscale the matrix before extracting term
        Matrix3x4<T> copy(*this);

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
    inline Matrix3x4<T> Matrix3x4<T>::RotationX(T angle)
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
    inline Matrix3x4<T> Matrix3x4<T>::RotationY(T angle)
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
    inline Matrix3x4<T> Matrix3x4<T>::RotationZ(T angle)
    {
        Matrix3x4<T> mat;

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
    inline Matrix3x4<T> Matrix3x4<T>::Rotation(T angle, const Point3<T>& axis)
    {
        Matrix3x4<T> mat;

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
    inline Matrix3x4<T> Matrix3x4<T>::Rotation(T angle, const Point3<T>& axis, const Point3<T>& pivot)
    {
        // TODO - compute pre-multiplied version
        return
            Matrix3x4<T>(
                Matrix3x4<T>::Translation(-pivot.x, -pivot.y, -pivot.z),
                Matrix3x4<T>(
                    Matrix3x4<T>::Rotation(angle, axis),
                    Matrix3x4<T>::Translation(pivot.x, pivot.y, pivot.z)
                )
            );
    }


    // Return translation matrix
    template<typename T>
    inline Matrix3x4<T> Matrix3x4<T>::Translation(T tX, T tY, T tZ)
    {
        Matrix3x4<T> mat;

        mat.M[0][3] = tX;
        mat.M[1][3] = tY;
        mat.M[2][3] = tZ;

        return mat;
    }

    // Get translation values
    template<typename T>
    inline void Matrix3x4<T>::GetTranslation(T *tX, T *tY, T *tZ) const
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
    inline Matrix3x4<T> Matrix3x4<T>::Scaling(T sX, T sY, T sZ)
    {
        Matrix3x4<T> mat;
        mat.M[0][0] = sX;
        mat.M[1][1] = sY;
        mat.M[2][2] = sZ;
        return mat;
    }

    // Get scale values
    template<typename T>
    inline void Matrix3x4<T>::GetScale(T *tX, T *tY, T *tZ) const
    {
        if (tX)
            *tX = GetXScale();
        if (tY)
            *tY = GetYScale();
        if (tZ)
            *tZ = GetZScale();
    }

    template<typename T>
    inline T Matrix3x4<T>::GetScale() const
    {
        T x, y, z;
        GetScale(&x, &y, &z);
        return sqrtf(x*x + y*y + z*z) * 0.707106781f;
    }

    // create camera view matrix
    template<typename T>
    inline void Matrix3x4<T>::ViewRH(const Point3<T>& eyePt, const Point3<T>& lookAtPt, const Point3<T>& upVec)
    {
        // view direction
        Point3<T> zAxis = (eyePt - lookAtPt);
        zAxis.Normalize();
        View(eyePt, zAxis, upVec);
    }

    // create camera view matrix
    template<typename T>
    inline void Matrix3x4<T>::ViewLH(const Point3<T>& eyePt, const Point3<T>& lookAtPt, const Point3<T>& upVec)
    {
        // view direction
        Point3<T> zAxis = (lookAtPt - eyePt);
        zAxis.Normalize();
        View(eyePt, zAxis, upVec);
    }

    template<typename T>
    inline void Matrix3x4<T>::View(const Point3<T>& eyePt, const Point3<T>& zAxis, const Point3<T>& upVec)
    {
        // right direction
        Point3<T> xAxis;
        xAxis.Cross(upVec, zAxis);
        xAxis.Normalize();

        // up direction
        Point3<T> yAxis;
        yAxis.Cross(zAxis, xAxis);
        //    yAxis.Normalize();

        M[0][0] = xAxis.x;
        M[0][1] = xAxis.y;
        M[0][2] = xAxis.z;
        M[0][3] = -xAxis.Dot(eyePt);

        M[1][0] = yAxis.x;
        M[1][1] = yAxis.y;
        M[1][2] = yAxis.z;
        M[1][3] = -yAxis.Dot(eyePt);

        M[2][0] = zAxis.x;
        M[2][1] = zAxis.y;
        M[2][2] = zAxis.z;
        M[2][3] = -zAxis.Dot(eyePt);
    }

    // This is an axial bound of an oriented (and/or
    // sheared, scaled, etc) box.
    template<typename T>
    inline void    Matrix3x4<T>::EncloseTransform(Rect<T> *pr, const Rect<T>& r) const
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

    // Returns the inverse matrix
    template<typename T>
    inline Matrix3x4<T> Matrix3x4<T>::GetInverse() const
    {
        T m00 = M[0][0], m01 = M[0][1], m02 = M[0][2], m03 = M[0][3];
        T m10 = M[1][0], m11 = M[1][1], m12 = M[1][2], m13 = M[1][3];
        T m20 = M[2][0], m21 = M[2][1], m22 = M[2][2], m23 = M[2][3];
        T m30 = 0, m31 = 0, m32 = 0, m33 = 1;

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
            Matrix3x4<T> tmp;   // identity
            tmp.Tx() = -Tx();
            tmp.Ty() = -Ty();
            tmp.Tz() = -Tz();
            return tmp;
        }
        T invDet = 1 / det;

        T d00 = t00 * invDet;
        T d10 = t10 * invDet;
        T d20 = t20 * invDet;
//        T d30 = t30 * invDet;

        T d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        T d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        T d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
//        T d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m10 * m31 - m11 * m30;
        v1 = m10 * m32 - m12 * m30;
        v2 = m10 * m33 - m13 * m30;
        v3 = m11 * m32 - m12 * m31;
        v4 = m11 * m33 - m13 * m31;
        v5 = m12 * m33 - m13 * m32;

        T d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        T d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        T d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
//        T d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m21 * m10 - m20 * m11;
        v1 = m22 * m10 - m20 * m12;
        v2 = m23 * m10 - m20 * m13;
        v3 = m22 * m11 - m21 * m12;
        v4 = m23 * m11 - m21 * m13;
        v5 = m23 * m12 - m22 * m13;

        T d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        T d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        T d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
//        T d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        return Matrix3x4<T>(
            d00, d01, d02, d03,
            d10, d11, d12, d13,
            d20, d21, d22, d23
            );
    }

    //////////////////////////////////////////////////////////////////////////
    // ** End Inline Implementation
    //////////////////////////////////////////////////////////////////////////

    //
    // refcountable Matrix34
    //
    template <typename T>
    class Matrix3x4Ref: public Matrix3x4<T>, public RefCountBase<Matrix3x4<T>, Stat_Default_Mem>
    {
    public:
        inline void operator = (const Matrix3x4<T> &mat)
        {
            memcpy(this->M, mat.M, sizeof(this->M));
        }
    };

    typedef Matrix3x4<float> Matrix3F;
    typedef Matrix3x4Ref<float> Matrix3FRef;

#ifdef SF_ENABLE_SIMD
    // Alignment specialization
    template<>
    class Matrix3x4Data<float>
    {
    public:
        static const unsigned Rows = 3;
        SF_SIMD_ALIGN( float M[Rows][4] );
    };
#endif

#if defined(SF_ENABLE_SIMD) || ((defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG))
    // Now pre-declare any explicit specializations found in source files.
    template<> void Matrix3F::MultiplyMatrix(const Matrix3F&m1, const Matrix2F &m2);
    template<> void Matrix3F::MultiplyMatrix(const Matrix3F&m1, const Matrix3F &m2);
    template<> void Matrix3F::MultiplyMatrix(const Matrix2F&m1, const Matrix3F &m2);
#endif

#if (defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG)
    template<> void Matrix3F::MultiplyMatrix_Opt(const Matrix3F&m1, const Matrix3F &m2, float* mask);
    template<> void Matrix3F::MultiplyMatrix_Opt(const Matrix2F&m1, const Matrix3F &m2, float* mask);
#endif

}} // Scaleform<T>::Render

#endif      // INC_SF_Render_Matrix3x4_H
