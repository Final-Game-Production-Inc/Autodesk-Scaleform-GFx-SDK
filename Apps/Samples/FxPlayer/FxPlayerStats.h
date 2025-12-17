/**************************************************************************

Filename    :   FxPlayerStats.h
Content     :   Statistics reporting for FxPlayer 
Created     :   February 23, 2009
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxPlayerStats_H
#define INC_FxPlayerStats_H

#include "FxStatItem.h"

///////////////////////////////////////////////////////////////////////////////
class FxPerformanceRootStatItem;

class FxPerformanceElementStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;

public:
    FxPerformanceElementStatItem(
        FxStatItem* pparent,
        const FxPerformanceRootStatItem& parent,
        unsigned statId
        );

public:
    virtual void Update(const FxStatUpdate& update);

    const FxPerformanceRootStatItem& GetParent() const { return Parent; }

private:
    FxPerformanceElementStatItem& operator =(const FxPerformanceElementStatItem&);

private:
    const FxPerformanceRootStatItem&    Parent;
    const unsigned                      StatId;
};

///////////////////////////////////////////////////////////////////////////////
// Represents the performance root which has the following items
//
// +Advance
//  - Action Script
//  - Timeline
//  - Input
// - Display

class FxPerformanceRootStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;

public:
    FxPerformanceRootStatItem(
        FxGlobalStatTracker* pstatTracker,
        Movie* pmovie
        );

public:
    virtual FxGlobalStatTracker* GetStatTracker() const { return pStatTracker; }

    virtual void    GetName(String* pstring);
    virtual void    GetText(String* pstring);

    virtual void    Update(const FxStatUpdate& update);

    void SetMovie(Movie* pmovie) { pMovieView = pmovie; OnTickStat(); }

    // Retrieve statistic.
    Double GetAverage(unsigned statId) const;
    Double GetPeak(unsigned statId) const;

private:
    FxPerformanceRootStatItem* GetThis() { return this; }
    void OnFrameStat();
    void OnTickStat();

private:
    typedef ArrayLH<Ptr<FxPerformanceElementStatItem> > ChildrenType;

    FxGlobalStatTracker*        pStatTracker;
    Ptr<Movie>          pMovieView;
    StatBag                    Sums;
    StatBag                    Peaks;
    unsigned                    Frames;
    ChildrenType                Children;

    Ptr<FxMessageStatItem>     pHeapInfoItem;
};

#endif // INC_SF_GFX_PLAYERSTATS_H
