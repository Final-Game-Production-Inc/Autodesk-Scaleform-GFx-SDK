/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TessCurves.h
Content     :   Quadratic and Cubic cuves tessellation
Created     :
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TessCurves_H
#define INC_SF_Render_TessCurves_H

#include "Render_TessDefs.h"

namespace Scaleform { namespace Render {

void TessellateQuadCurve(TessBase* con, const ToleranceParams& param, 
                         CoordType x2, CoordType y2,
                         CoordType x3, CoordType y3);

void TessellateCubicCurve(TessBase* con, const ToleranceParams& param, 
                          CoordType x2, CoordType y2,
                          CoordType x3, CoordType y3,
                          CoordType x4, CoordType y4);

}} // Scaleform::Render

#endif
