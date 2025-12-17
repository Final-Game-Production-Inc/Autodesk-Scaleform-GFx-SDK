/**************************************************************************

PublicHeader:   Render
Filename    :   Render_CxForm.h
Content     :   Color transform for renderer.
Created     :   August 17, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_CxForm_H
#define INC_SF_Render_CxForm_H

#include "Render_Color.h"
#include "Kernel/SF_SIMD.h"

namespace Scaleform { namespace Render {

class Cxform
{
public:

    // 0,1,2,3 -> R,G,B,A
    SF_SIMD_ALIGN( float   M[2][4] );   // [mult, add][RGBA]

    enum NoInitType { NoInit };

    Cxform();
    inline Cxform(NoInitType) { }

    explicit Cxform(Color color)
    {
        M[0][0] = 0; M[0][1] = 0; M[0][2] = 0; M[0][3] = 0;
        M[1][0] = color.GetRed()/255.f; M[1][1] = color.GetGreen()/255.f;
        M[1][2] = color.GetBlue()/255.f; M[1][3] = color.GetAlpha()/255.f;
    }

    // Append applies our transformation before argument one.
    // Proper form: childCx.Append(parentCx).
    void    Append(const Cxform& c);
    void    Append_NonOpt(const Cxform& c);
    void    Prepend(const Cxform& c);
    void    Prepend_NonOpt(const Cxform& c);
    Color   Transform(const Color in) const;

    void    SetToAppend(const Cxform& c0, const Cxform& c1);
    void    SetToAppend_NonOpt(const Cxform& c0, const Cxform& c1);

    // Normalize transform from range 0...255 to 0...1
    void    Normalize();
    void    Normalize_NonOpt();
#if (defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG)
    void    Normalize_Opt(float *invScale);
#endif

    void    GetAsFloat2x4(float (*rows)[4]) const;
    void    GetAsFloat2x4Aligned(float (*rows)[4]) const;
    void    GetAsFloat2x4Aligned_NonOpt(float (*rows)[4]) const;

    void    SetIdentity();
    bool    IsIdentity() const;
    bool    operator == (const Cxform& x) const;
    bool    operator != (const Cxform& x) const;

    bool    RequiresBlend() const
    {
        return M[0][3] < 1.0f;
    }

    static Cxform   Identity;
};

}} // Scaleform::Render

#endif
