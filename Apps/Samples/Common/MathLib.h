/**************************************************************************

Filename    :   MathLib.h
Content     :   Simple Matrix math
Created     :   
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_MATHLIB_H
#define INC_MATHLIB_H

void MatrixMult(float *po, const float *pa, const float *pb);
void VectorMult(float *po, const float *pa, float x, float y, float z=0, float w=1);
void VectorMult(float *po, const float *pa, const float *pv);
void VectorInvHomog(float *pv);
void MakePerspective(float *pp, float fov, float aspect, float z0, float z1);
void MakeIdentity(float *pp);
void MakeRotateY(float *pp, float angle);
void MakeRotateX(float *pp, float angle);
void MakeRotateZ(float *pp, float angle);
void Translate(float *pp, float x, float y, float z);
void MatrixTranspose(float *po, const float *pa);
void MatrixInverse(float *po, const float *pa);

#endif
