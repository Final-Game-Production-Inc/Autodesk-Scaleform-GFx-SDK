/**************************************************************************

Filename    :   Render_Filters.cpp
Content     :   Filter and FilterSet implementations.
Created     :   May 18, 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Filters.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Render_Primitive.h"

namespace Scaleform { namespace Render {

//--------------------------------------------------------------------
bool BlurFilterImpl::CanCacheAcrossTransform(bool deltaTrans, bool deltaRot, bool deltaScale) const
{
    SF_UNUSED2(deltaTrans, deltaRot);
    return !deltaScale;
}

//--------------------------------------------------------------------
// ***** BlurFilter
Filter* BlurFilter::Clone(MemoryHeap *heap) const
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);
    return SF_HEAP_NEW(heap) BlurFilter(Params);
}

bool BlurFilter::IsContributing() const
{
    return (Params.BlurX > 20 || Params.BlurY > 20) && Params.Passes > 0;
}

//--------------------------------------------------------------------
// ***** ShadowFilter
Filter* ShadowFilter::Clone(MemoryHeap *heap) const
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);
    return SF_HEAP_NEW(heap) ShadowFilter(Params, Angle, TwipsToPixels(Distance));
}

bool ShadowFilter::IsContributing() const
{
    return Params.Colors[0].GetAlpha() > 0 && Params.Passes > 0;
}

bool ShadowFilter::CanCacheAcrossTransform(bool deltaTrans, bool deltaRot, bool deltaScale) const
{
    SF_UNUSED(deltaTrans);
    return !deltaRot && !deltaScale;
}

//--------------------------------------------------------------------
// ***** GlowFilter
Filter* GlowFilter::Clone(MemoryHeap *heap) const
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);
    return SF_HEAP_NEW(heap) GlowFilter(Params);
}

bool GlowFilter::IsContributing() const
{
    return Params.Colors[0].GetAlpha() > 0 && Params.Passes > 0;
}

//--------------------------------------------------------------------
// ***** BevelFilter
Filter* BevelFilter::Clone(MemoryHeap *heap) const
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);
    return SF_HEAP_NEW(heap) BevelFilter(Params, Angle, TwipsToPixels(Distance));
}

bool BevelFilter::IsContributing() const
{
    return (Params.Colors[0].GetAlpha() > 0 || 
           Params.Colors[1].GetAlpha() > 0) && Params.Passes > 0;
}

bool BevelFilter::CanCacheAcrossTransform(bool deltaTrans, bool deltaRot, bool deltaScale) const
{
    SF_UNUSED(deltaTrans);
    return !deltaRot && !deltaScale;
}


//--------------------------------------------------------------------
// ***** GradientFilter
GradientFilter::GradientFilter(FilterType type, GradientData* gradient, float dist, float angle, float blurx, float blury, unsigned passes) :
    BlurFilterImpl(type)
{
    SF_DEBUG_ASSERT1(type == Filter_GradientBevel || type == Filter_GradientGlow, 
        "Expected filter type to be Filter_GradientBevel or Filter_GradientGlow, but it was (%d)", type);
    Params.BlurX  = PixelsToTwips(blurx);
    Params.BlurY  = PixelsToTwips(blury);
    Params.Passes = passes;
    SetAngleDistance(angle, PixelsToTwips(dist));
    Params.Strength = 1.0f;
    Params.Gradient = gradient;
}

GradientFilter::GradientFilter(FilterType type, const BlurFilterParams& params, float angle, float dist) :
    BlurFilterImpl(type, params)
{
    SetAngleDistance(angle, PixelsToTwips(dist));
}

Filter* GradientFilter::Clone(MemoryHeap *heap) const
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);
    return SF_HEAP_NEW(heap) GradientFilter(Type, Params, Angle, TwipsToPixels(Distance));
}

bool GradientFilter::IsContributing() const
{
    return Params.Passes > 0 && Params.Gradient != 0;
}

bool GradientFilter::CanCacheAcrossTransform(bool deltaTrans, bool deltaRot, bool deltaScale) const
{
    SF_UNUSED(deltaTrans);
    return !deltaRot && !deltaScale;
}

void GradientFilter::GenerateGradientImage(PrimitiveFillManager& mgr) const
{
    GradientImage = *mgr.createGradientImage(Params.Gradient, 0.0f);
}

//--------------------------------------------------------------------
// ***** ColorMatrixFilter

static float ColorMatrix_Identity[20] =
{
    1.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 0.0f
};

ColorMatrixFilter::ColorMatrixFilter() : Filter(Filter_ColorMatrix)
{
    memcpy(MatrixData, ColorMatrix_Identity, sizeof(MatrixData));
}
ColorMatrixFilter::~ColorMatrixFilter()
{
}

bool ColorMatrixFilter::IsContributing() const
{
    return memcmp( MatrixData, ColorMatrix_Identity, sizeof(ColorMatrix_Identity)) != 0;
}

Filter* ColorMatrixFilter::Clone(MemoryHeap* heap) const
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);

    ColorMatrixFilter* newFilter = SF_HEAP_NEW(heap) ColorMatrixFilter();
    if (newFilter)
    {
        memcpy(newFilter->MatrixData, MatrixData, sizeof(MatrixData));
    }
    return newFilter;
}

//--------------------------------------------------------------------
// ***** CacheAsBitmapFilter
Filter* CacheAsBitmapFilter::Clone( MemoryHeap * pheap) const
{
    if (!pheap)
        pheap = Memory::GetHeapByAddress(this);
    return SF_HEAP_NEW(pheap) CacheAsBitmapFilter();
}


//--------------------------------------------------------------------
// ***** DisplacementMapFilter
DisplacementMapFilter::DisplacementMapFilter(Render::Image* mapBitmap, PointF mapPoint, 
                                             DrawableImage::ChannelBits compx, 
                                             DrawableImage::ChannelBits compy, 
                                             DisplacementMode mode,
                                             float scaleX, float scaleY,
                                             Color color) :
    Render::Filter(Filter_DisplacementMap),
    DisplacementMap(mapBitmap),
    MapPoint(mapPoint),
    ComponentX(compx), ComponentY(compy),
    Mode(mode), ScaleX(scaleX), ScaleY(scaleY),
    ColorValue(color)
{

}

bool DisplacementMapFilter::IsContributing() const
{
    return DisplacementMap != 0;
}

Filter* DisplacementMapFilter::Clone(MemoryHeap *heap) const
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);
    return SF_HEAP_NEW(heap) DisplacementMapFilter(DisplacementMap, MapPoint, ComponentX, ComponentY, Mode, ScaleX, ScaleY, ColorValue);
}

//--------------------------------------------------------------------
// ***** FilterSet

FilterSet::FilterSet(Filter* filter)
    : Frozen(false), CacheAsBitmap(false), pCacheAsBitmapFilter(0)
{
    if (filter)
        AddFilter(filter);
}
FilterSet::~FilterSet()
{
}

void FilterSet::Freeze()
{
    if (!Frozen)
    {
        for (UPInt i=0; i<Filters.GetSize(); i++)
            Filters[i]->Freeze();
        Frozen = true;
    }
}


unsigned FilterSet::GetFilterCount() const
{
    return (unsigned)Filters.GetSize();
}


const Filter* FilterSet::GetFilter( UPInt index ) const
{
    return Filters[index].GetPtr();
}


void FilterSet::SetFilter( UPInt index, Filter* filter )
{
    SF_DEBUG_ASSERT(!IsFrozen(), "Cannot modify a filter set that is frozen."); 
    Filters[index] = filter;
}


void FilterSet::AddFilter( Filter* filter )
{
    SF_DEBUG_ASSERT(!IsFrozen(), "Cannot modify a filter set that is frozen."); 
    if (Filters.GetSize() == 1 && Filters[0]->GetFilterType() == Filter_CacheAsBitmap)
        Filters[0] = filter;
    else
        Filters.PushBack(filter);
}


void FilterSet::InsertFilterAt( UPInt index, Filter* filter )
{
    SF_DEBUG_ASSERT(!IsFrozen(), "Cannot modify a filter set that is frozen."); 
    if (Filters.GetSize() == 1 && Filters[0]->GetFilterType() == Filter_CacheAsBitmap)
        Filters[0] = filter;
    else
        Filters.InsertAt(index, filter);
}


void FilterSet::RemoveFilterAt( UPInt index )
{
    SF_DEBUG_ASSERT(!IsFrozen(), "Cannot modify a filter set that is frozen."); 
    Filters.RemoveAt(index);
    if (Filters.GetSize() == 0 && CacheAsBitmap)
    {
        if (!pCacheAsBitmapFilter)
            pCacheAsBitmapFilter = *SF_NEW CacheAsBitmapFilter();
        Filters.PushBack(pCacheAsBitmapFilter);
    }
}

void FilterSet::RemoveFilter(Filter* filter)
{
    SF_DEBUG_ASSERT(!IsFrozen(), "Cannot modify a filter set that is frozen."); 
    for (UPInt i=0; i<Filters.GetSize(); i++)
    {
        if (Filters[i] == filter)
        {
            Filters.RemoveAt(i);
            i--;
        }
    }

    if (Filters.GetSize() == 0 && CacheAsBitmap)
    {
        if (!pCacheAsBitmapFilter)
            pCacheAsBitmapFilter = *SF_NEW CacheAsBitmapFilter();
        Filters.PushBack(pCacheAsBitmapFilter);
    }
}


bool FilterSet::GetCacheAsBitmap() const
{
    return CacheAsBitmap;
}

void FilterSet::SetCacheAsBitmap(bool enable)
{
    // If we have no filters, we need to add in a 'bogus' cacheAsBitmap filter to the array.
    CacheAsBitmap = enable;
    if (enable && GetFilterCount() == 0)
    {
        if (pCacheAsBitmapFilter == 0)
            pCacheAsBitmapFilter = *SF_NEW CacheAsBitmapFilter();
        AddFilter(pCacheAsBitmapFilter);
    }
    else if (!enable && GetFilterCount() == 1 && Filters[0]->GetFilterType() == Filter_CacheAsBitmap)
            Filters.RemoveAt(0);
}

FilterSet* FilterSet::Clone(bool deepCopy, MemoryHeap *heap) const
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);

    FilterSet* newSet = SF_HEAP_NEW(heap) FilterSet;
    if (!newSet)
        return 0;

    newSet->CacheAsBitmap = CacheAsBitmap;

    for (UPInt i=0; i< Filters.GetSize(); i++)
    {
        if (deepCopy)
        {
            Ptr<Filter> newFilter = *Filters[i]->Clone(heap);
            newSet->AddFilter(newFilter);
        }
        else
        {
            newSet->AddFilter(Filters[i]);
        }
    }

    return newSet;
}

bool FilterSet::IsContributing() const
{
    // If a FilterSet has cacheAsBitmap set on it, report that it is always contributing,
    // regardless of whether its filters actually do anything or not (so it will always be
    // rendered to a texture).
    if (CacheAsBitmap)
        return true;

    unsigned f;
    for (f = 0; f < Filters.GetSize(); ++f )
    {
        const Filter* filter = Filters[f];
        if ( filter && filter->IsContributing() )
            return true;
    }
    return false;
}

bool FilterSet::CanCacheAcrossTransform(bool deltaTrans, bool deltaRot, bool deltaScale) const
{
    unsigned f;
    for (f = 0; f < Filters.GetSize(); ++f )
    {
        const Filter* filter = Filters[f];
        if ( filter && !filter->CanCacheAcrossTransform(deltaTrans, deltaRot, deltaScale) )
            return false;
    }
    return true;
}

}} // Scaleform::Render
