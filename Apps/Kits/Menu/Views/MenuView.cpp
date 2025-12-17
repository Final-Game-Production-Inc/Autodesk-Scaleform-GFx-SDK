/**************************************************************************

Filename    :   MenuView.cpp
Content     :  C++ implementation of the standard view layout in the 
                in the Menu Kit.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "MenuView.h"
#include "ManagerView.h"

bool    MenuView::OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget) 
{
    SF_UNUSED(widgetPath);

    bool bHandled = false;
    if (!SFstrcmp(widgetName, "help"))
    {
        widget.GetMember("textField", &HelpTxt);
        
        GFx::Value helpText(HelpString);
        HelpTxt.SetMember("htmlText", helpText);
        bHandled = true;
    }
    else if (!SFstrcmp(widgetName, "title"))
    {
        GFx::Value tempVal;
        TitleMC = widget;
        TitleMC.GetMember("textField", &tempVal);
        tempVal.SetText(ViewNameFriendly);
        bHandled = true;
    }
    else if (!SFstrcmp(widgetName, "back"))
    {
        GFx::Value tempVal;
        widget.GetMember("btn", &tempVal);

        // Cast it to a GFxClikWidget and give it a reference to the MovieView so we can add
        // event listeners.
        BackBtn = GFxClikWidget(tempVal);

        // Set the label for the button.
        tempVal.SetString("BACK");
        BackBtn.SetMember("label", tempVal);

        String pressEventString = (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) ? "press" : "buttonPress";
        Ptr<UIViewListenerProxy> pressListener = *new UIViewListenerProxy(this, OnBackButton_Press);
        BackBtn.AddEventListener(pManager->GetMovieView(), pressEventString, pressListener);

        Ptr<UIViewListenerProxy> focusInListener = *new UIViewListenerProxy(this, OnBackBtn_FocusIn);
        BackBtn.AddEventListener(pManager->GetMovieView(), "focusIn", focusInListener);

        bHandled = true;
    }
    else if (!SFstrcmp(widgetName, "info"))
    {
        widget.GetMember("textField", &InfoTxt); 
        bHandled = true;
    }
    else if (!SFstrcmp(widgetName, "footer"))
    {
        FooterMC = widget;
        bHandled = true;
    }

    return bHandled;
}

/** 
 * Event listener for BackBtn's "press" event.
 * Pops a view from the ManagerView's ViewStack and returns the user to the previous screen.
 */
void    MenuView::OnBackButton_Press(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    SF_UNUSED(params);
    MenuView* pview = (MenuView*) pthis;
    pview->ReturnToPreviousViewImpl();
}

/** 
 * Event listener for BackBtn's "focusIn" event.
 * Clears the info textField. Required since a list won't fire a "change" event when focus 
 * leaves the component.
 */
void    MenuView::OnBackBtn_FocusIn(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    SF_UNUSED(params);
    MenuView* pview = (MenuView*) pthis;
    if ( !(pview->InfoTxt.IsNull() || pview->InfoTxt.IsUndefined()) )
    {
        pview->InfoTxt.SetText("");
    }
}
