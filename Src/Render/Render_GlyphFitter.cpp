/**************************************************************************

Filename    :   Render_GlyphFitter.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_GlyphFitter.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
GlyphFitter::GlyphFitter(MemoryHeap* heap, int nominalFontHeight) :
    NominalFontHeight(nominalFontHeight),
    LHeap(heap),
    Contours(&LHeap),
    Vertices(&LHeap),
    Events(&LHeap),
    LerpPairs(&LHeap),
    LerpRampX(&LHeap),
    LerpRampY(&LHeap),
    StartX(0),
    StartY(0),
    LastXf(0),
    LastYf(0)
{}


//------------------------------------------------------------------------
void GlyphFitter::Clear() 
{ 
    Contours.ClearAndRelease(); 
    Vertices.ClearAndRelease(); 
    Events.ClearAndRelease();
    LerpPairs.ClearAndRelease();
    LerpRampX.ClearAndRelease();
    LerpRampY.ClearAndRelease();
    LHeap.ClearAndRelease();
}

//------------------------------------------------------------------------
void GlyphFitter::MoveTo(float x, float y)
{
    ContourType c;
    VertexType  v;
    c.NumVertices = 1;
    c.StartVertex = (unsigned)Vertices.GetSize();
    v.x = SInt16(x);
    v.y = SInt16(y);
    Contours.PushBack(c);
    Vertices.PushBack(v);
    StartX = x;
    StartY = y;
    LastXf = x;
    LastYf = y;
}

//------------------------------------------------------------------------
void GlyphFitter::LineTo(float x, float y)
{
    const VertexType& pv = Vertices.Back();
    VertexType  tv;
    tv.x = SInt16(x);
    tv.y = SInt16(y);
    if(tv.x != pv.x || tv.y != pv.y)
    {
        Vertices.PushBack(tv);
        ++Contours.Back().NumVertices;
    }
    LastXf = x;
    LastYf = y;
}

//------------------------------------------------------------------------
void GlyphFitter::ClosePath()
{
    if (LastXf != StartX || LastYf != StartY)
        LineTo(StartX, StartY);
}


//------------------------------------------------------------------------
void GlyphFitter::removeDuplicateClosures()
{
    unsigned i;
    for(i = 0; i < Contours.GetSize(); ++i)
    {
        ContourType& c = Contours[i];
        if(c.NumVertices > 2)
        {
            const VertexType& v1 = Vertices[c.StartVertex];
            const VertexType& v2 = Vertices[c.StartVertex + c.NumVertices - 1];
            if(v1.x == v2.x && v1.y == v2.y)
            {
                --c.NumVertices;
            }
        }
    }
}

//------------------------------------------------------------------------
void GlyphFitter::computeBounds()
{
    UPInt i, j;

    SInt16 minX =  32767;
    SInt16 minY =  32767;
    SInt16 maxX = -32767;
    SInt16 maxY = -32767;

    MinX =  32767;
    MinY =  32767;
    MaxX = -32767;
    MaxY = -32767;

    for(i = 0; i < Contours.GetSize(); ++i)
    {
        const ContourType& c = Contours[i];
        if(c.NumVertices > 2)
        {
            VertexType v1 = Vertices[c.StartVertex + c.NumVertices - 1];
            int sum = 0;
            for(j = 0; j < c.NumVertices; ++j)
            {
                const VertexType& v2 = Vertices[c.StartVertex + j];
                if(v2.x < minX) minX = v2.x;
                if(v2.y < minY) minY = v2.y;
                if(v2.x > maxX) maxX = v2.x;
                if(v2.y > maxY) maxY = v2.y;
                sum += v1.x * v2.y - v1.y * v2.x;
                v1 = v2;
            }

            if(minX < MinX || minY < MinY || maxX > MaxX || maxY > MaxY)
            {
                MinX = minX;
                MinY = minY;
                MaxX = maxX;
                MaxY = maxY;
                Direction = (sum > 0) ? DirCCW : DirCW;
            }
        }
    }
}

//------------------------------------------------------------------------
void GlyphFitter::detectEvents(FitDir dir)
{
    unsigned i, j;
    int idx;
    int minCoord = (dir == FitX) ? MinX : MinY;

    Events.Resize((dir == FitX) ? MaxX - MinX + 1 : MaxY - MinY + 1);
    Events.Zero();

    for(i = 0; i < (unsigned)Contours.GetSize(); ++i)
    {
        const ContourType& c = Contours[i];
        if(c.NumVertices > 2)
        {
            Events[0]                 = DirCW | DirCCW;
          //Events[Events.GetSize() - 1] = DirCW | DirCCW;

            for(j = 0; j < c.NumVertices; ++j)
            {
                unsigned i1 = j;
                unsigned i2 = (j + 1) % c.NumVertices;
                unsigned i3 = (j + 2) % c.NumVertices;
                VertexType v1 = Vertices[c.StartVertex + i1]; 
                VertexType v2 = Vertices[c.StartVertex + i2]; 
                VertexType v3 = Vertices[c.StartVertex + i3];

                if(dir == FitX)
                {
                    SInt16 t;
                    t = v1.x; v1.x = -v1.y; v1.y = t;
                    t = v2.x; v1.x = -v2.y; v2.y = t;
                    t = v3.x; v1.x = -v3.y; v3.y = t;
                }

                bool done = false;
                if((v1.y >= v2.y && v3.y >= v2.y) ||
                   (v1.y <= v2.y && v3.y <= v2.y))
                {
                    // Local min or max
                    //------------------
                    idx = v2.y - minCoord;
                    if(v1.x <= v2.x && v2.x <= v3.x)
                    {
                        Events[idx] |= (Direction == DirCW)? DirCCW: DirCW;
                        done = true;
                    }
                    if(v1.x >= v2.x && v2.x >= v3.x)
                    {
                        Events[idx] |= (Direction == DirCW)? DirCW: DirCCW;
                        done = true;
                    }
                }

                if(!done)
                {
                    if(v1.y == v2.y)
                    {
                        // Flat shoulder
                        //-------------------
                        idx = v2.y - minCoord;
                        if(v1.x < v2.x)
                            Events[idx] |= (Direction == DirCW)? DirCCW: DirCW;

                        if(v1.x > v2.x)
                            Events[idx] |= (Direction == DirCW)? DirCW: DirCCW;
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------
void GlyphFitter::computeLerpRamp(FitDir dir, int unitsPerPixel, int middle,
                                  int lowerCaseTop, int upperCaseTop)
{
    LerpPairs.Clear();
    VertexType lerpPair;

    lerpPair.x = SInt16(-SnappedHeight * 4);
    lerpPair.y = SInt16(-SnappedHeight * 4);
    LerpPairs.PushBack(lerpPair);

    int prev  = -32767;
    int top1  = lowerCaseTop;
    int top2  = upperCaseTop;
    int top1s = snapToPixel(top1 + unitsPerPixel, unitsPerPixel);
    int top2s = snapToPixel(top2 + unitsPerPixel, unitsPerPixel);
    int y, snapped;
    int minCoord = (dir == FitX) ? MinX : MinY;
    int minDist = unitsPerPixel + 1;//unitsPerPixel / 2;
    UByte event;

    unsigned i;

    for(i = 0; i < Events.GetSize(); ++i)
    {
        y = int(i) + minCoord;
        event = Events[i];

        if(y <= middle || dir == FitX)
        {
            // Snap to bottom
            //----------------------
            if(event & DirCW)
            {
                if(y > prev + minDist)
                {
                    snapped = snapToPixel(y + unitsPerPixel / 2 + 1, unitsPerPixel);
                    if(LerpPairs.Back().y != snapped)
                    {
                        lerpPair.x = (SInt16)y;
                        lerpPair.y = (SInt16)snapped;
                        LerpPairs.PushBack(lerpPair);
                    }
                    prev = y;
                }
            }
        }
        else
        {
            // Snap to top
            //----------------------
            if(event & DirCCW)
            {
                bool done = false;
                if(top2)
                {
                    if(y >= top2 && y < top2 + minDist)
                    {
                        if(y <= prev + minDist ||
                           LerpPairs.Back().y + unitsPerPixel >= top2s)
                        {
                            LerpPairs.PopBack();
                        }
                        lerpPair.x = (SInt16)y;
                        lerpPair.y = (SInt16)top2s;
                        LerpPairs.PushBack(lerpPair);
                        prev = y;
                        done = true;
                    }
                    else
                    if(y >= top1 && y < top1 + minDist)
                    {
                        if(y <= prev + minDist ||
                           LerpPairs.Back().y + unitsPerPixel >= top1s)
                        {
                            LerpPairs.PopBack();
                        }
                        lerpPair.x = (SInt16)y;
                        lerpPair.y = (SInt16)top1s;
                        LerpPairs.PushBack(lerpPair);
                        prev = y;
                        done = true;
                    }
                }
                if(!done)
                {
                    snapped = snapToPixel(y + unitsPerPixel, unitsPerPixel);
                    if(y <= prev + minDist ||
                       LerpPairs.Back().y + unitsPerPixel >= snapped)
                    {
                        LerpPairs.PopBack();
                    }
                    lerpPair.x = (SInt16)y;
                    lerpPair.y = (SInt16)snapped;
                    LerpPairs.PushBack(lerpPair);
                    prev = y;
                }
            }
        }
    }

    lerpPair.x = SInt16(SnappedHeight * 4);
    lerpPair.y = SInt16(SnappedHeight * 4);
    LerpPairs.PushBack(lerpPair);

    ArrayUnsafe<SInt16>& ramp = (dir == FitX) ? LerpRampX : LerpRampY;
    ramp.Resize(Events.GetSize());

    VertexType v1 = LerpPairs[0];
    VertexType v2 = LerpPairs[1];
    unsigned topIdx = 2;
    for(i = 0; i < Events.GetSize(); ++i)
    {
        y = int(i) + minCoord;
        if(y >= v2.x && topIdx < LerpPairs.GetSize())
        {
            v1 = v2;
            v2 = LerpPairs[topIdx++];
        }
        ramp[i] = SInt16(v1.y + (y - v1.x) * (v2.y - v1.y) / (v2.x - v1.x) - minCoord);
    }
}

//------------------------------------------------------------------------
void GlyphFitter::FitGlyph(int heightInPixels, int widthInPixels, 
                           int lowerCaseTop,   int upperCaseTop)
{
    UnitsPerPixelX = widthInPixels  ? NominalFontHeight / widthInPixels  : 1;
    UnitsPerPixelY = heightInPixels ? NominalFontHeight / heightInPixels : 1;
    SnappedHeight  = NominalFontHeight / UnitsPerPixelY * UnitsPerPixelY;

    if(heightInPixels || widthInPixels)
    {
        removeDuplicateClosures();
        computeBounds();
        if(heightInPixels && MaxY > MinY) 
        {
            detectEvents(FitY);
            computeLerpRamp(FitY, UnitsPerPixelY, 
                            MinY + (MaxY - MinY)/3, 
                            lowerCaseTop, upperCaseTop);
        }
        if(widthInPixels && MaxY > MinY) 
        {
            detectEvents(FitX);
            computeLerpRamp(FitX, UnitsPerPixelX, 
                            MinX + (MaxX - MinX)/3, 0, 0);
        }
    }
}

}} // Scaleform::Render
