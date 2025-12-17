/**************************************************************************

Filename    :   Render_TessGen.h
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef SF_Render_TessGen_H
#define SF_Render_TessGen_H

#include "Render_Containers.h"
#include "Render_Tessellator.h"
#include "Render_Stroker.h"
#include "Render_StrokerAA.h"
#include "Render_Hairliner.h"
#include "Render_ToleranceParams.h"

namespace Scaleform { namespace Render {

//--------------------------------------------------------------------
struct MeshGenerator
{
    LinearHeap      Heap1;
    LinearHeap      Heap2;
    LinearHeap      Heap3;
    LinearHeap      Heap4;
    Tessellator     mTess;
    Stroker         mStroker;
    StrokeSorter    mStrokeSorter;
#ifdef SF_RENDER_ENABLE_HAIRLINER
    Hairliner       mHairliner;
#endif
    StrokerAA       mStrokerAA;

    MeshGenerator(MemoryHeap* h) :
        Heap1(h),
        Heap2(h),
        Heap3(h),
        Heap4(h),
        mTess(&Heap1, &Heap2)
        ,mStroker(&Heap3)
        ,mStrokeSorter(&Heap4)
#ifdef SF_RENDER_ENABLE_HAIRLINER
        ,mHairliner(&Heap3)
#endif
        ,mStrokerAA(&Heap3)
    {
    }

    void Clear()
    {
        mTess.Clear();
        mStroker.Clear();
        mStrokeSorter.Clear();
#ifdef SF_RENDER_ENABLE_HAIRLINER
        mHairliner.Clear();
#endif
        mStrokerAA.Clear();
        Heap1.ClearAndRelease();
        Heap2.ClearAndRelease();
        Heap3.ClearAndRelease();
        Heap4.ClearAndRelease();
    }
};


//--------------------------------------------------------------------
struct StrokeGenerator
{
    LinearHeap      Heap1;
    LinearHeap      Heap2;
    Stroker         mStroker;
    StrokeSorter    mStrokeSorter;
    VertexPath      mPath;

    StrokeGenerator(MemoryHeap* h) : Heap1(h), Heap2(h)
        ,mStroker(&Heap1), mStrokeSorter(&Heap2)
        ,mPath(&Heap1)
    {}

    void Clear()
    {
        mStroker.Clear();
        mStrokeSorter.Clear();
        Heap1.ClearAndRelease();
        Heap2.ClearAndRelease();
    }
};


}} // Scaleform::Render

#endif
