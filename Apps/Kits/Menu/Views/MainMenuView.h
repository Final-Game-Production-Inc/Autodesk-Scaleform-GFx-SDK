/**************************************************************************

Filename    :   MainMenuView.h
Content     :   C++ interface for the 'main menu' view in Menu Kit

Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_MainMenuView_H
#define INC_MainMenuView_H

// Includes
#include "MenuView.h"
#include "../States/GameStateWidgetSupport.h"
#include "../Util/GFxClikWidget.h"

// ********************************************************************
// An instance of MainMenuView is bound to the SWF movieclip by the
// main menu game state.
//
class MainMenuView : public MenuView
{
public:
    MainMenuView(GFx::Value pval, const char* viewName, ManagerView* pmanager) : MenuView(pval, viewName, pmanager)
    {
        // This String will be displayed in the footer for the view.
        ViewNameFriendly = "MAIN MENU";

        // Populate the options for the Main Menu. These could come from an external file.
        Option InstantActionOption;
        InstantActionOption.OptionId = START_GAME_ID;
        InstantActionOption.OptionLabel = "START GAME";
        InstantActionOption.OptionDesc = "Dive straight into the carnage.";
        ListOptions.PushBack(InstantActionOption);

        Option MultiplayerOption;
        MultiplayerOption.OptionId = SETTINGS_ID;
        MultiplayerOption.OptionLabel = "SETTINGS";
        MultiplayerOption.OptionDesc = "Configure the settings for the demo.";
        ListOptions.PushBack(MultiplayerOption);

        Option ExitOption;
        ExitOption.OptionId = EXIT_ID;
        ExitOption.OptionLabel = "EXIT";
        ExitOption.OptionDesc = "Exit the demo.";
        ListOptions.PushBack(ExitOption);

#ifdef SF_OS_PS3
        HelpString = "<img vspace='-5' src='ps3_a_png'> ACCEPT &nbsp; <img vspace='-5' src='ps3_b_png'> EXIT";
#elif defined SF_OS_XBOX360
        HelpString = "<img vspace='-5' src='xbox_a_png'> ACCEPT &nbsp; <img vspace='-5' src='xbox_b_png'> EXIT";
#else
        HelpString = "<img vspace='-5' src='pc_enter_png'> ACCEPT &nbsp; <img vspace='-5' src='pc_esc_png'> EXIT";
#endif
    }
    ~MainMenuView() {}

    enum MenuOption {
        START_GAME_ID,
        SETTINGS_ID,
        EXIT_ID
    };

    /** An option to be included in the list for the view. */
    struct Option
    {
        MenuOption      OptionId;
        String         OptionLabel;
        String         OptionDesc;
    };

    // Inherited WidgetHandler declarations.
    virtual bool        OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget);

    // Inherited UIView method declarations.
    virtual void        OnTopMostView(bool bPlayOpenAnimation = false);
    virtual void        DisableSubComponents(bool bDisableComponents);
    virtual void        ReturnToPreviousViewImpl();

    /** 
     * Updates the description/info textField (InfoTxt) whenever the selectedIndex of the 
     * list changes, based on the new selectedIndex
     */
    void                UpdateDescription(int selectedIndex);
    /** Updates the dataProvider for the list based on the ListOptions array of options. */
    void                UpdateListDataProvider();

    /** Spawns an exit dialog using the Manager and configures it. Public so that Manager can access it. */
    void                        SpawnExitDialog();
    /** Retrieves a reference to the menu's list. */
    GFxClikWidget&        GetList()   { return List; }

protected:
    /** Static GFxClikWidget event listeners for the list which function as proxies for methods in this view. */
    static void         OnList_ItemPress(UIView* pthis, const GFx::FunctionHandler::Params& params);
    static void         OnList_Change(UIView* pthis, const GFx::FunctionHandler::Params& params);
    static void         OnExitDialog_AcceptBtn_Press(UIView* pthis, const GFx::FunctionHandler::Params& params);

    /** Reference to the list for the view. */
    GFxClikWidget       List;
    /** Reference to the dataProvider for the list. */
    GFx::Value            ListDataProvider;

    /** The array of options for this view. These options are displayed in the view's list. */
    Array<Option>      ListOptions;

private:
    //

};

#endif  // INC_MainMenuView_H
