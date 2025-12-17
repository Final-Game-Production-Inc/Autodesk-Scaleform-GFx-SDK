/**************************************************************************

Filename    :   MathLib.cpp
Content     :   Simple Matrix math
Created     :   
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Math.h"

#define M(x,r,c) x[(r)*4+(c)]

void MatrixMult(float *po, const float *pa, const float *pb)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(po,i,j) = M(pa, i,0) * M(pb, 0,j) + M(pa, i,1) * M(pb, 1,j) + 
                        M(pa, i,2) * M(pb, 2,j) + M(pa, i,3) * M(pb, 3,j);
}

void VectorMult(float *po, const float *pa, float x, float y, float z, float w)
{
    po[0] = M(pa,0,0) * x + M(pa,1,0) * y + M(pa,2,0) * z + M(pa,3,0) * w;
    po[1] = M(pa,0,1) * x + M(pa,1,1) * y + M(pa,2,1) * z + M(pa,3,1) * w;
    po[2] = M(pa,0,2) * x + M(pa,1,2) * y + M(pa,2,2) * z + M(pa,3,2) * w;
    po[3] = M(pa,0,3) * x + M(pa,1,3) * y + M(pa,2,3) * z + M(pa,3,3) * w;
}

void VectorMult(float *po, const float *pa, const float *v)
{
    po[0] = M(pa,0,0) * v[0] + M(pa,1,0) * v[1] + M(pa,2,0) * v[2] + M(pa,3,0) * v[3];
    po[1] = M(pa,0,1) * v[0] + M(pa,1,1) * v[1] + M(pa,2,1) * v[2] + M(pa,3,1) * v[3];
    po[2] = M(pa,0,2) * v[0] + M(pa,1,2) * v[1] + M(pa,2,2) * v[2] + M(pa,3,2) * v[3];
    po[3] = M(pa,0,3) * v[0] + M(pa,1,3) * v[1] + M(pa,2,3) * v[2] + M(pa,3,3) * v[3];
}

void VectorInvHomog(float *v)
{
    v[0] *= v[3];
    v[1] *= v[3];
    v[2] *= v[3];
    v[3] = 1;
}

void MakePerspective(float *pp, float fov, float aspect, float z0, float z1)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(pp,i,j) = 0;

    float dz = z1-z0;
    float sinfov = sin(fov*0.5f);
    float cotfov = cos(fov*0.5f) / sinfov;

    M(pp,0,0) = cotfov/aspect;
    M(pp,1,1) = cotfov;
    M(pp,2,2) = -(z0 + z1) / dz;
    M(pp,2,3) = -1;
    M(pp,3,2) = -2 * z0 * z1 / dz;
}

void MakeIdentity(float *pp)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(pp,i,j) = 0;

    M(pp,0,0) = 1;
    M(pp,1,1) = 1;
    M(pp,2,2) = 1;
    M(pp,3,3) = 1;
}

void MakeRotateY(float *pp, float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(pp,i,j) = 0;

    M(pp,0,0) = c;
    M(pp,2,2) = c;
    M(pp,2,0) = -s;
    M(pp,0,2) = s;
    M(pp,1,1) = 1;
    M(pp,3,3) = 1;
}

void MakeRotateX(float *pp, float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(pp,i,j) = 0;

    M(pp,1,1) = c;
    M(pp,2,2) = c;
    M(pp,2,1) = -s;
    M(pp,1,2) = s;
    M(pp,0,0) = 1;
    M(pp,3,3) = 1;
}


void MakeRotateZ(float *pp, float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(pp,i,j) = 0;

    M(pp,0,0) = c;
    M(pp,2,2) = c;
    M(pp,2,0) = -s;
    M(pp,0,2) = s;
    M(pp,1,1) = 1;
    M(pp,3,3) = 1;
}

void Translate(float *pp, float x, float y, float z)
{
    for (int i = 0; i < 4; i++)
        M(pp, 3,i) += x * M(pp, 0,i) + y * M(pp, 1,i) + z * M(pp, 2,i);
}

void MatrixTranspose(float *po, const float *pa)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(po,i,j) = M(pa,j,i);

}

float Cofactor(const float *pa, int i, int j)
{
    const int subs[4][3] = {{1,2,3},{0,2,3},{0,1,3},{0,1,2}};

    #define SUBM(m,a,b) M(m,subs[i][a],subs[j][b])

    float a;
    a =  SUBM(pa, 0,0) * SUBM(pa, 1,1) * SUBM(pa, 2,2);
    a += SUBM(pa, 1,0) * SUBM(pa, 2,1) * SUBM(pa, 0,2);
    a += SUBM(pa, 2,0) * SUBM(pa, 0,1) * SUBM(pa, 1,2);
    a -= SUBM(pa, 0,0) * SUBM(pa, 2,1) * SUBM(pa, 1,2);
    a -= SUBM(pa, 1,0) * SUBM(pa, 0,1) * SUBM(pa, 2,2);
    a -= SUBM(pa, 2,0) * SUBM(pa, 1,1) * SUBM(pa, 0,2);

    return ((i + j) & 1) ? -a : a;
}

void MatrixInverse(float *po, const float *pa)
{
    float c[16];
    float det = 0;

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(c,i,j) = Cofactor(pa, i,j);

    for (int i = 0; i < 4; i++)
        det += M(c, 0,i) * M(pa, 0,i);

    det = 1.0f / det;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M(po, j,i) = det * M(c, i,j);
}
