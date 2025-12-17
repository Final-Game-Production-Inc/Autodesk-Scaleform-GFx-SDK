/**************************************************************************

Filename    :   GFx_String.cpp
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

#include "GFx/GFx_String.h"

#include "Kernel/SF_UTF8Util.h"
#include "Kernel/SF_Debug.h"
//#include "Kernel/SF_Functions.h"

#include <stdlib.h>
#include <ctype.h>

#ifdef SF_OS_QNX
# include <strings.h>
#endif

namespace Scaleform { namespace GFx {

}} // Scaleform::GFx
