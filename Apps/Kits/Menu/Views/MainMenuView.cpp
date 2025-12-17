/**************************************************************************

Filename    :   MainMenuView.cpp
Content     :  C++ interface for the Main Menu view of the Menu Kit.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "MainMenuView.h"
#include "ManagerView.h"

/** Handler for CLIK components' onLoad() callbacks. */
bool    MainMenuView::OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget)
{
    // printf("\nMainMenuView: WidgetName: %s, WidgetPath %s", widgetName, widgetPath);
    bool bHandled = false;
    if (!SFstrcmp(widgetName, "list"))
    {
        List = GFxClikWidget(widget);

        String changeEventString = (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) ? "change" : "indexChange";

        Ptr<UIViewListenerProxy> changeListener = *new UIViewListenerProxy(this, OnList_Change);
        List.AddEventListener(pManager->GetMovieView(), changeEventString, changeListener);

        Ptr<UIViewListenerProxy> itemPressListener = *new UIViewListenerProxy(this, OnList_ItemPress);
        List.AddEventListener(pManager->GetMovieView(), "itemPress", itemPressListener);

        pManager->SetSelectionFocus(List);

        GFx::Value selectedIndex;
        selectedIndex.SetNumber(0);
        List.SetMember("selectedIndex", selectedIndex);

        UpdateListDataProvider();
        UpdateDescription(0);
        bHandled = true;
    }

    if (!bHandled)
    {
        bHandled = MenuView::OnWidgetInitialized(widgetName, widgetPath, widget);
    }

    return bHandled;
}

/** Enable/disable sub-components of the view. */
void    MainMenuView::DisableSubComponents(bool bDisableComponents)
{
    if ( List.IsValid() )
    {
        GFx::Value bDisable;
        bDisable.SetBoolean(bDisableComponents);
        List.SetMember("disabled", bDisable);
    }
}

/** Called when the view is pushed on to the stack. Reoccurring view setup can be done here. */
void    MainMenuView::OnTopMostView(bool bPlayOpenAnimation)
{
    UIView::OnTopMostView(bPlayOpenAnimation);
    pManager->SetSelectionFocus(List);
    UpdateDescription(-1);
}

/** Called just before view is popped from the stack. Custom logic for the view's shutdown should be added here. */
void    MainMenuView::ReturnToPreviousViewImpl()
{
    // Since the Main Menu can't be popped from the viewStack, we'll show the ExitDialog instead.
    SpawnExitDialog();
}

/** Updates the dataProvider for the list with the Options in ListOptions. */
void    MainMenuView::UpdateListDataProvider()
{
    GFx::Value dataProviderArray, tempObj, tempVal;
    pManager->GetMovieView()->CreateArray(&dataProviderArray);
    
    for (unsigned i = 0; i < ListOptions.GetSize(); i++)
    {        
        pManager->GetMovieView()->CreateObject(&tempObj, "Object");

        tempVal.SetNumber(ListOptions[i].OptionId);
        tempObj.SetMember("id", tempVal);

        tempVal.SetString(ListOptions[i].OptionLabel);
        tempObj.SetMember("label", tempVal);

        tempVal.SetString(ListOptions[i].OptionDesc);
        tempObj.SetMember("desc", tempVal);

        dataProviderArray.SetElement(i, tempObj);
    }

    if (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) 
    {
        // Set the dataProvider using ScrollingList :: set dataProvider() to ensure "change" event is fired.
        List.SetMember("dataProvider", dataProviderArray);
    }
    else if (GetManagerView()->GetMovieView()->GetAVMVersion() == 2) 
    {
        GFx::Value dataProviderObject;
        pManager->GetMovieView()->CreateObject(&dataProviderObject, "scaleform.clik.data.DataProvider", &dataProviderArray, 1);
        List.SetMember("dataProvider", dataProviderObject);
        List.GetRef().Invoke("validateNow");
    }

    // Make sure we have a reference pointing to the latest dataProvider for the list for convenient access.
    List.GetMember("dataProvider", &ListDataProvider);
}

/** Updates the information textField with the description for the selected item in the list. */
void    MainMenuView::UpdateDescription(int selectedIndex)
{
    if (!(InfoTxt.IsNull() || InfoTxt.IsUndefined()))
    {
        // If a selectedIndex wasn't provided, retrieve it.
        if (selectedIndex < 0 && List.IsValid())
        {
            GFx::Value tempVal;
            List.GetMember("selectedIndex", &tempVal);
            selectedIndex = unsigned(toNumber(tempVal));
        }

        // If it's still -1, set it back to 0 before we access the array.
        if (selectedIndex < 0)
        {
            selectedIndex = 0;
        }

		if (selectedIndex > ((int)ListOptions.GetSize())-1) { return; }
        // Set the description for the Info textField.
        String description = ListOptions[selectedIndex].OptionDesc;
        InfoTxt.SetText(description);
    }
}

/** Requests that the Exit Dialog view be pushed on to the viewStack and then configures it. */
void    MainMenuView::SpawnExitDialog()
{
    MenuDialog* ExitDialog;
    ExitDialog = (MenuDialog*)pManager->SpawnDialog("ExitDialog");

    SF_ASSERT(ExitDialog->GetAcceptButton().IsValid());
    SF_ASSERT(ExitDialog->GetBackButton().IsValid());

    String pressEventString = (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) ? "press" : "buttonPress";
    ExitDialog->GetAcceptButton().ASRemoveAllEventListeners( pressEventString );

    Ptr<UIViewListenerProxy> pressListener = *new UIViewListenerProxy(this, OnExitDialog_AcceptBtn_Press);
    ExitDialog->GetAcceptButton().AddEventListener(pManager->GetMovieView(), pressEventString, pressListener);

    ExitDialog->SetTitle("EXIT GAME");
    ExitDialog->SetInfo("Are you sure you wish to exit?");
    
    ExitDialog->SetAcceptButtonLabel("EXIT GAME");
    ExitDialog->SetBackButtonLabel("CANCEL");
}

/**
 * Event listener for "itemPress" event from the List.
 * Pushes the appropriate view on to the viewStack based on the selected item.
 */
void    MainMenuView::OnList_ItemPress(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    MainMenuView* pview = (MainMenuView*)pthis;
    EventData eventData = ConvertParamsToEventData(params);
    unsigned selectedIndex = eventData.Index;
    unsigned itemId = pview->ListOptions[selectedIndex].OptionId;

    switch(itemId)
    {
        case(MainMenuView::START_GAME_ID):
            pview->pManager->PushViewByName("StartGame");
            break;
        case(MainMenuView::SETTINGS_ID):
            pview->pManager->PushViewByName("Settings");
            break;
        case(MainMenuView::EXIT_ID):
            pview->SpawnExitDialog();
            break;
        default:
            break;
    }
}

/**
 * Event listener for "changes" event from the List.
 * Updates the description based on the new selectedIndex.
 */
void    MainMenuView::OnList_Change(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    MainMenuView* pview = (MainMenuView*)pthis;
    EventData eventData = ConvertParamsToEventData(params);
    unsigned selectedIndex = eventData.Index;
    pview->UpdateDescription(selectedIndex);
}

void    MainMenuView::OnExitDialog_AcceptBtn_Press(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    SF_UNUSED(params);
    MainMenuView* pview = (MainMenuView*)pthis;
    pview->GetManagerView()->ExitGame();
}
