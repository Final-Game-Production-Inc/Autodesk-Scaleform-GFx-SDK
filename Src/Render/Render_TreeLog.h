/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TreeLog.h
Content     :   Rendering log used to esitmate sorting efficiency
Created     :   
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_RenderTreeLog_H
#define INC_RenderTreeLog_H

#include "Render/Render_Types2D.h"
#include "Render/Render_Matrix2x4.h"

//#define GFX_RTREELOG_ENABLED

namespace Scaleform { 
namespace Render {

// This RenderTreeLog is used for debugging only, allowing object
// tree with bounds information to be dumped so as to allow sorter
// testing for Gfx 4.0 renderer

#ifdef GFX_RTREELOG_ENABLED

class RenderTreeLog
{
    static void    IndentLine();
public:
    static void    StartLog(const char* pfilename);
    static bool    IsLogging();
    static void    EndLog();

    static void    StartObject(const char* pname,
        const RectF& bounds, const Matrix2F& matrix);
    static void    EndObject();

    static void    LogRender(const char* pcmdName, unsigned key);
};

// Macros to allow easy compile-out of logging calls (and their arguments)
#define GFX_RTREELOG_STARTOBJECT(name, b, m)  \
    if (Render::RenderTreeLog::IsLogging()) Render::RenderTreeLog::StartObject(name, b, m)
#define GFX_RTREELOG_ENDOBJECT() Render::RenderTreeLog::EndObject()

#else

#define GFX_RTREELOG_STARTOBJECT(name, b, m)
#define GFX_RTREELOG_ENDOBJECT()

#endif

}}

#endif // INC_RenderTreeLog_H

