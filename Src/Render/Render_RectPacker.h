/**************************************************************************

Filename    :   Render_RectPacker.h
Content     :   
Created     :   2007
Authors     :   Maxim Shemanarev

Notes       :   Specialized simple containers and functions

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_RectPacker_H
#define INC_SF_Render_RectPacker_H

#include "Kernel/SF_ArrayPaged.h"

namespace Scaleform { namespace Render {

//----------------------------------------------------------------------------
class RectPacker
{
    enum { SID = Stat_Default_Mem };
    enum { Packed = 0x80000000U };

public:
    struct RectType
    {
        unsigned x, y, Id;
    }; 

    struct PackType
    {
        unsigned StartRect;
        unsigned NumRects;
    }; 

    RectPacker();

    UPInt GetNumBytes() const;

    //============= Set parameters
    void     SetWidth    (unsigned w) { Width  = w;}
    void     SetHeight   (unsigned h) { Height = h;}
    unsigned GetWidth()   const { return Width;  }
    unsigned GetHeight()  const { return Height; }

    //============= Prepare data and pack
    void Clear()
    {
        SrcRects.Clear();
        PackedRects.Clear();
        Packs.Clear();
        PackTree.Clear();
        Failed.Clear();
    }

    void ClearAndRelease()
    {
        SrcRects.ClearAndRelease();
        PackedRects.ClearAndRelease();
        Packs.ClearAndRelease();
        PackTree.ClearAndRelease();
        Failed.ClearAndRelease();
    }

    void AddRect(unsigned w, unsigned h, unsigned id)
    {
        if(w && h && w <= Width && h <= Height)
        {
            RectType r;
            r.x  = w;
            r.y  = h;
            r.Id = id;
            SrcRects.PushBack(r);
        }
        else
        {
            Failed.PushBack(id);
        }
    }

    void Pack();

    //============= Access the result
    UPInt           GetNumPacks()          const { return Packs.GetSize(); }
    const PackType& GetPack(UPInt packIdx) const { return Packs[packIdx]; }
    const RectType& GetRect(const PackType& pack, unsigned rectIdx) const
    {
        return PackedRects[pack.StartRect + rectIdx];
    }
    UPInt           GetNumFailed()       const { return Failed.GetSize(); }
    unsigned        GetFailed(UPInt idx) const { return Failed[idx]; }

private:
    struct NodeType
    {
        unsigned x, y, Width, Height, Id, Node1, Node2;
    };

    static bool cmpRectsY(const RectType& a, const RectType& b)
    {
        if(b.y != a.y) return b.y < a.y;
        return b.x < a.x;
    }
    void packRects(unsigned nodeIdx, unsigned start);
    void splitSpace(unsigned nodeIdx, const RectType& rect);
    void emitPacked();

    unsigned                                Width;
    unsigned                                Height;
    unsigned                                NumPacked;
    unsigned                                MinWidth;
    unsigned                                MinHeight;
    ArrayPagedLH_POD<RectType, 8, 64, SID>  SrcRects;
    ArrayPagedLH_POD<RectType, 8, 64, SID>  PackedRects;
    ArrayPagedLH_POD<PackType, 4, 16, SID>  Packs;
    ArrayPagedLH_POD<NodeType, 8, 64, SID>  PackTree;
    ArrayPagedLH_POD<unsigned, 6, 64, SID>  Failed;
};

}} // namespace Scaleform { namespace Render {

#endif
