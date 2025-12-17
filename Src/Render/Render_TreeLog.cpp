/**************************************************************************

Filename    :   Render_TreeLog.cpp
Content     :   Rendering log used to esitmate sorting efficiency
Created     :   
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Render_TreeLog.h"
#include "Render/Render_Twips.h"
#include "Kernel/SF_MsgFormat.h"

#ifdef GFX_RTREELOG_ENABLED

#include <stdio.h>

namespace Scaleform { 
namespace Render {

static FILE* GRenderLog_pLogFile = 0;
unsigned     GRenderLog_ObjectIndent = 0;

void    RenderTreeLog::StartLog(const char* pfilename)
{
    #pragma warning(suppress : 4996)
    GRenderLog_pLogFile = fopen(pfilename, "w+");
    GRenderLog_ObjectIndent = 0;

}
void    RenderTreeLog::EndLog()
{
    if (GRenderLog_pLogFile)
    {
        fclose(GRenderLog_pLogFile);
        GRenderLog_pLogFile = 0;
    }
}

bool    RenderTreeLog::IsLogging()
{
    return GRenderLog_pLogFile != 0;
}

void    RenderTreeLog::IndentLine()
{
    if (!GRenderLog_pLogFile)
        return;
    for (unsigned i = 0; i< GRenderLog_ObjectIndent; i++)
        fprintf(GRenderLog_pLogFile, "  ");
}

void    RenderTreeLog::StartObject(const char* pname, const RectF& bounds, const Matrix2F& m)
{
    if (!GRenderLog_pLogFile)
        return;

    char buffer[512];

    IndentLine();
    fprintf(GRenderLog_pLogFile, "[Object %s]\n", pname);
    GRenderLog_ObjectIndent++;
    IndentLine();
    fprintf(GRenderLog_pLogFile, "Bounds: (%g, %g, %g, %g)\n",
        SF_TWIPS_TO_PIXELS(bounds.x1), SF_TWIPS_TO_PIXELS(bounds.y1),
        SF_TWIPS_TO_PIXELS(bounds.x2), SF_TWIPS_TO_PIXELS(bounds.y2));
    IndentLine();
    fprintf(GRenderLog_pLogFile, "Matrix: ");
    
    //IndentLine();
    Format(StringDataPtr(buffer, 512),
        "( {0:4.4}, {1:4.4}, {2:4.4},\n",
        m.Sx(), m.Shx(), SF_TWIPS_TO_PIXELS(m.Tx()));
    fprintf(GRenderLog_pLogFile, "%s", buffer);

    IndentLine();
    Format(StringDataPtr(buffer, 512),
        "          {0:4.4}, {1:4.4}, {2:4.4} )\n",
        m.Shy(), m.Sy(), SF_TWIPS_TO_PIXELS(m.Ty()));
    fprintf(GRenderLog_pLogFile, "%s", buffer);
    
    
}

void    RenderTreeLog::EndObject()
{
    if (!GRenderLog_pLogFile)
        return;

    GRenderLog_ObjectIndent--;
    IndentLine();
    fprintf(GRenderLog_pLogFile, "[End_Object]\n");
}

void    RenderTreeLog::LogRender(const char* pcmdName, unsigned key)
{
    if (!GRenderLog_pLogFile)
        return;
    IndentLine();
    fprintf(GRenderLog_pLogFile, "[%s Key='%d']\n", pcmdName, key);
    
}

}}

#endif
