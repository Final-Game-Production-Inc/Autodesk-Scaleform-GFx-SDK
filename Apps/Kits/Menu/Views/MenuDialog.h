/**************************************************************************

Filename    :   MenuDialog.h
Content     :  C++ interface for dialog boxes managed by the ManagerView.
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_MenuDialog_H
#define INC_MenuDialog_H

// Includes
#include "UIView.h"
#include "../Util/GFxClikWidget.h"
#include "../States/GameStateWidgetSupport.h"

class MenuDialog : public UIView, public WidgetHandler
{
public:
    MenuDialog(GFx::Value val,  const char* viewName, ManagerView* pmanager) : UIView(val, viewName, pmanager) {}
    virtual ~MenuDialog() {}

    // Inherited WidgetHandler declarations.
    virtual bool    OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget);

    // Inherited UIView method declarations.
    virtual void    OnTopMostView(bool bPlayOpenAnimation = false);
    virtual void    DisableSubComponents(bool bDisableComponents);

    static void     OnBackButton_Press(UIView* pthis, const GFx::FunctionHandler::Params& params);

    GFxClikWidget  GetAcceptButton() const  { return AcceptBtn; }
    GFxClikWidget  GetBackButton() const    { return BackBtn; }

    void            SetTitle(const char* title);
    void            SetInfo(const char* info);
    void            SetAcceptButtonLabel( const char* label );
    void            SetBackButtonLabel( const char* label );

protected:
    /** Reference to the Accept button, clicked when the user accepts the option presented. */
    GFxClikWidget       AcceptBtn;
    /** Reference to Back button for returning to previous screen. */
    GFxClikWidget       BackBtn;
    /** Reference to the title text at the bottom within the footer. */
    GFx::Value            TitleTxt;
    /** Reference to the info/description text area. */
    GFx::Value            InfoTxt;
};

#endif  // INC_MenuDialog_H
