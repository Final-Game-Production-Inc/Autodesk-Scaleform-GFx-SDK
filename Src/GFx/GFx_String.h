/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_String.h
Content     :   String UTF8 string implementation with copy-on
                write semantics (thread-safe for assignment but not
                modification).
Created     :   April 27, 2007
Authors     :   Ankur Mohan, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFx_String_H
#define INC_SF_GFx_String_H

#include "Kernel/SF_String.h"

// GFxString... classes have been renamed to String.. classes and moved to GKernel
// typedefs are here only for backward compatibility

namespace Scaleform { namespace GFx {

// typedef String       String;
// typedef SF::StringLH     StringLH;
// typedef SF::StringDH     StringDH;
// typedef SF::StringBuffer StringBuffer;
// typedef StringDataPtr StringDataPtr;



}} // Scaleform::GFx

#endif // INC_SF_GFx_String_H
