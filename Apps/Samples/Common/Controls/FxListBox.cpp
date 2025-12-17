/**************************************************************************

Filename    :   FxListBox.cpp
Content     :   GFx to C++ ListBox
Created     :   2/23/2007
Authors     :   David Cook, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GAlgorithm.h"
#include "FxListBox.h"

template<class Container, class EqualityComparable>
SPInt    FindItem(const Container &c, const EqualityComparable &val)
{
    SPInt size = (SPInt)c.GetSize();
    for (SPInt i=0; i<size; i++)
        if (c[i] == val)
            return i;
    return -1; 
}


// ********************************************************************
// FxListBox constructor
//
// Initializes internal states.
//
FxListBox::FxListBox()
{
    pSortMap        = 0;
    pMovie          = NULL;

    DisplayMode     = DM_Automatic;
    ViewState       = VS_IndexFixed;

    AutoUpdateUI    = true;

    Reset();    
}


// ********************************************************************
// Register the movie containing the list box view.
//
void FxListBox::RegisterMovieContainingWidget(GFxMovieView* pmovie)
{
    pMovie = pmovie;
}


// ********************************************************************
// Register the list box view (widget's) path
//
// This path is used for C++ to ActionScript communication 
//
void FxListBox::SetASWidgetPath(const char* plp)
{
    GFC_DEBUG_ASSERT( (plp != NULL), "Parameter cannot be NULL!" );

    ListBoxPB.SetParentPath(plp);
}


// ********************************************************************
// Set the flag that controls redrawing on list update
//
// If true, adding and removing elements from the list will force
// an immediate refresh of the list view. Disabling auto update
// allows users to control refresh rates.
//
void FxListBox::SetAutoUpdateUI(bool flag)
{
    AutoUpdateUI = flag;
}


// ********************************************************************
// Refresh the UI view
//
void FxListBox::UIRefreshView()
{
    if (!ListBoxPB.HasParentPath())
        return;

    // Set the display index, display count, visible count
    UISetItemInfo();

    // Clear the list
    pMovie->Invoke(ListBoxPB.BuildPath(".ClearList"), "");

    // Add the visible items
    UInt count = (ViewTopIndex+VisibleItemsCount < ListItemsCount) ? 
        ViewTopIndex+VisibleItemsCount : ListItemsCount;
    if (count)
    {
        for (UInt i = ViewTopIndex; i < count; i++)
            UIAddItem(GetListItem(i));

    }

    // Display the visible items
    pMovie->Invoke(ListBoxPB.BuildPath(".SetList"), "");
}


// ********************************************************************
// A helper to add an item to the UI
//
void FxListBox::UIAddItem(GPtr<FxListItem> pli)
{
    if ( (!ListBoxPB.HasParentPath()) || (pli.GetPtr() == NULL) )
        return;

    // Fill in the temporary structures with the appropriate data
    for (unsigned i=0; i<ListItemDataKeys.GetSize(); i++)
    {
        (*pli)(ListItemDataValues[i], ListItemDataKeys[i]);
    }
    // Set the last data value to be the enabled/disabled flag
    ListItemDataValues[ListItemDataKeys.GetSize()].SetBoolean(pli->Enabled);

    // Add the item; params are an array of values expected by the list box
    // the expected order of values is set when SetItemDataMap is called
    pMovie->Invoke(ListBoxPB.BuildPath(".Add"), &ListItemDataValues[0], ListItemDataValues.GetSize());
}


// ********************************************************************
// Set the item info in the UI
//
// The current display index, display count and visible count is sent 
// to the view.
//
void FxListBox::UISetItemInfo()
{
    if (!ListBoxPB.HasParentPath())
        return;

    GFxValue v[3];
    v[0] = (Double)ViewTopIndex;
    v[1] = (Double)ListItemsCount;
    v[2] = (Double)VisibleItemsCount;
    pMovie->Invoke(ListBoxPB.BuildPath(".SetItemInfo"), v, 3);

    pMovie->Invoke(ListBoxPB.BuildPath(".SelectDataItem"), "%d", SelectedItemIndex);
}


// ********************************************************************
// Sort on the provided column by idx
//
void FxListBox::SortUsingNewColumn(int newId)
{
    GPtr<FxListItemComparator> pItemCompare = GetItemCompare();
    
    if (pItemCompare.GetPtr() == NULL)
        return;

    int oldId = pItemCompare->GetSortId();
    bool order = pItemCompare->IsAscendingSort();

    if (oldId == newId)
    {
        order   = !order;
    }
    else if (newId == -1)   // default sort
    {
        newId = oldId;
    }   
    else
    {
        pItemCompare->SetSortId(newId);
    }

    pItemCompare->SetAscendingSort(order);

    // Sort the item list
    Sort();

    const char *pname = GetSortNameByID(newId);
    if (!pname)
        pname = "";

    GFxValue v[2];
    v[0] = GFxValue(pname);
    v[1] = GFxValue(order);
    pMovie->Invoke(ListBoxPB.BuildPath(".SetSortColumn"), v, 2);
}


// ********************************************************************
// Set the sort mapping
//
void FxListBox::SetSortMapping(const SortMapping *p, UInt numColumns)
{
    pSortMap = p;
    NumSortColumns = numColumns;
}


// ********************************************************************
// Get the sort column idx mapping by name
//
int     FxListBox::GetSortIDByName(const char* name) const
{
    if (pSortMap == NULL)
        return -1;
    for (UInt i=0; i < NumSortColumns; i++)
    {
        if ( G_strcmp(pSortMap[i].pName, name) == 0 )
            return pSortMap[i].ID;
    }
    return -1;
}


// ********************************************************************
// Get the sort column name mapping by idx
//
const char* FxListBox::GetSortNameByID(int id) const
{    
    if (pSortMap == NULL)
        return NULL;
    for (UInt i=0; i < NumSortColumns; i++)
    {
        if (id == pSortMap[i].ID)
            return pSortMap[i].pName;
    }
    return NULL;
}


// ********************************************************************
// Scroll the selection
//
bool FxListBox::SetSelectionItemIndexByOffset(int val)
{
    int oldIdx = GetSelectedItemIndex();
    int sidx = oldIdx;
    UInt pageSz = VisibleItemsCount;

    if ((val*val) == (int)(pageSz*pageSz))
    {
        int topIdx = ViewTopIndex;
        // page scrolling
        if (val < 0)
        {
            // scrolling up
            if (oldIdx == topIdx)
            {
                // scroll full page
                sidx += val + 1;
            }
            else
            {
                // scroll until top of current page
                sidx = topIdx;
            }
        }
        else
        {
            int bottomIdx = topIdx + VisibleItemsCount - 1;
            // scrolling down
            if (oldIdx == bottomIdx)
            {
                // scroll full page
                sidx += val - 1;
            }
            else
            {
                // scroll until end of current page
                sidx = bottomIdx;
            }
        }
    }
    else
    {
        // normal scrolling
        sidx += val;
    }

    //int sz = pItemList->GetDisplayItems().size();
    int sz = ListItemsCount;
    if (sidx < 0)
        sidx = 0;
    else if (sidx > (sz-1))
        sidx = sz-1;
    SetSelectedItemIndex(sidx);

    if (oldIdx == GetSelectedItemIndex())
    {
        // no action required
        return false;
    }
    else
    {
        MakeItemVisible(sidx);
        return true;
    }
}


// ********************************************************************
// fscommand callback function
//
bool FxListBox::VFsCallback(GFxMovieView* pmovie, int commandKey, 
                            const char* command, const char* args)
{
    GUNUSED2(pmovie, command);
    switch(commandKey)
    {
    case CMD_FxList_OnInitialize:
        {
            SetASWidgetPath(&args[0]);
            SortUsingNewColumn(-1); // force default sort       
            UIRefreshView();
            return 1;
        }
    case CMD_FxList_OnDestroy:
        {
            SetASWidgetPath("");
            return 1;
        }
    case CMD_FxList_OnSetVisibleCount:
        {
            int val = atoi(&args[0]);
            SetVisibleItemsCount(val);
            UIRefreshView();
            return 1;
        }
    case CMD_FxList_OnSetItemDataMap:
        {
            ListItemDataKeys.Clear();
            ListItemDataValues.Clear();

            int i=0, w=0;
            GFxValue value;
            while (args[i++])
            {
                if (args[i] == ',' || args[i] == ':' || args[i] == 0)
                {
                    int len = (i-w)+1;
                    char *str = (char*)malloc(len);
                    G_strncpy(str, len, args+w, i-w);
                    str[len-1] = 0;
                    ListItemDataKeys.PushBack(str);
                    ListItemDataValues.PushBack(value);
                    free(str);
                    w = i+1;
                }
            }
            // include an extra slot for enabled flag
            ListItemDataValues.PushBack(value);

            return 1;
        }
    case CMD_FxList_OnItemSelect:
        {
            //OnItemSelected(atoi(args));
            SetSelectedItemIndex( atoi(args) );
            return 1;
        }
    case CMD_FxList_OnScroll:
        {
            // move the display
            if ( SetDisplayIndexByOffset(atoi(args) ) )
            {
                UIRefreshView();
            }
            return 1;
        }
    case CMD_FxList_OnScrollDrag:
        {
            if ( SetDisplayIndexByPercent(atof(args)) )
            {
                UIRefreshView();
            }
            return 1;
        }
    case CMD_FxList_OnSelectionScroll:
        {
            if ( SetSelectionItemIndexByOffset(atoi(args)) ) 
            {
                UIRefreshView();
            }
            return 1;
        }
    case CMD_FxList_OnItemRollover:
        {
            return 1;
        }
    case CMD_FxList_OnItemRollout:
        {
            return 1;
        }
    case CMD_FxList_OnHeaderRelease:
        {
            int sid = GetSortIDByName(args);
            if (sid>=0)
                SortUsingNewColumn(sid);
            UIRefreshView();
            return 1;
        }
    case CMD_FxList_OnHeaderPress:
        {
            return 1;
        }
    case CMD_FxList_OnClearSelection:
        {
            //OnSelectedItemCleared();
            SetSelectedItemIndex( -1 );
            return 1;
        }
    }
    return 0;
}



// ********************************************************************
// Add a new ListItem
//
// Select it if needed.
//
SPInt FxListBox::AddListItem(GPtr<FxListItem> pitem, bool select)
{
    // find the insertion index
    SPInt insertIdx = -1;
    if (pItemCompare)
    {
        // if there is a comparator, try to find the item in the 
        // displayed list
        insertIdx = G_LowerBound(ListItems, pitem, FxListItemComparatorWrapper(pItemCompare));

        GPtr<FxListItem> ptr = GetListItem(insertIdx);
        if (ptr)
        {
            if (pItemCompare->Compare(ptr.GetPtr(), pitem.GetPtr()) == 0)
            {
                GFC_DEBUG_MESSAGE1(1, 
                    "FxListBox::Add(%d) - Found a duplicate! Not adding..", 
                    ptr.GetPtr());
                return -1;
            }
        }
    }
    else
    {
        // else, insert to the end
        insertIdx = ListItems.GetSize();
    }

    // depending on the display state, check to see if displayindex 
    // needs to be shifted to keep the view from scrolling
    if (ViewTopIndex < ListItemsCount)
    {
        switch (ViewState)
        {
        case VS_IndexFixed:
            {
                // maintain the current display index to keep the top 
                // most element in the view in-place
                if ( (ViewTopIndex != 0) && ((SPInt)ViewTopIndex >= insertIdx) )
                    ViewTopIndex++;
                break;
            }
        case VS_SelectionFixed:
            {   
                // maintain the display index to keep the selected 
                // item in-place
                if (SelectedItemIndex >= insertIdx)
                    ViewTopIndex++;
                break;
            }
        }
    }

    // track the current selection
    GPtr<FxListItem> pSelection = NULL;
    if (select)
        pSelection = pitem;
    else if (SelectedItemIndex != -1)
        pSelection = ListItems[SelectedItemIndex];

    // insert the element and assign the selection afterwards
    ListItems.InsertAt(insertIdx, pitem);
    ListItemsCount++;
    CheckDisplayIndex();
    SetSelectedItem(pSelection);

    if (AutoUpdateUI)
        UISetItemInfo();

    // return the insertion index
    return insertIdx;
}


// ********************************************************************
// Remove a ListItem
//
SPInt FxListBox::RemoveListItem(GPtr<FxListItem> pitem)
{
    // Try to find the server in the displayed list
    SPInt removeIdx = -1;
    if (pItemCompare)
    {
        // if there is a comparator, try to find the item in the 
        // displayed list
        removeIdx = G_LowerBound(ListItems, pitem, FxListItemComparatorWrapper(pItemCompare));
    }
    else
    {
        // else, iterate through all elements to find the item
        removeIdx = FindItem(ListItems, pitem);
    }

    // if removal index was not found, return error (-1)
    if (removeIdx == -1)
    {
        return removeIdx;
    }

    // if the item to be removed is selected, lose the selection
    if (SelectedItemIndex != -1 && pitem == GetListItem(SelectedItemIndex))
    {
        SetSelectedItemIndex(-1);
    }

    // depending on the display state, check to see if displayindex 
    // needs to be shifted to keep the view from scrolling
    if (ViewTopIndex > 0)
    {
        // Adjust the display index
        switch (ViewState)
        {
        case VS_IndexFixed:
            {
                // maintain the current display index to keep the top 
                // most element in the view in-place
                if ( (ViewTopIndex != 0) && ((SPInt)ViewTopIndex >= removeIdx) )
                    ViewTopIndex--;
                break;
            }
        case VS_SelectionFixed:
            {   
                // maintain the display index to keep the selected item
                // in-place if the selected item was to be removed, 
                // then SelectedItemIndex==-1, and the display state is 
                // DISP_Index, thus the logic holds true
                if ( SelectedItemIndex >= removeIdx )
                    ViewTopIndex--;
                break;
            }
        }
    }

    // track the current selection
    GPtr<FxListItem> pItem = NULL;
    if (SelectedItemIndex != -1)
        pItem = ListItems[SelectedItemIndex];

    // remove the element and assign the selection afterwards
    ListItems.RemoveAt(removeIdx);
    ListItemsCount--;
    CheckDisplayIndex();
    SetSelectedItem(pItem);

    if (AutoUpdateUI)
        UISetItemInfo();

    // return the removal index
    return removeIdx;
}


// ********************************************************************
// Remove all ListItems
//
// Maintains visible count.
//
void FxListBox::RemoveAllListItems()
{
    ListItems.Clear();
    ListItemsCount = 0;
    ViewTopIndex = 0;
    ClearSelection();
}

// ********************************************************************
// Clears all variables
//
void FxListBox::Reset()
{
    VisibleItemsCount = 0;
    RemoveAllListItems();
}


// ********************************************************************
// Returns item at index
//
GPtr<FxListItem>   FxListBox::GetListItem(UPInt index) const
{
    if ( index >= ListItems.GetSize() )
        return 0;
    return ListItems[index];
}


// ********************************************************************
// Set the visible item count
//
void FxListBox::SetVisibleItemsCount(UPInt vi)
{
    VisibleItemsCount = vi;
    MakeItemVisible(SelectedItemIndex);
    CheckDisplayIndex();
    DetermineViewState();
}


// ********************************************************************
// Get the visible item count
//
UPInt FxListBox::GetVisibleItemsCount() const
{
    return VisibleItemsCount;
}


// ********************************************************************
// Set the display item count
//
void FxListBox::SetListItemsCount(UInt count)
{
    ListItemsCount = count;
}

// ********************************************************************
// Get the display item count
//
UPInt FxListBox::GetListItemsCount() const
{
    return ListItemsCount;
}


// ********************************************************************
// Set the display index
//
bool FxListBox::SetDisplayIndex(UInt di)
{
    if (di == ViewTopIndex) return false;
    ViewTopIndex = di;
    CheckDisplayIndex();
    DetermineViewState();
    return true;
}


// ********************************************************************
// Get the display index
//
UPInt FxListBox::GetDisplayIndex() const
{
    return ViewTopIndex;
}


// ********************************************************************
// Set the display index by percent
//
// Min=0, Max=DisplayCount
//
bool FxListBox::SetDisplayIndexByPercent(double pos)
{
    if (pos < 0.0)
        pos = 0.0;
    if (pos > 1.0)
        pos = 1.0;
    double idx = (pos * (ListItemsCount - VisibleItemsCount + 1));
    UInt di = (unsigned) floor(idx + 0.5);
    if (di == ViewTopIndex) return false;
    ViewTopIndex = di;
    CheckDisplayIndex();
    DetermineViewState();
    return true;
}


// ********************************************************************
// Set the display index by offset
//
// Integer offset from the current position
//
bool FxListBox::SetDisplayIndexByOffset(int delta)
{
    int i = int(ViewTopIndex) + delta;
    if (i < 0) 
        i = 0;
    if (i == int(ViewTopIndex)) return false;
    ViewTopIndex = (unsigned) i;
    CheckDisplayIndex();
    DetermineViewState();
    return true;
}

// ********************************************************************
// Make the item at the index visible in the view
//
void FxListBox::MakeItemVisible(SPInt idx)
{
    if (idx < 0)
        return;
    if (ViewTopIndex > (UInt)idx) 
    {
        SetDisplayIndex(idx);
    }
    else if ( (ViewTopIndex + VisibleItemsCount - 1) < (UInt)idx ) 
    { 
        SetDisplayIndex(idx - VisibleItemsCount + 1);
    }
}

// ********************************************************************
// Returns true if the item is visible in view
//
bool FxListBox::IsVisible(GPtr<FxListItem> pitem)
{
    UPInt count = (ViewTopIndex+VisibleItemsCount < ListItemsCount) ? 
        ViewTopIndex+VisibleItemsCount : ListItemsCount;
    for (UPInt i=ViewTopIndex; i < count; i++)
    {
        if (ListItems[i] == pitem)
            return true;
    }
    return false;
}


// ********************************************************************
// Check the display index and fix it if display index is out of bounds
//
void FxListBox::CheckDisplayIndex()
{
    if (ListItemsCount < VisibleItemsCount)
        ViewTopIndex = 0;
    else if (ViewTopIndex > ListItemsCount - VisibleItemsCount)
        ViewTopIndex = ListItemsCount - VisibleItemsCount;
}


// ********************************************************************
// Set the view state variable based on the state of the other
// variables.
//
// The logic tries to infer the view state automatically (such as
// maintain display index if no item is selected, etc.)
//
void FxListBox::DetermineViewState()
{
    if (DisplayMode == DM_Manual)
        return;

    // if displaymode is automatic, then figure out view state
    ViewStateType ds = VS_IndexFixed;

    // make sure the selected item is not in the display view
    unsigned idx = SelectedItemIndex;
    if ( (idx >= ViewTopIndex) && 
        (idx < ViewTopIndex + VisibleItemsCount) )
        ds = VS_SelectionFixed;

    ViewState = ds;
}


// ********************************************************************
// Get the view state
//
FxListBox::ViewStateType FxListBox::GetViewState() const
{
    return ViewState;
}


// ********************************************************************
// Set the view state
//
// View state determines the behavior of the list view when items
// are added and removed. (Only when display mode is not manual)
//
void FxListBox::SetViewState(FxListBox::ViewStateType state)
{
    GFC_DEBUG_MESSAGE(DisplayMode == DM_Automatic, 
        "WARNING! SetViewState is useless when display mode is automatic!");
    ViewState = state;
}


// ********************************************************************
// Set the display mode
//
// If manual mode, view state is disregarded.
//
void FxListBox::SetDisplayMode(FxListBox::DisplayModeType mode)
{
    DisplayMode = mode;
}

// ********************************************************************
// Returns true if an item is selected
//
bool FxListBox::IsItemSelected()
{
    return (SelectedItemIndex != -1);
}


// ********************************************************************
// Clear the current selection
//
void FxListBox::ClearSelection()
{
    SetSelectedItemIndex(-1);
}


// ********************************************************************
// Set the currently selected item
//
void FxListBox::SetSelectedItemIndex(int idx)
{
    if ( (idx > -1) && (idx < (int)ListItemsCount ) )
    {
        SelectedItemIndex = idx;
    }
    else
    {
        SelectedItemIndex = -1;
    }
    DetermineViewState();
}

// ********************************************************************
// Get the current selection's index
//
int FxListBox::GetSelectedItemIndex()
{
    return SelectedItemIndex;
}


// ********************************************************************
// Set the item as the currently selected item
//
void FxListBox::SetSelectedItem(GPtr<FxListItem> pItem)
{
    SetSelectedItemIndex( FindItem(ListItems, pItem) );
}


// ********************************************************************
// Get the currently selected item
//
GPtr<FxListItem> FxListBox::GetSelectedItem()
{
    return GetListItem(SelectedItemIndex);
}


// ********************************************************************
// Sort using the current sort settings (if there is a comparator)
//
void FxListBox::Sort()
{
    if (!pItemCompare)
        return;

    FxListItem* pItem = NULL;
    if (SelectedItemIndex > -1)
        pItem = ListItems[SelectedItemIndex];

    if (ListItemsCount < 1)
        return;

    std::sort(ListItems.Begin().GetPtr(), ListItems.Begin().GetPtr() + ListItems.GetSize(), 
        FxListItemComparatorWrapper(pItemCompare) );

    CheckDisplayIndex();
    SetSelectedItem(pItem);
    MakeItemVisible(SelectedItemIndex);
}


// ********************************************************************
// Set the ListItem comparator
//
void FxListBox::SetItemCompare(GPtr<FxListItemComparator> pcompare)
{
    pItemCompare = pcompare;
}


// ********************************************************************
// Get the ListItem comparator
//
GPtr<FxListItemComparator> FxListBox::GetItemCompare() const
{
    return pItemCompare;
}
