/**************************************************************************

Filename    :   Render_VertexPath.cpp
Content     :   
Created     :
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_TessDefs.h"

namespace Scaleform { namespace Render {

void VertexPath::Clear() 
{ 
    Vertices.ClearAndRelease(); 
    Paths.ClearAndRelease();
    LastVertex = 0;
}

void VertexPath::AddVertex(CoordType x, CoordType y) 
{ 
    VertexBasic v = {x, y}; 
    Vertices.PushBack(v); 
}

void VertexPath::ClosePath()
{
    if (Vertices.GetSize() - LastVertex > 2 && 
        (Vertices[LastVertex].x != Vertices.Back().x || 
         Vertices[LastVertex].y != Vertices.Back().y))
    {
        Vertices.PushBack(Vertices[LastVertex]);
    }
}

void VertexPath::FinalizePath(unsigned, unsigned, bool, bool)
{
    if (Vertices.GetSize() - LastVertex < 3) 
    {
        Vertices.CutAt(LastVertex);
    }
    else
    {
        PathBasic p = { LastVertex, (unsigned)Vertices.GetSize() - LastVertex };
        Paths.PushBack(p);
        LastVertex = (unsigned)Vertices.GetSize();
    }
}


}} // Scaleform::Render
