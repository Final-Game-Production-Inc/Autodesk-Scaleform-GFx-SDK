/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Twips.h
Content     :   
Created     :   January, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Twips_H
#define INC_SF_Render_Twips_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace Render {

template <typename T> T TwipsToPixels(T x) { return T((x) / T(20.)); }
template <typename T> T PixelsToTwips(T x) { return T((x) * T(20.)); }

// Specializations to use mult instead of div op
template <> inline float  TwipsToPixels(float x)  { return x * 0.05f; }
template <> inline double TwipsToPixels(double x) { return x * 0.05;  }

inline RectF TwipsToPixels(const RectF& x) 
{ 
    return RectF(TwipsToPixels(x.x1),  TwipsToPixels(x.y1), 
                 TwipsToPixels(x.x2),  TwipsToPixels(x.y2));
}

inline RectF PixelsToTwips(const RectF& x) 
{ 
    return RectF(PixelsToTwips(x.x1),  PixelsToTwips(x.y1), 
                 PixelsToTwips(x.x2),  PixelsToTwips(x.y2));
}

// Necessary for now
#define SF_TWIPS_TO_PIXELS(x)  TwipsToPixels(float(x))
#define SF_PIXELS_TO_TWIPS(x)  PixelsToTwips(float(x))

template <typename T>
int RoundTwips(T v)
{
    return Alg::IRound(v);
}

template <typename T>
int AlignTwipsToPixel(T v)
{
    return Alg::IRound(v);
}

}} // Scaleform::Render

#endif
