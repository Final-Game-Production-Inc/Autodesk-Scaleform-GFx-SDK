/**************************************************************************

PublicHeader:   Render
Filename    :   Render_ScreenToWorld.h
Content     :   Screen to World reverse transform helper class 
Created     :   Jan 15, 2010
Authors     :   Mustafa Thamer

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_SCREENTOWORLD_H
#define INC_SF_Render_SCREENTOWORLD_H

#include "Render/Render_Types2D.h"
#include "Render/Render_Matrix4x4.h"

namespace Scaleform { namespace Render {

//
// This class computes a screen to world transform.
// It requires the following inputs to be set:
//      normalized screen coords - these are from the mouse generally and should be -1 to 1
//      view, perspective and word matrices - these are what are set in the renderer and on the object
//
class ScreenToWorld
{
public:

    ScreenToWorld() : Sx(FLT_MAX), Sy(FLT_MAX), LastX(FLT_MAX), LastY(FLT_MAX) { }

    // required inputs
    void SetNormalizedScreenCoords(float nsx, float nsy) { Sx=nsx; Sy=-nsy; }   // note: the Y is inverted
    void SetView(const Matrix3F &mView)         { MatView = mView; }
    void SetProjection(const Matrix4F &mProj)   { MatProj = mProj; }
    void SetWorld(const Matrix3F &mWorld)       { MatWorld = mWorld; }

    // computes the answer
    void GetWorldPoint(PointF *ptOut);
    void GetWorldPoint(Point3F *ptOut);

    PointF GetLastWorldPoint() const { return PointF(LastX, LastY); }
private:
    void VectorMult(float *po, const float *pa, float x, float y, float z, float w);
    void VectorMult(float *po, const float *pa, const float *v);
    void VectorInvHomog(float *v);

    float Sx, Sy;
    float LastX, LastY;
    Matrix4F MatProj;
    Matrix3F MatView;
    Matrix3F MatWorld;
    Matrix4F MatInvProj;
};

}}

#endif  // INC_SF_Render_SCREENTOWORLD_H

