/**************************************************************************

Filename    :   Render_ScreenToWorld.cpp
Content     :   Clip to model space, reverse transform helper class 
Created     :   Jan 15, 2010
Authors     :   Mustafa Thamer

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Render_ScreenToWorld.h"

#define M(x,r,c) x[(r)+(c)*4]

namespace Scaleform { namespace Render {


void ScreenToWorld::GetWorldPoint(PointF *ptOut)
{
    Point3F pt3;
    GetWorldPoint(&pt3);
    ptOut->x = pt3.x;
    ptOut->y = pt3.y;
}

void ScreenToWorld::GetWorldPoint(Point3F *ptOut)
{
    if (Sx == FLT_MAX || Sy == FLT_MAX)
          return;

    Point3F v, vPickRayDir, vPickRayOrig;
    Matrix4F invMVMat, MVMat;

    // compute inverse projection, clip to view
    MatInvProj.SetInverse(MatProj);

    // compute inverse world*view, view to model
    MVMat = Matrix3F(MatView, MatWorld);
    invMVMat.SetInverse(MVMat);

    // point to matrix arrays directly
    float *InvMV = (float*)invMVMat;
    float *InvProj = (float*)MatInvProj;
    const float *Proj = (const float*)MatProj;

    float x1p[4], x1o[4];
    float x2p[4], x2o[4];
    float wz1[4], wz2[4];

    // set up ray end points (Z planes) in view space, then project to clip space
    VectorMult(wz1, Proj, 0, 0, -0.5, 1);
    VectorMult(wz2, Proj, 0, 0, -100.0, 1);     // preserve precision by using a larger range

    // transform screen input coords back to view space, project onto planes, make homogeneous (scale by w) 
    VectorMult(x1p, InvProj, Sx*wz1[3], Sy*wz1[3], wz1[2]*wz1[3], wz1[3]);
    VectorMult(x2p, InvProj, Sx*wz2[3], Sy*wz2[3], wz2[2]*wz2[3], wz2[3]);
    VectorInvHomog(x1p);
    VectorInvHomog(x2p);

    // now project pane points from view to model space
    VectorMult(x1o, InvMV, x1p);
    VectorMult(x2o, InvMV, x2p);

    // compute deltas and ray length
    float dx = x2o[0]-x1o[0];
    float dy = x2o[1]-x1o[1];
    float dz = x2o[2]-x1o[2];
    float mz = (dz != 0) ? -x1o[2]/dz : 0.0f;

    // compute point in model space along ray
    LastX = ptOut->x = x1o[0] + mz * dx;
    LastY = ptOut->y = x1o[1] + mz * dy;
            ptOut->z = x1o[2] + mz * dz;    // ~0
}

//
// utility Vector4 routines
//
void ScreenToWorld::VectorMult(float *po, const float *pa, float x, float y, float z, float w)
{
    po[0] = M(pa,0,0) * x + M(pa,1,0) * y + M(pa,2,0) * z + M(pa,3,0) * w;
    po[1] = M(pa,0,1) * x + M(pa,1,1) * y + M(pa,2,1) * z + M(pa,3,1) * w;
    po[2] = M(pa,0,2) * x + M(pa,1,2) * y + M(pa,2,2) * z + M(pa,3,2) * w;
    po[3] = M(pa,0,3) * x + M(pa,1,3) * y + M(pa,2,3) * z + M(pa,3,3) * w;
}

void ScreenToWorld::VectorMult(float *po, const float *pa, const float *v)
{
    po[0] = M(pa,0,0) * v[0] + M(pa,1,0) * v[1] + M(pa,2,0) * v[2] + M(pa,3,0) * v[3];
    po[1] = M(pa,0,1) * v[0] + M(pa,1,1) * v[1] + M(pa,2,1) * v[2] + M(pa,3,1) * v[3];
    po[2] = M(pa,0,2) * v[0] + M(pa,1,2) * v[1] + M(pa,2,2) * v[2] + M(pa,3,2) * v[3];
    po[3] = M(pa,0,3) * v[0] + M(pa,1,3) * v[1] + M(pa,2,3) * v[2] + M(pa,3,3) * v[3];
}

void ScreenToWorld::VectorInvHomog(float *v)
{
    v[0] *= v[3];
    v[1] *= v[3];
    v[2] *= v[3];
    v[3] = 1;
}

}}
