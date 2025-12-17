/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_FilterDesc.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_FilterDesc_H
#define INC_SF_GFX_FilterDesc_H

#include "Kernel/SF_Types.h"

#include "GFx/GFx_PlayerStats.h"
#include "GFx/GFx_StreamContext.h"
#include "Render/Render_Filters.h"
//#include "Render/Text/Text_FilterDesc.h"

namespace Scaleform { namespace GFx {

class Stream;

// Helper function used to load filters.
template <class T> 
unsigned LoadFilters(T* pin, Render::FilterSet* filters);

}} // Scaleform::GFx

#endif
