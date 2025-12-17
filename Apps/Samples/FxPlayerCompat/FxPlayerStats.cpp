/**************************************************************************

Filename    :   FxPlayerStats.cpp
Content     :   Statistics reporting for FxPlayer 
Created     :   February 23, 2009
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerStats.h"
#include "GFx_Kernel.h"
#include "Kernel/SF_MsgFormat.h"
///////////////////////////////////////////////////////////////////////////////
FxPerformanceRootStatItem::FxPerformanceRootStatItem(
    FxGlobalStatTracker* pstatTracker,
    Movie* pmovie
    ) 
: BaseClass(NULL) // This is a root item.
, pStatTracker(pstatTracker)
, pMovieView(pmovie)
, Sums(Memory::GetHeapByAddress(GetThis()))
, Peaks(Memory::GetHeapByAddress(GetThis()))
, Frames(0)
, pHeapInfoItem(NULL)
{
    SetExpanded(true);
    pHeapInfoItem = 
        *SF_HEAP_AUTO_NEW(this) FxMessageStatItem(this, "", "|>  Average|>  Peak");
    AddChild(pHeapInfoItem);
}

void FxPerformanceRootStatItem::Update( const FxStatUpdate& update)
{
    switch (update.GetType())
    {
    case FxStatUpdate::Update_Init:
        // Create children.

        // "Advance"
        {
            // Advance root ...
            Children.PushBack(*SF_HEAP_AUTO_NEW(this) FxPerformanceElementStatItem(this, *this, StatMV_Advance_Tks));
            const ChildrenType::ValueType adv = Children.Back();
            AddChild(adv);

            // Children ...
            Children.PushBack(*SF_HEAP_AUTO_NEW(this) FxPerformanceElementStatItem(adv.GetPtr(), *this, StatMV_Action_Tks));
            adv->AddChild(Children.Back());

            Children.PushBack(*SF_HEAP_AUTO_NEW(this) FxPerformanceElementStatItem(adv.GetPtr(), *this, StatMV_Timeline_Tks));
            adv->AddChild(Children.Back());

            Children.PushBack(*SF_HEAP_AUTO_NEW(this) FxPerformanceElementStatItem(adv.GetPtr(), *this, StatMV_Input_Tks));
            adv->AddChild(Children.Back());
        }

        // "Display"
        Children.PushBack(*SF_HEAP_AUTO_NEW(this) FxPerformanceElementStatItem(this, *this, StatMV_Display_Tks));
        AddChild(Children.Back());

        OnTickStat();
        break;
    case FxStatUpdate::Update_Tick:
        // Update all children.
        for (unsigned i = 0; i < Children.GetSize(); ++i)
        {
            Children[i]->Update(update);
        }
        OnTickStat();
        break;
    case FxStatUpdate::Update_Frame:
        OnFrameStat();
        break;
    }
}

void FxPerformanceRootStatItem::OnFrameStat()
{
    StatBag tmpStatBag(Memory::GetHeapByAddress(this));

    pMovieView->GetStats(&tmpStatBag);
    Sums += tmpStatBag;
    Peaks.SetMax(tmpStatBag);
    ++Frames;
}

void FxPerformanceRootStatItem::OnTickStat()
{
    Sums.Clear();
    Peaks.Clear();
    Frames = 0;
}

Double FxPerformanceRootStatItem::GetAverage(unsigned statId) const
{
    StatInfo statInfo;

    if (Sums.GetStat(&statInfo, statId))
    {
        const TimerStat& stat = *statInfo.ToTimerStat();

        return Frames ? static_cast<Double>(stat.GetTicks()) / (Frames * 1000) : 0.0;
    }

    return 0.0;
}

Double FxPerformanceRootStatItem::GetPeak(unsigned statId) const
{
    StatInfo statInfo;

    if (Peaks.GetStat(&statInfo, statId))
    {
        const TimerStat& stat = *statInfo.ToTimerStat();

        return static_cast<Double>(stat.GetTicks()) / 1000;
    }

    return 0.0;
}

// Virtual method.
void FxPerformanceRootStatItem::GetText(String* pstring)
{
    pstring->Clear();
}

// Virtual method.
void FxPerformanceRootStatItem::GetName(String* pstring)
{
    *pstring = "Performance";
}

///////////////////////////////////////////////////////////////////////////////
FxPerformanceElementStatItem::FxPerformanceElementStatItem( 
    FxStatItem* pparent,
    const FxPerformanceRootStatItem& parent, 
    unsigned statId 
    ) 
: BaseClass(pparent)
, Parent(parent)
, StatId(statId)
{
    SetName(StatDesc::GetDesc(StatId)->GetName());
    SetExpanded(true);
}

void FxPerformanceElementStatItem::Update( const FxStatUpdate& update )
{
    if (update.GetType() == FxStatUpdate::Update_Tick)
    {
        String text;
        Format(text, "|8>  {0:.3}|11>  {1:.3}",
            GetParent().GetAverage(StatId),
            GetParent().GetPeak(StatId)
            );
        SetText(text);
    }
}

