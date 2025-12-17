/**************************************************************************

Filename    :   Render_RectPacker.cpp
Content     :   
Created     :   2007
Authors     :   Maxim Shemanarev

Notes       :   Specialized simple containers and functions

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Render_RectPacker.h"
#include "Kernel/SF_Alg.h"

#ifdef GDEBUGDRAW
#include "AggDraw.h"
extern AggDraw* DrawPtr;
#endif

namespace Scaleform { namespace Render {

//----------------------------------------------------------------------------
RectPacker::RectPacker(): 
    Width(1024), 
    Height(1024) {}

//----------------------------------------------------------------------------
UPInt RectPacker::GetNumBytes() const
{
    return  SrcRects.GetNumBytes() +
            PackedRects.GetNumBytes() +
            Packs.GetNumBytes() +
            PackTree.GetNumBytes() +
            Failed.GetNumBytes();
}

//----------------------------------------------------------------------------
void RectPacker::Pack()
{
    PackedRects.Clear();
    Packs.Clear();
    PackTree.Clear();
    if (SrcRects.GetSize() == 0) return;
    Alg::QuickSort(SrcRects, cmpRectsY);

    MinWidth  = SrcRects[SrcRects.GetSize() - 1].x;
    MinHeight = SrcRects[SrcRects.GetSize() - 1].y;
    for (UPInt i = 0; i < SrcRects.GetSize(); ++i)
    {
        if (SrcRects[i].x < MinWidth)
            MinWidth = SrcRects[i].x;
    }

    NumPacked = 0;

    do
    {
        unsigned prevPacked = NumPacked;
        PackTree.Clear();
        NodeType rootNode;
        rootNode.x      = 0;
        rootNode.y      = 0;
        rootNode.Width  = Width; 
        rootNode.Height = Height; 
        rootNode.Id     = ~0U;
        rootNode.Node1  = ~0U;
        rootNode.Node2  = ~0U;
        PackTree.PushBack(rootNode);
        packRects(0, 0);
        if(NumPacked > prevPacked)
        {
            PackType pack;
            pack.StartRect = (unsigned)PackedRects.GetSize();
            emitPacked();
            pack.NumRects  = (unsigned)PackedRects.GetSize() - pack.StartRect;
            Packs.PushBack(pack);
        }
    }
    while(NumPacked < SrcRects.GetSize());
}

//----------------------------------------------------------------------------
void RectPacker::packRects(unsigned nodeIdx, unsigned start)
{
    UPInt i;
    const NodeType& node = PackTree[nodeIdx];
    if(node.Width >= MinWidth && node.Height >= MinHeight)
    {
        for(i = start; i < SrcRects.GetSize(); ++i)
        {
            RectType& rect = SrcRects[i];
            if((rect.Id & Packed) == 0)
            {
                if(rect.x <= node.Width && rect.y <= node.Height)
                {
                    splitSpace(nodeIdx, rect);
                    rect.Id |= Packed;
                    ++NumPacked;
                    packRects(node.Node1, (unsigned)i);
                    packRects(node.Node2, (unsigned)i);
                    return;
                }
            }
        }
    }

}

//----------------------------------------------------------------------------
void RectPacker::splitSpace(unsigned nodeIdx, const RectType& rect)
{
    // Split the working area vertically with respect
    // to the rect that is being stored.
    //---------------------------
    NodeType& node  = PackTree[nodeIdx];
    NodeType  node1 = node;
    NodeType  node2 = node;

    node1.x      += rect.x;
    node1.Height  = rect.y;
    node1.Width  -= rect.x;
    node2.y      += rect.y;
    node2.Height -= rect.y;

    PackTree.PushBack(node1);
    PackTree.PushBack(node2);

    // This pack area now represents the rect that is just stored, 
    // so save the relevant info to it, and assign the children.
    node.Width    = rect.x;
    node.Height   = rect.y;
    node.Id       = rect.Id;
    node.Node1    = (unsigned)PackTree.GetSize() - 2;
    node.Node2    = (unsigned)PackTree.GetSize() - 1;
}

//----------------------------------------------------------------------------
void RectPacker::emitPacked()
{
    UPInt i;
    RectType   rect;
    for(i = 0; i < PackTree.GetSize(); ++i)
    {
        const NodeType& node = PackTree[i];
        if(node.Id != ~0U)
        {
            rect.x        = node.x;
            rect.y        = node.y;
            rect.Id       = node.Id;
            PackedRects.PushBack(rect);
        }
    }
}
}} // namespace Scaleform { namespace Render {
