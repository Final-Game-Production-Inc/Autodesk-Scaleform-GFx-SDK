/**************************************************************************

Filename    :   StartGameView.cpp
Content     :  C++ implementation of the Start Game view of the Menu Kit.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "StartGameView.h"
#include "ManagerView.h"

bool    StartGameView::OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget)
{
    bool bHandled = false;

    //printf("\nStartGameView: WidgetName: %s, WidgetPath %s", widgetName, widgetPath);
    // The primary option list on the left side of the view.
    if (!SFstrcmp(widgetName, "list"))
    {
        List = GFxClikWidget(widget);

        String changeEventString = (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) ? "change" : "indexChange";

        Ptr<UIViewListenerProxy> changeListener = *new UIViewListenerProxy(this, OnList_Change);
        List.AddEventListener(pManager->GetMovieView(), changeEventString, changeListener);

        Ptr<UIViewListenerProxy> itemPressListener = *new UIViewListenerProxy(this, OnList_ItemPress);
        List.AddEventListener(pManager->GetMovieView(), "itemPress", itemPressListener);

        Ptr<UIViewListenerProxy> focusInListener = *new UIViewListenerProxy(this, OnList_FocusIn);
        List.AddEventListener(pManager->GetMovieView(), "focusIn", focusInListener);

        bHandled = true;
    }
    // The image scroller on the right side of the view.
    else if (!SFstrcmp(widgetName, "imgScroller"))
    {
        ImageScroller = widget;
        
        if (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) {
            GFx::Value selectedIndex;
            selectedIndex.SetNumber(0);
            widget.SetMember("selectedIndex", selectedIndex);
        }
        
        bHandled = true;
    }
    
    if (!bHandled)
    {
        bHandled = MenuView::OnWidgetInitialized(widgetName, widgetPath, widget);
    }

    return bHandled;
}

void    StartGameView::OnViewActivated()
{
    //
}

/** Enable/disable sub-components of the view. */
void    StartGameView::DisableSubComponents(bool bDisableComponents)
{
    GFx::Value bDisable;
    bDisable.SetBoolean(bDisableComponents);

    SF_ASSERT(List.IsValid());
    List.SetMember("disabled", bDisable);

    SF_ASSERT(BackBtn.IsValid());
    BackBtn.SetMember("disabled", bDisable);
}

/** Called when the view is pushed on to the stack. Reoccurring view setup can be done here. */
void    StartGameView::OnTopMostView( bool bIsTopView )
{
    MenuView::OnTopMostView(bIsTopView);
    
    if ( List.IsValid() )
    {
        UpdateListDataProvider();

        // Reset the selectedIndex of the List to 0.
        GFx::Value tempVal( (double)0.0 );
        List.SetMember("selectedIndex", tempVal);

        pManager->SetSelectionFocus(List);
    }
}

void    StartGameView::UpdateListDataProvider()
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

        tempVal.SetString(ListOptions[i].OptionImage);
        tempObj.SetMember("image", tempVal);

        dataProviderArray.SetElement(i, tempObj);
    }

    if (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) 
    {
        List.SetMember("dataProvider", dataProviderArray);
        ImageScroller.SetMember("dataProvider", dataProviderArray);
    }

    else if (GetManagerView()->GetMovieView()->GetAVMVersion() == 2) 
    {
        GFx::Value dataProviderObject;
        pManager->GetMovieView()->CreateObject(&dataProviderObject, "scaleform.clik.data.DataProvider", &dataProviderArray, 1);
        List.SetMember("dataProvider", dataProviderObject);
        List.GetRef().Invoke("validateNow");

        ImageScroller.SetMember("dataProvider", dataProviderObject);
    }

    tempVal.SetNumber(0);
    ImageScroller.SetMember("selectedIndex", tempVal);
    List.GetMember("dataProvider", &ListDataProvider);
}

void    StartGameView::UpdateDescription(int selectedIndex)
{
    SF_ASSERT(List.IsValid());
    if (!(InfoTxt.IsNull() || InfoTxt.IsUndefined()))
    {
        // If a selectedIndex wasn't provided, retrieve it.
        if (selectedIndex < 0)
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

        // Set the description for the Info textField.
        String description = ListOptions[selectedIndex].OptionDesc;
        InfoTxt.SetText(description);
    }
}

/** 
 * "itemPress" eventListener for the list.
 * Launches the game.
 */
void    StartGameView::OnList_ItemPress(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    StartGameView* pview = (StartGameView*)pthis;
    EventData eventData = ConvertParamsToEventData(params);
    unsigned selectedIndex = eventData.Index;
    unsigned itemId = pview->ListOptions[selectedIndex].OptionId;

    switch(itemId)
    {
        case(StartGameView::CASUAL_ID):
        case(StartGameView::NORMAL_ID):
        case(StartGameView::VETERAN_ID):
            pview->GetManagerView()->LaunchGame();
            break;
        default:
            break;
    }
}

/** 
 * "change" eventListener for the list.
 * Updates the description textField and the image based on the new selected index.
 */
void    StartGameView::OnList_Change(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    StartGameView* pview = (StartGameView*)pthis;
    EventData eventData = ConvertParamsToEventData(params);
    unsigned selectedIndex = eventData.Index;
    pview->UpdateDescription(selectedIndex);

    // Update the selectedIndex of the Image Scroller.
    GFx::Value tempVal( (double)selectedIndex );
    pview->ImageScroller.SetMember("selectedIndex", tempVal);
}

void    StartGameView::OnList_FocusIn(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    SF_UNUSED(params);
    StartGameView* pview = (StartGameView*)pthis;
    GFx::Value selectedIndexValue;
    pview->GetList().GetMember("selectedIndex", &selectedIndexValue);
    unsigned selectedIndex = (unsigned)toNumber(selectedIndexValue);
    pview->UpdateDescription(selectedIndex);
}

/** Moves the user backward on the view stack by popping the topmost view / dialog. */
void    StartGameView::ReturnToPreviousViewImpl()
{
    UIView::ReturnToPreviousViewImpl();
}
