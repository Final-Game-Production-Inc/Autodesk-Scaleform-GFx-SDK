/**************************************************************************

Filename    :   StartGameView.h
Content     :  C++ implementation of the Start Game view of the Menu Kit.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_StartGameView_H
#define INC_StartGameView_H

// Includes
#include "MenuView.h"
#include "../States/GameStateWidgetSupport.h"
#include "../Util/GFxClikWidget.h"

class StartGameView : public MenuView
{
public:
    StartGameView(GFx::Value pval, const char* viewName, ManagerView* pmanager) : MenuView(pval, viewName, pmanager)
    {
        ViewNameFriendly = "SELECT DIFFICULTY";

        Option CasualOption;
        CasualOption.OptionId = CASUAL_ID;
        CasualOption.OptionLabel = "CASUAL";
        CasualOption.OptionDesc = "For players looking to experience the game without any of the challenge.";
        CasualOption.OptionImage = "images/difficulty_easy.png";
        ListOptions.PushBack(CasualOption);

        Option NormalOption;
        NormalOption.OptionId = NORMAL_ID;
        NormalOption.OptionLabel = "NORMAL";
        NormalOption.OptionDesc = "Recommended for the standard player.";
        NormalOption.OptionImage = "images/difficulty_medium.png";
        ListOptions.PushBack(NormalOption);

        Option VeteranOption;
        VeteranOption.OptionId = VETERAN_ID;
        VeteranOption.OptionLabel = "VETERAN";
        VeteranOption.OptionDesc = "For experienced veterans only.";
        VeteranOption.OptionImage = "images/difficulty_hard.png";
        ListOptions.PushBack(VeteranOption);
    }
    ~StartGameView() {}

    enum DifficultyLevel {
        CASUAL_ID,
        NORMAL_ID,
        VETERAN_ID
    };

    struct Option
    {
        DifficultyLevel     OptionId;
        String             OptionLabel;
        String             OptionDesc;
        String             OptionImage;
    };

    // Inherited WidgetHandler declarations.
    virtual bool        OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget);

    // Inherited UIView method declarations.
    virtual void        OnViewActivated();
    virtual void        OnTopMostView( bool bIsTopView );
    virtual void        DisableSubComponents(bool bDisableComponents);

    const GFxClikWidget&       GetList() const     { return List; }

    /** 
     * Updates the description/info textField (InfoTxt) whenever the selectedIndex of the 
     * list changes, based on the new selectedIndex
     */
    void                UpdateDescription(int selectedIndex);
    /** Updates the dataProvider for the list based on the ListOptions array of options. */
    void                UpdateListDataProvider();

    /** Moves the user backward on the view stack by popping the topmost view / dialog. */
    virtual void        ReturnToPreviousViewImpl();

protected:
    /** Static GFxClikWidget event listeners for the list which function as proxies for methods in this view. */
    static void         OnList_ItemPress(UIView* pthis, const GFx::FunctionHandler::Params& params);
    static void         OnList_Change(UIView* pthis, const GFx::FunctionHandler::Params& params);
    static void         OnList_FocusIn(UIView* pthis, const GFx::FunctionHandler::Params& params);

    /** Reference to the list for the view. */
    GFxClikWidget       List;
    /** Reference to the dataProvider for the list. */
    GFx::Value            ListDataProvider;

    /** Reference to the image scroller component on the right side of the view. */
    GFx::Value            ImageScroller;

    /** The array of options for this view. These options are displayed in the view's list. */
    Array<Option>      ListOptions;

private:

};

#endif  // INC_StartGameView_H
