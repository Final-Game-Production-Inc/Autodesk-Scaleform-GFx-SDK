/**************************************************************************

Filename    :   GFx_CandidateListBox.h
Content     :   GFx to C++ ListBox
Created     :   10/12/2007
Authors     :   David Cook, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FXCANDIDATELISTBOX_H
#define INC_FXCANDIDATELISTBOX_H

#include "GFx/GFx_Player.h"
#include "GFx_Command.h"
#include "Kernel/SF_Std.h"
#include "GFx/GFx_PlayerStats.h"
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_RefCount.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{

class GFxIMEWin32Impl;
// ****************************************************************************
// Base class for all FxListBox list items
//
class FxCandidateListItem : public RefCountBaseNTS<FxCandidateListItem, StatIME_Mem>
{ 
public:

    FxCandidateListItem() {}
    virtual ~FxCandidateListItem() {}

    // Get the display values of the listitem. These values are displayed by
    // the listem in the UI.
    // Fill the Value with the value mapped by the given key
    virtual void    operator () (Value& pvalue, const String& key) = 0;
};

// ****************************************************************************
// Base class for all list boxes that require communication with the FxListBox 
// actionscript class. 

// It can also be instantiated to allow composition of multiple listboxes
// by a wrapper class. This provides functionality for special cases, such as  
// where a single actionscript FxListBox widget is driven by two C++ models. 
// In this case a wrapper class will create two FxListBox objects and shunt
// fscommands to the appropriate object.
//
class FxCandidateListBox :   public FxCommandHandler, public NewOverrideBase<StatIME_Mem>
{
public:

    enum CommandKeys
    {
        CMD_FxList_OnInitialize = 0xF0, 
        CMD_FxList_OnDestroy = 0xF1,
        CMD_FxList_OnSetVisibleCount = 0xF2,
        CMD_FxList_OnSetItemDataMap = 0xF3, 
        CMD_FxList_OnItemSelect = 0xF6,

        CMD_FxList_NumCommands,
    };

    virtual FxCommand* GetCommandMap() = 0;

    // All commands registered to the command manager are then sent to the VFsCallback function.
    // Derived classes should return 1 if command handled, otherwise, 0 to continue processing
    virtual bool                VFsCallback(Movie* pmovie, 
                                    int cmdKey, const char* cmdName, const char* args);    

	FxCandidateListBox(GFxIMEWin32Impl* pimeImpl);

    virtual ~FxCandidateListBox(){};

    // Initialization methods
    void                        RegisterMovieContainingWidget(Movie* pmovie);
    bool                        IsWidgetInitialized() { return (ListPath[0] != NULL); }
    void                        SetASWidgetPath(const char* plp);                  // Set the ActionScript path to the list box widget
    // Methods to communicate with the ActionScript widget
    void                        UIRefreshView();

    // List data related methods
    SPInt                       AddListItem(Ptr<FxCandidateListItem> pitem, bool select = false);
    void                        RemoveAllListItems();
    void                        RemoveList();
    
    // Selection related methods
    void                        SetSelectedItemIndex(SPInt idx, bool refreshFlashList = false);

protected:

    // (PPS Note: THESE CAN BE MOVED TO FxWidget)
    Movie*						pMovie;                 // The movie containing the widget
    enum                        { PathSize = 1024 };    
    char                        ListPath[PathSize];     // Character buffer holding the path to the list box widget. If ListPath is 0 length, then 
                                                        // the list is considered to be uninitialized. 
    char                        CurrPath[PathSize];     // Temporary character buffer (used to build ActionScript paths for Invoke, etc.)

private:

    Array<String>           ListItemDataKeys;       // Temporary structure to hold the column names (used to retrieve values from a list item)
    Array<Value>          ListItemDataValues;     // Temporary structure to hold values returned in conjunction with a key in ItemData

    void                        UIAddItem(Ptr<FxCandidateListItem> pli);   // Helper to add one item to the UI list

    UPInt                       VisibleItemsCount;      // Number of visible items in the list widget. (ie: number of rows in the viewport)
    
    ArrayLH<Ptr<FxCandidateListItem> > ListItems;     // The list items
    SPInt                       SelectedItemIndex;      // The index of the selected item (-1 if no item is selected)
    GFxIMEWin32Impl				*pImeImpl;
};
}}}



#endif // INC_FXCANDIDATELISTBOX_H
