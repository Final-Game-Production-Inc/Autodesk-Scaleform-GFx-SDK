/**************************************************************************

Filename    :   MenuDialog.cpp
Content     :  C++ interface a dialog window in the Main Menu managed
                by the ManagerView.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "MenuDialog.h"
#include "ManagerView.h"

bool    MenuDialog::OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget) 
{
    SF_UNUSED(widgetPath);
    bool bHandled = false;

    // The title textField for the dialog.
    if (!SFstrcmp(widgetName, "popup_title"))
    {
        if ( (TitleTxt.IsUndefined() || TitleTxt.IsNull()) )
        {
            widget.GetMember("textField", &TitleTxt);
            bHandled = true;
        }
    }

    // Information textField for the current selection.
    else if (!SFstrcmp(widgetName, "popup_info"))
    {
        if ( (InfoTxt.IsUndefined() || InfoTxt.IsNull()) )
        {
            widget.GetMember("textField", &InfoTxt); 
            bHandled = true;
        }
    }

    else if (!SFstrcmp(widgetName, "back"))
    {
        
        //if ( (BackBtn.IsUndefined() || BackBtn.IsNull()) )
        //{
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

            bHandled = true;
        //}
    }

    else if (!SFstrcmp(widgetName, "exit"))
    {
        if ( !AcceptBtn.IsValid() )
        {
            GFx::Value tempVal;
            widget.GetMember("btn", &tempVal);

            // Cast it to a GFxClikWidget and give it a reference to the MovieView so we can add
            // event listeners.
            AcceptBtn = GFxClikWidget(tempVal);
        }

        bHandled = true;
    }

    return bHandled;
}

void    MenuDialog::OnTopMostView(bool bPlayOpenAnimation)
{
    UIView::OnTopMostView(bPlayOpenAnimation);
    pManager->SetSelectionFocus(BackBtn);
}

/** Enable/disable sub-components of the view. */
void    MenuDialog::DisableSubComponents(bool bDisableComponents)
{
    GFx::Value bDisable;
    bDisable.SetBoolean(bDisableComponents);

    if (AcceptBtn.IsValid())
    {
        AcceptBtn.SetMember("disabled", bDisable);
    }

    if (BackBtn.IsValid())
    {
        BackBtn.SetMember("disabled", bDisable);
    }
}

void    MenuDialog::OnBackButton_Press(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    SF_UNUSED(params);
    MenuDialog* pview = (MenuDialog*) pthis;
    pview->ReturnToPreviousViewImpl();
}

void    MenuDialog::SetTitle(const char* title)
{
    if ( !(TitleTxt.IsNull() || TitleTxt.IsUndefined()) )
    {
        TitleTxt.SetText(title);
    }
}

void    MenuDialog::SetInfo(const char* info)
{
    if ( !(InfoTxt.IsNull() || InfoTxt.IsUndefined()) )
    {
        InfoTxt.SetText(info);
    }
}

void    MenuDialog::SetAcceptButtonLabel( const char* label )
{
    if ( AcceptBtn.IsValid() )
    {
        GFx::Value labelVal;
        labelVal.SetString( label );
        AcceptBtn.SetMember( "label", labelVal );
    }
}

void    MenuDialog::SetBackButtonLabel( const char* label )
{
    if ( BackBtn.IsValid() )
    {
        GFx::Value labelVal;
        labelVal.SetString( label );
        BackBtn.SetMember( "label", labelVal );
    }
}
