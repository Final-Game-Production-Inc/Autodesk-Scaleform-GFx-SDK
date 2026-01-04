/**************************************************************************

PublicHeader:   FontProvider_FT2
Filename    :   Render_FT2Helper.h
Content     :   Helper for FreeType2 font provider.
                Removes dependency on nonpublic headers from GFxFontProviderFT2 
Created     :   3/18/2009
Authors     :   Dmitry Polenur, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

----------------------------------------------------------------------
----The code of these classes was taken from the Anti-Grain Geometry
Project and modified for the use by Scaleform. 
Permission to use without restrictions is hereby granted to 
Scaleform Corporation by the author of Anti-Grain Geometry Project.
See http://antigtain.com for details.
**************************************************************************/

#ifndef INC_SF_GFX_FT2Helper_H
#define INC_SF_GFX_FT2Helper_H

#include "Kernel/SF_Types.h"
#include "Render/Render_Math2D.h"

namespace Scaleform { namespace Render {

class PathPacker;
class ShapeDataInterface;
class GlyphShape;
struct ShapePosInfo;

// see FT_Vector in ftimage.h
struct FTVector
{
    long x; // must be same type as FT_Pos
    long y;
};

// see FT_Outline in ftimage.h
struct  FTOutline
{
    SInt16      NContours;      // number of contours in glyph        
    SInt16      NPoints;        // number of points in the glyph      

    FTVector*   Points;         // the outline's points               
    char*       Tags;           // the points flags                   
    SInt16*     Contours;       // the contour end points             

    int         Flags;          // outline masks
};

// A helper class for GFxFontProviderFT2. Provides functionality to create shapes
// from FreeType's glyphs.
class FT2Helper
{
    enum 
    {
        FTCurveTagOn            = 1,        
        FTCurveTagConic         = 0,
        FTCurveTagCubic         = 2
    };
public:
    static void SF_STDCALL cubicToQuadratic(GlyphShape * pshape, ShapePosInfo * pos, int hintedGlyphSize, unsigned fontHeight, Render::CoordType x2, Render::CoordType y2, Render::CoordType x3, Render::CoordType y3, Render::CoordType x4, Render::CoordType y4);
    static bool SF_STDCALL decomposeGlyphOutline(const FTOutline& outline, GlyphShape* shape, unsigned fontHeight);

    //static ShapeDataInterface* SF_STDCALL CreateShape(unsigned shapePageSize, unsigned glyphSize);
    //static void SF_STDCALL ReleaseShape(ShapeDataInterface* shape);

private:
    static inline Render::CoordType SF_STDCALL FtToTwips(float v)
    {
        return float(((int)v * 20) >> 6);
    }

    static inline Render::CoordType SF_STDCALL FtToS1024(float v)
    {
        return float((int)v >> 6);
    }
    static inline Render::CoordType SF_STDCALL FtToTwips(long v)
    {
        return float((v * 20) >> 6);
    }

    static inline Render::CoordType SF_STDCALL FtToS1024(long v)
    {
        return float(v >> 6);
    }

    static inline char SF_STDCALL GetCurveTag(char flag)
    {
        return flag & 3;
    }
};

}} // namespace Scaleform { namespace GFx {

#endif //#ifndef INC_SF_GFX_FT2Helper_H
