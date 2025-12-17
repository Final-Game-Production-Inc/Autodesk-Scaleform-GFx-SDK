/**************************************************************************

Filename    :   SystemUIDialog.h
Content     :  C++ interface for the System UI's dialog box shown on
                an error.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SystemUIDialog_H
#define INC_SystemUIDialog_H

// Includes
#include "UIView.h"
#include "../Util/GFxClikWidget.h"
#include "../States/GameStateWidgetSupport.h"

class GameState_System;

class SystemUIDialog : public UIView, public WidgetHandler
{
public:
    SystemUIDialog(GFx::Value val, GameState_System* pgameState) : UIView(val, NULL, NULL), pGameState(pgameState) {}
    virtual ~SystemUIDialog() {}

    GameState_System*           GetGameState() const    { return pGameState; }
    // Required for AS3 since we won't receive a initCallback for the View until after all children.
    void                        SetWidgetRef(GFx::Value val)         { View = val; }
    void                        SetMovieView(GFx::Movie* pmovieView) { pMovieView = pmovieView; }
    GFx::Movie*                 GetMovieView() const    { return pMovieView; }

    // Inherited WidgetHandler declarations.
    virtual bool    OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget);

    // Inherited UIView method declarations.
    virtual void    OnTopMostView(bool bPlayOpenAnimation = false);
    virtual void    DisableSubComponents(bool bDisableComponents);

    static void     OnBackButton_Press(UIView* pthis, const GFx::FunctionHandler::Params& params);

    const GFxClikWidget&  GetBackButton() const     { return BackBtn; }

    void            SetTitle(const char* title);
    void            SetInfo(const char* info);
    void            SetBackButtonLabel(const char* label);

    /** Sets focus to a particular MovieClip using Selection.setFocus(). */
    void            SetSelectionFocus(GFxClikWidget& targetMovieClip);

protected:
    /** Reference to Back button for returning to previous screen. */
    GFxClikWidget       BackBtn;
    /** Reference to the title text at the bottom within the footer. */
    GFx::Value            TitleTxt;
    /** Reference to the info/description text area. */
    GFx::Value            InfoTxt;

    /** Reference to the GameState that created this view. */
    GameState_System*       pGameState;
    /** Reference to the MovieView so new Objects can be created dynamically as needed. */
    GFx::Movie*           pMovieView;
};

#endif  // INC_SystemUIDialog_H
