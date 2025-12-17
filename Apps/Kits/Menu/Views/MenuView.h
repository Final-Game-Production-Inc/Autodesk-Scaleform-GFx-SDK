/**************************************************************************

Filename    :   MenuView.h
Content     :  C++ implementation of the standard view layout in the 
                in the Menu Kit.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_MenuView_H
#define INC_MenuView_H

// Includes
#include "UIView.h"
#include "../Util/GFxClikWidget.h"
#include "../States/GameStateWidgetSupport.h"

class MenuView : public UIView, public WidgetHandler
{
public:
    MenuView(GFx::Value val,  const char* viewName, ManagerView* pmanager) : UIView(val, viewName, pmanager), 
        ViewNameFriendly(viewName)
    {
#ifdef SF_OS_PS3
        HelpString = "<img vspace='-5' src='ps3_a_png'> ACCEPT &nbsp; <img vspace='-5' src='ps3_b_png'> BACK";
#elif defined SF_OS_XBOX360
        HelpString = "<img vspace='-5' src='xbox_a_png'> ACCEPT &nbsp; <img vspace='-5' src='xbox_b_png'> BACK";
#else
        HelpString = "<img vspace='-5' src='pc_enter_png'> ACCEPT &nbsp; <img vspace='-5' src='pc_esc_png'> BACK";
#endif
    }
    virtual ~MenuView() {}

    // Inherited WidgetHandler declarations.
    virtual bool    OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget);
    
    /** Clears the InfoTxt textField whenever focus enters the "Back" button. */
    static void    OnBackBtn_FocusIn(UIView* pthis, const GFx::FunctionHandler::Params& params);
    static void    OnBackButton_Press(UIView* pthis, const GFx::FunctionHandler::Params& params);

protected:
    /** Title of the view. Used in the footer (TitleMC). */
    String              ViewNameFriendly;
    /** The context menu (button assignments) displayed in the footer beneath the title. */
    String              HelpString;

    /** Reference to the text field of the help text in the footer which displays controls. */
    GFx::Value          HelpTxt;
    /** Reference to the title text at the bottom within the footer. */
    GFx::Value          TitleMC;
    /** Reference to Back button for returning to previous screen. */
    GFxClikWidget       BackBtn;
    /** Reference to the footer of the view. */
    GFx::Value          FooterMC;
    /** Reference to the info/description text area. */
    GFx::Value          InfoTxt;

    /** Button images and strings to be displayed in the help text of the footer. */
    String              AcceptHelpImage, AcceptHelpText, CancelHelpImage, CancelHelpText;
};

#endif  // INC_MenuView_H
