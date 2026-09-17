/**********************************************************************

PublicHeader:   Render
Filename    :   Render_MatrixState.h
Content     :   MatrixState defines the set of matrices required for the HAL
                to perform transformations on things it renders.
Created     :   2014/01/22
Authors     :   Bart Muzzin

Copyright   :   Copyright 2014 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#pragma once
#ifndef INC_Render_MatrixState_h
#define INC_Render_MatrixState_h

#include "Kernel/SF_RefCount.h"
#include "Render/Render_Matrix3x4.h"
#include "Render/Render_Matrix4x4.h"
#include "Render/Render_Viewport.h"

namespace Scaleform { namespace Render {

class HAL;

// 3D stereo support
enum StereoDisplay
{
    StereoCenter,       // the non-stereo case
    StereoLeft,         // use the left eye perspective
    StereoRight         // use the right eye perspective
};

struct StereoParams
{
    float DisplayWidthCm;           // the width of the display, computed internally
    float Distortion;               // 0 to 1, the strength of the stereo effect, defaults to 0.75
    float DisplayDiagInches;        // the diagonal size of the output display in inches, defaults to 52
    float DisplayAspectRatio;  
    float EyeSeparationCm;          // the distance between eyes in cm, defaults to 6.4
    StereoParams() : DisplayWidthCm(0), Distortion(0.75f), DisplayDiagInches(52.f), DisplayAspectRatio(9.f/16.f), EyeSeparationCm(6.4f) {  }
};

class StereoImplBase : public RefCountBase<StereoImplBase, Stat_Default_Mem>
{
protected:
    StereoParams S3DParams;

public:
    virtual void SetParams(const StereoParams& p) { S3DParams = p; }
    virtual const StereoParams& GetParams() { return S3DParams; }
    virtual void GetStereoProj(const Matrix4F& proj, float eyeZ, Matrix4F* left, Matrix4F* right, float factor = 1.0f) const;
};

class MatrixState : public RefCountBase<MatrixState, StatRender_Mem>
{
    friend class HAL;
public:


    Matrix2F            View2D;
    Matrix3F            View3D;
    Matrix4F            Proj3D;
    mutable Matrix4F    Proj3DLeft;        // 3D stereo support
    mutable Matrix4F    Proj3DRight;       // 3D stereo support

    Matrix2F            User;
    Matrix2F            User3D;
    Matrix2F            Orient2D;
    Matrix4F            Orient3D;

    Rect<int>           ViewRectOriginal;       // 'full screen' view rectangle.
    mutable Rect<int>   ViewRect;               // Current viewport rectangle.

    mutable Matrix2F    UserView;               // 'Final' concattenated User x View x Orientation matrix (for 2D shapes).
    mutable Matrix4F    UVPO;                   // 'Final' concattenated User x View x Projection x Orientation matrix (for 3D shapes).

    mutable Matrix4F    ViewRectCompensated3D;  // Matrix compensating for cullrect changes (reduced sized viewports).

    mutable bool        UVPOChanged;
    bool                OrientationSet;

    // 3D stereo support
    StereoDisplay       S3DDisplay; 
    Ptr<StereoImplBase> S3DImpl;

    HAL*     pHAL;

    MatrixState(HAL* phal);
    MatrixState();

    const Matrix4F&     GetUVP() const;
    const Matrix4F&     GetUVP( const Rect<int> & viewRect ) const;
    virtual Matrix2F&   GetFullViewportMatrix(const Size<int>& rtSize);

    virtual void        SetUserMatrix(const Matrix2F& user);
    virtual void        SetUserMatrix(const Matrix2F& user, const Matrix2F& user3D);
    virtual void        SetViewportMatrix(const Matrix2F& vp);
    virtual Viewport    SetOrientation(const Viewport& vp);
    virtual void        CopyFrom(MatrixState* state);
    virtual void        CopyTo(MatrixState* state);

    virtual void        Copy(MatrixState* outmat, MatrixState* inmat);

protected:
    virtual void        recalculateUVPOC() const;

    // 3D stereo support
    const               Matrix4F& updateStereoProjection(float factor = 1.0f) const;

    Matrix2F            FullViewportMVP;        // MVP for a 2D quad to fill the entire viewport.
};

// Class used for creating derived MatrixState objects
class MatrixStateFactory : public NewOverrideBase<StatRender_RenderPipeline_Mem>
{
public:
    MatrixStateFactory(MemoryHeap* heap = 0);
    virtual ~MatrixStateFactory() { }

    virtual MatrixState*       CreateMatrixState() const;
protected:
    MemoryHeap*                 pHeap;              // The heap in which MatrixState objects are allocated.
};

}} // Scaleform::Render

#endif
