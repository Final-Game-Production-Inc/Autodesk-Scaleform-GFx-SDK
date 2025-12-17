/**************************************************************************

Filename    :   FxListBox.h
Content     :   GFx to C++ ListBox
Created     :   2/23/2007
Authors     :   David Cook, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxListBox_H
#define INC_FxListBox_H

#include "GFxPlayer.h"
#include "FxCommand.h"
#include "GStd.h"
#include <vector>
#include <algorithm>


// ********************************************************************
// Base class for all FxListBox list items
//
class FxListItem : public GRefCountBase<FxListItem, GStat_Default_Mem>
{ 
public:
    
    // Enabled flag. The listitem in the UI will display if its 
    // disabled state if this variable is set to false.
    bool Enabled;

    FxListItem() : Enabled(true) {}
    virtual ~FxListItem() {}

    // Get the display values of the listitem. These values are 
    // displayed by the listem in the UI.
    // Fill the GFxValue with the value mapped by the given key
    virtual void    operator () (GFxValue& pvalue, 
        const GString& key) = 0;
};


// ********************************************************************
// Base class for all FxListItem comparators
//
//
// (PPS Note: THIS CAN BE MOVED TO FxSortedListBox)
class FxListItemComparator : public GRefCountBase<FxListItemComparator, GStat_Default_Mem>
{
public:
    virtual void    SetSortId(int id)           { SortID = id; }
    int             GetSortId() const           { return SortID; }
    virtual void    SetAscendingSort(bool flag)     { Ascending = flag; }
    bool            IsAscendingSort() const        { return Ascending; }

    void CopySortMetricsFrom(GPtr<FxListItemComparator> comp)
    {
        SortID = comp->SortID;
        Ascending = comp->Ascending;
    }

    // Derived classes will implement the comparison logic, but are 
    // expected to return the following:
    // negative values  : if pa < pb
    // 0                : if pa == pb
    // positive values  : if pa > pb
    virtual int     Compare(const GPtr<FxListItem>& pa, const 
        GPtr<FxListItem>& pb) const = 0;

protected:

    // The sort key id (works in conjunction with the sort map assigned 
    // to the corresponding listbox)
    int SortID;     

    // True if the sorting is expected to be in ascending order
    bool Ascending;  
};



// ********************************************************************
// Base class for all list boxes that require communication with the 
// FxListBox actionscript class. 

// It can also be instantiated to allow composition of multiple 
// listboxes by a wrapper class. This provides functionality for 
// special cases, such as where a single actionscript FxListBox widget 
// is driven by two C++ models. In this case a wrapper class will 
// create two FxListBox objects and shunt fscommands to the appropriate 
// object.
//
class FxListBox :   public FxCommandHandler
{
public:

    // Display mode determines if the listbox infers the view state 
    // automatically or expects the derived class to set the view 
    // state appropriately.
    enum DisplayModeType
    {
        DM_Manual,
        DM_Automatic,
    };

    // View state determines if on add/remove of a listitem, the 
    // listbox view should follow a certain behavior (ie: keep the 
    // display index fixed, etc.)
    enum ViewStateType
    {
        VS_None,
        VS_IndexFixed,
        VS_SelectionFixed,
    };

    // (PPS Note: THIS CAN BE MOVED TO FxSortedListBox)
    // Sort map
    // An array of these elements constitutes a sort map. The ID 
    // and name are henceforth mapped to each other, and one can be 
    // found using the other.
    struct SortMapping
    {
        int     ID;
        char*   pName;
    };

    // *** GFx List Box Commands
    // Derived classes or wrappers need to create a command map using 
    // these values. The corresponding actionscript listbox widget 
    // takes a Tag string in its constructor. This Tag is appended to 
    // certain fscommand strings to allow identification of where the 
    // fscommand originated from. The following is the complete set 
    // of commands emitted from an FxListBox widget (where MyListTag is
    // the Tag used in the widget's constructor)
    //
    // GFxCommand FxListBox_Commands[] =
    // {
    //    { CMD_FxList_OnInitialize,         "MyListTag_OnInitialize"       },
    //    { CMD_FxList_OnSetVisibleCount,    "MyListTag_OnSetVisibleCount"  },
    //    { CMD_FxList_OnSetItemDataMap,     "MyListTag_OnSetItemDataMap"   },
    //    { CMD_FxList_OnScroll,             "MyListTag_OnScroll"           },
    //    { CMD_FxList_OnScrollDrag,         "MyListTag_OnScrollDrag"       },
    //    { CMD_FxList_OnItemSelect,         "MyListTag_OnItemSelect"       },
    //    { CMD_FxList_OnItemRollover,       "MyListTag_OnItemRollover"     },
    //    { CMD_FxList_OnItemRollout,        "MyListTag_OnItemRollout"      },
    //    { CMD_FxList_OnHeaderPress,        "MyListTag_OnHeaderPress"      },
    //    { CMD_FxList_OnHeaderRelease,      "MyListTag_OnHeaderRelease"    },
    //    { CMD_FxList_OnClearSelection,     "MyListTag_OnClearSelection"   },
    //    { CMD_FxList_OnSelectionScroll,    "MyListTag_OnSelectionScroll"  },
    //    { CMD_FxList_OnExecute,            "MyListTag_OnExecute" },
    //    { 0, 0},
    // };
    //
    enum CommandKeys
    {
        // (PPS Note: THESE CAN BE MOVED TO FxWidget)
        CMD_FxList_OnInitialize = 0xF0, 
        CMD_FxList_OnDestroy,

        // (PPS Note: THESE CAN BE MOVED TO FxScrollableWidget) 
        CMD_FxList_OnSetVisibleCount,
        CMD_FxList_OnScroll, 
        CMD_FxList_OnScrollDrag, 

        CMD_FxList_OnSetItemDataMap, 
        CMD_FxList_OnItemSelect,
        CMD_FxList_OnItemRollover,
        CMD_FxList_OnItemRollout,
        CMD_FxList_OnClearSelection,
        CMD_FxList_OnSelectionScroll,

        // This command has no default implementation, and derived
        // classes are requried to implement a handler for it.
        CMD_FxList_OnExecute,    
                                        
        // (PPS Note: THESE CAN BE MOVED TO FxSortedListBox)
        CMD_FxList_OnHeaderRelease,
        CMD_FxList_OnHeaderPress,
        
        CMD_FxList_NumCommands,
    };

    // Methods that dervied classes should implement to return a commandmap
    virtual FxCommand* GetCommandMap() = 0;

    // All commands registered to the command manager are then sent to 
    // the VFsCallback function. Derived classes should return 1 if 
    // command handled, otherwise, 0 to continue processing
    virtual bool                VFsCallback(class GFxMovieView* pmovie, 
                                    int cmdKey, const char* cmdName, const char* args);    

    FxListBox();

    // Initialization methods
    // (PPS Note: THESE CAN BE MOVED TO FxWidget)
    void                        RegisterMovieContainingWidget(GFxMovieView* pmovie);
    bool                        IsWidgetInitialized() { return ListBoxPB.HasParentPath(); }
 
    // Methods to communicate with the ActionScript widget
    void                        SetAutoUpdateUI(bool flag);         
    // Sets the DisplayIndex, NumItems, VisibleItems
    void                        UISetItemInfo();                                       
    void                        UIRefreshView();

    // List data related methods
    SPInt                       AddListItem(GPtr<FxListItem> pitem, bool select = false);
    SPInt                       RemoveListItem(GPtr<FxListItem> pitem);
    void                        RemoveAllListItems();
    GPtr<FxListItem>            GetListItem(UPInt index) const;

    // (PPS Note: THESE CAN BE MOVED TO FxScrollablWidget)
    // View visible size related methods
    void                        SetVisibleItemsCount(UPInt vi);
    UPInt                       GetVisibleItemsCount() const;

    // Display index (index of first row in move) related methods
    UPInt                       GetDisplayIndex() const;
    bool                        SetDisplayIndex(UPInt di);
    bool                        SetDisplayIndexByPercent(Double pos);
    bool                        SetDisplayIndexByOffset(SPInt delta);

    // List count related methods
    UPInt                       GetListItemsCount() const;
    void                        SetListItemsCount(UPInt count);

    // View positioning related methods
    void                        MakeItemVisible(SPInt idx);
    bool                        IsVisible(GPtr<FxListItem> pitem);

    // View state related methods
    ViewStateType               GetViewState() const;
    void                        SetViewState(ViewStateType state);
    void                        DetermineViewState();

    // Display mode related methdos
    void                        SetDisplayMode(DisplayModeType mode);

    // Selection related methods
    bool                        IsItemSelected();
    // Sets the selected item index to -1
    void                        ClearSelection();                                       
    GPtr<FxListItem>            GetSelectedItem();
    void                        SetSelectedItem(GPtr<FxListItem> pItem);
    void                        SetSelectedItemIndex(SPInt idx);
    // The selection was scrolled
    bool                        SetSelectionItemIndexByOffset(int val);                 
    SPInt                       GetSelectedItemIndex();

    // Sort related methods
    // (PPS Note: THESE CAN BE MOVED TO FxSortedListBox)
    // Sort the list based on the comparator's settings
    void                        Sort();                                                 
    // Sort the list based on a column ID (from the sort map)
    void                        SortUsingNewColumn(int newId);                          
    void                        SetItemCompare(GPtr<FxListItemComparator> pcompare);
    GPtr<FxListItemComparator>  GetItemCompare() const;
    void                        SetSortMapping(const SortMapping *pmappings, UInt numColumns);   
    const char*                 GetSortNameByID(int id) const;
    int                         GetSortIDByName(const char *name) const;

    // Reset the listbox (clears all items, 0s dimension values, etc.)
    void                        Reset();                

protected:

    // (PPS Note: THESE CAN BE MOVED TO FxWidget)
    // The movie containing the widget
    GFxMovieView*               pMovie;                 
    // Set the ActionScript path to the list box widget
    void                        SetASWidgetPath(const char* plp);                  
    // Character buffer holding the path to the list box widget.   
    FxSwfPathBuilder            ListBoxPB;

private:

    // (PPS Note: THESE CAN BE MOVED TO FxSortedListBox)
    // The sort map
    const SortMapping*          pSortMap;               
    // Number of sort columns
    UInt                        NumSortColumns;         

    // Temporary structure to hold the column names (used to retrieve values from a list item)
    GArray<GString>      ListItemDataKeys;       
    // Temporary structure to hold values returned in conjunction with a key in ItemData
    GArray<GFxValue>       ListItemDataValues;     

    // Helper to add one item to the UI list
    void                        UIAddItem(GPtr<FxListItem> pli);   

    // Keep the display index within 0 and ListItemsCount-1
    void                        CheckDisplayIndex();    

    // (PPS Note: THIS CAN BE MOVED TO FxScrollablWidget)
    // Number of visible items in the list widget. (ie: number of rows in the viewport)
    UPInt                       VisibleItemsCount;      

    // The item index of the topmost list item in the viewport
    UPInt                       ViewTopIndex;           
    // The total number of items in the list
    UPInt                       ListItemsCount;         
    
    // The list items
    GArray<GPtr<FxListItem> >     ListItems;     
    // The index of the selected item (-1 if no item is selected)
    SPInt                       SelectedItemIndex;      
    
    // (PPS Note: THIS CAN BE MOVED TO FxSortedListBox)
    // The list item comparator
    GPtr<FxListItemComparator>  pItemCompare;           
    
    // See ViewStateType enum for description
    ViewStateType               ViewState;              
    // See DisplayModeType enum for description
    DisplayModeType             DisplayMode;            

    // If true, any changes to the listbox will
    // immediately causes a view refresh
    bool                        AutoUpdateUI;           

    //
    // FxListItem Comparator Wrapper
    //
    // This wrapper is required for the std::sort algorithm
    //
    // (PPS Note: THIS CAN BE MOVED TO FxSortedListBox)
    struct FxListItemComparatorWrapper
    {
        GPtr<FxListItemComparator> Comparator;

        FxListItemComparatorWrapper(GPtr<FxListItemComparator> cmp)
        {
            Comparator = cmp;
        }

        bool operator() (const GPtr<FxListItem> pa, const GPtr<FxListItem> pb) const
        {
            return (Comparator->Compare(pa, pb) < 0);
        }

    };
};


#endif // INC_FxListBox_H
