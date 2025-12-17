/**************************************************************************

Filename    :   SystemUIDialog.cpp
Content     :  C++ interface for the System UI's dialog box shown on
                an error.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "MenuDialog.h"
#include "ManagerView.h"

/** Handler for CLIK components' onLoad() callbacks. */
bool    SystemUIDialog::OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget) 
{
    SF_UNUSED(widgetPath);
    // printf("\nSystemUIDialog: WidgetName: %s, WidgetPath %s", widgetName, widgetPath);
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

    // The button.
    else if (!SFstrcmp(widgetName, "back"))
    {
        if ( !BackBtn.IsValid() )
        {
            GFx::Value tempVal;
            widget.GetMember("btn", &tempVal);

            // Cast it to a GFxClikWidget and give it a reference to the MovieView so we can add
            // event listeners.
            BackBtn.SetWidget(tempVal);

            // Set the label for the button.
            tempVal.SetString("BACK");
            BackBtn.SetMember("label", tempVal);

            String pressEventString = (GetMovieView()->GetAVMVersion() == 1) ? "press" : "buttonPress";
            Ptr<UIViewListenerProxy> pressListener = *new UIViewListenerProxy(this, OnBackButton_Press);
            BackBtn.AddEventListener(GetMovieView(), pressEventString, pressListener);

            bHandled = true;
        }
    }

    return bHandled;
}

/** Called when the view is pushed on to the stack. Reoccurring view setup can be done here. */
void    SystemUIDialog::OnTopMostView(bool bPlayOpenAnimation)
{
    UIView::OnTopMostView(bPlayOpenAnimation);
    SetSelectionFocus(BackBtn);
}

/** Enable/disable sub-components of the view. */
void    SystemUIDialog::DisableSubComponents(bool bDisableComponents)
{
    GFx::Value bDisable;
    bDisable.SetBoolean(bDisableComponents);
    if ( BackBtn.IsValid() )
    {
        BackBtn.SetMember("disabled", bDisable);
    }
}

/** Called just before view is popped from the stack. Custom logic for the view's shutdown should be added here. */
void    SystemUIDialog::OnBackButton_Press(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    SF_UNUSED(params);
    SystemUIDialog* pview = (SystemUIDialog*) pthis;
    pview->GetGameState()->OnCloseSystemDialog();
}

void    SystemUIDialog::SetTitle(const char* title)
{
    if ( !(TitleTxt.IsNull() || TitleTxt.IsUndefined()) )
    {
        TitleTxt.SetText(title);
    }
}

void    SystemUIDialog::SetInfo(const char* info)
{
    if ( !(InfoTxt.IsNull() || InfoTxt.IsUndefined()) )
    {
        InfoTxt.SetText(info);
    }
}

void    SystemUIDialog::SetBackButtonLabel( const char* label )
{
    SF_ASSERT(BackBtn.IsValid());
    GFx::Value labelVal;
    labelVal.SetString( label );
    BackBtn.SetMember( "label", labelVal );
}

/** Sets focus to a particular MovieClip using Selection.setFocus(). */
void    SystemUIDialog::SetSelectionFocus(GFxClikWidget& targetMovieClip)
{
    View.Invoke("setSelectionFocus", NULL, &(targetMovieClip.GetRef()), 1);
}
