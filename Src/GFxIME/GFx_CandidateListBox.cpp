/**************************************************************************

Filename    :   GFx_CandidateListBox.cpp
Content     :   GFx to C++ ListBox
Created     :   2/23/2007
Authors     :   David Cook, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "GFx_CandidateListBox.h"

#ifndef SF_NO_IME_SUPPORT
#include "GFx_IMEWin32Impl.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{

// ****************************************************************************
// ListBox View: Constructor
//
FxCandidateListBox::FxCandidateListBox(GFxIMEWin32Impl* pimeImpl)
{
    ListPath[0]     = 0;
    pMovie          = NULL; 
	pImeImpl		= pimeImpl;
}


// ****************************************************************************
// ListBox View: Initialize the view
//
void FxCandidateListBox::RegisterMovieContainingWidget(Movie* pmovie)
{
    pMovie = pmovie;
}


// ****************************************************************************
// ListBox View: Set the AS list object's path
//
void FxCandidateListBox::SetASWidgetPath(const char* plp)
{
    if (plp)
    {
    //    SFstrcpy(ListPath, PathSize, plp);
        SFstrcpy(ListPath, PathSize, "candidateList");
    }
    else
        ListPath[0] = NULL;
}


// ****************************************************************************
// ListBox View: Refresh the UI view
//
// *** Communicate with the UI
void FxCandidateListBox::UIRefreshView()
{
    if (ListPath[0] == 0)
        return;

    // clear the list
    SFstrcpy(CurrPath, PathSize, ListPath);
    SFstrcat(CurrPath, PathSize, ".ClearList");
	Value result;
	pImeImpl->Invoke(CurrPath, &result, 0, 0);
//    pMovie->Invoke(CurrPath, 0);
    CurrPath[0] = 0;

    // Notify the UI of the current selection index (this may have changed)
    //UISelectItem(SelectedItemIndex);
    SFstrcpy(CurrPath, PathSize, ListPath);
    SFstrcat(CurrPath, PathSize, ".SelectDataItem");
 //   pMovie->Invoke(CurrPath, "%d", SelectedItemIndex);
	Array<Value> valArray;
	valArray.PushBack((double)SelectedItemIndex);
	pImeImpl->Invoke(CurrPath, &result, valArray.GetDataPtr(), 1);
    // add the visible items
        for (UInt32 i = 0; i < ListItems.GetSize(); i++)
            UIAddItem(ListItems[i]);

    // display the visible items

    SFstrcpy(CurrPath, PathSize, ListPath);
    SFstrcat(CurrPath, PathSize, ".SetList");
//    pMovie->Invoke(CurrPath, 0);
	pImeImpl->Invoke(CurrPath, &result, 0, 0);
    CurrPath[0] = 0;
}


// ****************************************************************************
// ListBox View: Add an item to the UI
//
void FxCandidateListBox::UIAddItem(Ptr<FxCandidateListItem> pli)
{
    if ( (ListPath[0] == 0) || (pli.GetPtr() == NULL) )
        return;

    SFstrcpy(CurrPath, PathSize, ListPath);
    SFstrcat(CurrPath, PathSize, ".Add");

    // fill in the temporary structures with the appropriate data
    for (unsigned i=0; i<ListItemDataKeys.GetSize(); i++)
    {
        (*pli)(ListItemDataValues[i], ListItemDataKeys[i]);
    }

    // add the item; params are an array of values expected by the list box
    // the expected order of values is set when SetItemDataMap is called
	Value result;

//	pMovie->Invoke(CurrPath, &result, &ListItemDataValues[0], (UInt32)ListItemDataValues.GetSize());
	Array<Value> valArray;
	pImeImpl->Invoke(CurrPath, &result, ListItemDataValues.GetDataPtr(), (UInt32)ListItemDataValues.GetSize());
    CurrPath[0] = 0;
}




// ****************************************************************************
// ListBox View: Callback function
//
bool FxCandidateListBox::VFsCallback(Movie* pmovie, int commandKey, const char* command, const char* args)
{
    SF_UNUSED2(pmovie, command);
    switch(commandKey)
    {

    case CMD_FxList_OnDestroy:
        {
            SetASWidgetPath(NULL);
            return 1;
        }
    case CMD_FxList_OnSetVisibleCount:
        {
            int val = atoi(&args[0]);
            VisibleItemsCount = val;
            UIRefreshView();
            return 1;
        }
    case CMD_FxList_OnSetItemDataMap:
        {
            ListItemDataKeys.Clear();
            ListItemDataValues.Clear();

            int i=0, w=0;
            while (args[i++])
            {
                if (args[i] == ',' || args[i] == ':' || args[i] == 0)
                {
                    int len = (i-w)+1;
                    char *str = (char*)malloc(len);
                    SFstrncpy(str, len, args+w, i-w);
                    str[len-1] = 0;
                    ListItemDataKeys.PushBack(str);
                    
                    free(str);
                    w = i+1;
                }
            }
            ListItemDataValues.Resize(ListItemDataKeys.GetSize()); 
            return 1;
        }
    case CMD_FxList_OnItemSelect:
        {
            //OnItemSelected(atoi(args));
            SetSelectedItemIndex( atoi(args) );
            return 1;
        }
    }
    return 0;
}



// ****************************************************************************
// Add a new ListItem
//
// Select it if needed
//
SPInt FxCandidateListBox::AddListItem(Ptr<FxCandidateListItem> pitem, bool select)
{
    SF_UNUSED(select);
    // insert the element and assign the selection afterwards
    ListItems.PushBack(pitem);

    // return the insertion index
    return 0;
}

// ****************************************************************************
// Remove all ListItems
//
// Maintains visible count
//
void FxCandidateListBox::RemoveAllListItems()
{
    ListItems.Clear();
}



// ****************************************************************************
// Set the currently selected item
//
void FxCandidateListBox::SetSelectedItemIndex(SPInt idx, bool refreshFlashList)
{
    SelectedItemIndex = idx;
	if (refreshFlashList)
	{
		Value result;
		Array<Value> valArray;
		SFstrcpy(CurrPath, PathSize, ListPath);
		SFstrcat(CurrPath, PathSize, ".SelectDataItem");
		valArray.PushBack((double)SelectedItemIndex);
	//	pMovie->Invoke(CurrPath, "%d", SelectedItemIndex);
		pImeImpl->Invoke(CurrPath, &result, valArray.GetDataPtr(), 1);
		SFstrcpy(CurrPath, PathSize, ListPath);
		SFstrcat(CurrPath, PathSize, ".SetList");
	//	pMovie->Invoke(CurrPath, "");
		pImeImpl->Invoke(CurrPath, &result, 0, 0);
	}
}

// ****************************************************************************
// Removes List
//
void FxCandidateListBox::RemoveList()
{
	Value result;
    ListItems.Clear();
    SFstrcpy(CurrPath, PathSize, ListPath);
    SFstrcat(CurrPath, PathSize, ".RemoveList");
   // pMovie->Invoke(CurrPath, "");
	pImeImpl->Invoke(CurrPath, &result, 0, 0);
    CurrPath[0] = 0;
}
}}}

#endif      // ifndef SF_NO_IME_SUPPORT